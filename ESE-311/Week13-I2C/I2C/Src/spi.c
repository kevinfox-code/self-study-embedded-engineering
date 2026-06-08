/*
    For NUCLEO-U575ZI-Q, this module implements:
    - SPI1: SCK=PA5, MISO=PA6, MOSI=PA7
    - CS(GPIO): PD14

    SPI1 is on the APB2 bus on this MCU.
    RCC_APBENR pin for SPI1 is bit 12(SPI1EN). SPI1EN=1 enables the clock for SPI1.

    GPIO is on AHB2 bus on this MCU.
    RCC_AHB2ENR1 pin for GPIOD is bit 3(GPIODEN). GPIODEN=1 enables the clock for GPIOD.
    RCC_AHB2ENR1 pin for GPIOA is bit 0(GPIOAEN). GPIOAEN=1 enables the clock for GPIOA.

    SPI Configuration Register 2 (SPI_CFG2) bits:
        CPOL bit 25 for clock polarity
        CPHA bit 24 for clock phase
        MASTER bit 22 for master mode
        SPIEN bit 0 for enabling the SPI peripheral
        SSM bit 26 for software slave management (SSM=1 means software controls the NSS pin, SSM=0
   means hardware controls the NSS pin)

    SPI Configuration Register 1 (SPI_CFG1):
        MBR[2:0] bits [30:28] for baud rate control

    SPI Control Register 1 (SPI_CR1) bits:
        SSI bit 12 is value to be forced onto the NSS pin when SSM=1 (software slave management
   enabled)


    SPI Status Register (SPI_SR) bits:
        RXWNE bit 15 for rxFIFO not empty (RXWNE=1 at least 4 bytes in buffer)

    SPI Data Register (SPI_DR) bits:
*/
#include "spi.h"
#include "uart.h"
#include "config.h"
#include <stddef.h>

#define SPI1EN  (1U << 12U) // bit 12 for SPI1EN in RCC_APB2ENR
#define SPI1RST (1U << 12U) // bit 12 for SPI1RST in RCC_APB2RSTR
#define GPIODEN (1U << 3U)  // bit 3 for GPIODEN in RCC_AHB2ENR1
#define GPIOAEN (1U << 0U)  // bit 0 for GPIOAEN in RCC_AHB2ENR1
#define SR_TXP  (1U << 1U)  /* Tx-packet space available  */
#define SR_RXP  (1U << 0U)  /* Rx-packet available        */
#define SR_EOT  (1U << 3U)  /* End of transfer (use instead of TXC with TSIZE > 0) */

#define CR1_SPE    (1U << 0U)
#define CR1_CSTART (1U << 9U)
#define CR1_SSI    (1U << 12U)

void spi_gpio_init(void)
{
    /* Enable clock access to GPIOA
     * NOTE: On the U575, GPIO clocks are on AHB2ENR1, not AHB1ENR */
    RCC_NS->AHB2ENR1 |= GPIOAEN;

    /* Set PA5, PA6, PA7 mode to alternate function (MODER = 0b10 per pin).
     * Single read-modify-write clears all three pin-pairs and sets the new value
     * atomically — avoids stale-read races from back-to-back RMW operations on
     * the same register. */
    GPIOA_NS->MODER = (GPIOA_NS->MODER & ~(0x3FU << 10)) | (0x2AU << 10);

    /* Set PD14 as output pin (chip select) */
    RCC_NS->AHB2ENR1 |= GPIODEN;
    /* Pre-set ODR bit HIGH before enabling output drive — prevents CS glitching LOW
     * during the MODER write since ODR defaults to 0 after reset. */
    GPIOD_NS->BSRR = (1U << 14U);
    /* PD14 MODER bits: [29:28] -> set bit28=1, bit29=0 for General Purpose Output (01) */
    GPIOD_NS->MODER = (GPIOD_NS->MODER & ~(0x3U << 28U)) | (0x1U << 28U);

    /* Set PA5, PA6, PA7 alternate function to AF5 (SPI1).
     * AF5 = 0b0101 per pin.  Same single-RMW rationale as MODER above. */
    GPIOA_NS->AFR[0] = (GPIOA_NS->AFR[0] & ~(0xFFFU << 20)) | (0x555U << 20);
}

void spi1_config(void)
{
    /* Enable clock access to SPI1, then reset the peripheral to guarantee a
     * clean register state regardless of what a previous firmware run left. */
    RCC_NS->APB2ENR |= SPI1EN;
    RCC_NS->APB2RSTR |= SPI1RST;
    __DSB();
    RCC_NS->APB2RSTR &= ~SPI1RST;

    /* --- All configuration must be done before setting SPE --- */

    /* CFG1: MBR=001 (/4 prescaler, bits[30:28]), DSIZE=00111 (8-bit, bits[4:0]).
     * Written as a single value — multiple read-modify-writes on the same
     * register can lose bits if the APB peripheral hasn't latched the previous
     * write before the next read-modify-write cycle reads it back. */
    SPI1_NS->CFG1 = (1U << 28U) /* MBR[0]=1 → /4 */
                  | (7U << 0U); /* DSIZE=7  → 8 bits */

    /* SSI must be set BEFORE writing MASTER in CFG2. With SSM=1, SSI=0 means
     * NSS is internally low; the hardware silently refuses MASTER=1 in that
     * state (it would be an instant mode-fault). Mirror the HAL ordering:
     * CR1.SSI=1 first, then CFG2 with MASTER. */
    SPI1_NS->CR1 = CR1_SSI; /* SSI=1, SPE=0 */

    /* CFG2: single write with all desired bits.
     *   bit 31 AFCNTR : peripheral always drives the AF pins (CLK stays at CPOL)
     *   bit 26 SSM    : software slave management (hardware ignores NSS pin)
     *   bit 25 CPOL   : clock idle high (Mode 3)
     *   bit 24 CPHA   : sample on second edge (Mode 3)
     *   bit 22 MASTER : master mode — generates SCK
     * COMM[1:0]=00 (full-duplex), LSBFRST=0 (MSB first): all left at 0. */
    SPI1_NS->CFG2 = (1U << 31U) /* AFCNTR */
                  | (1U << 26U) /* SSM    */
                  | (1U << 25U) /* CPOL   */
                  | (1U << 24U) /* CPHA   */
                  | (1U << 22U);/* MASTER */

    /* Enable SPI peripheral — CR1 SPE bit 0 */
    SPI1_NS->CR1 = CR1_SSI | CR1_SPE; /* SSI=1, SPE=1 */
}

spi_status_t spi1_transmit(const uint8_t *data, uint32_t size)
{
    if ((data == NULL) || (size == 0U))
    {
        return SPI_ERROR;
    }

    /* On H7/U5 SPI, TSIZE must be written while SPE=0 to reliably take effect.
     * Cycling SPE also flushes both FIFOs and clears any prior error state. */
    SPI1_NS->CR1 &= ~CR1_SPE;
    SPI1_NS->IFCR = 0x1FF8U;
    SPI1_NS->CR2 = size;
    SPI1_NS->CR1 |= CR1_SPE; /* SPE = 1 */

    /* Pre-load first byte before CSTART so TXFIFO is non-empty when clock starts */
    /* MISRA C:2012 Rule 11.3 deviation: byte-width FIFO access required by STM32U5 SPI hardware */
    *((__IO uint8_t *)&SPI1_NS->TXDR) = data[0];
    SPI1_NS->CR1 |= CR1_CSTART; /* CSTART */

#if DEBUG_ENABLED
    /* --- Register snapshot immediately after CSTART --- */
    {
        uint32_t cr1  = SPI1_NS->CR1;
        uint32_t sr   = SPI1_NS->SR;
        uint32_t cfg1 = SPI1_NS->CFG1;
        uint32_t cfg2 = SPI1_NS->CFG2;
        uint32_t cr2  = SPI1_NS->CR2;
        const char *hex = "0123456789ABCDEF";
        (void)uart_send_string("CR1=");
        for (int s = 28; s >= 0; s -= 4) (void)uart_send_char(hex[(cr1  >> s) & 0xF]);
        (void)uart_send_string(" CR2=");
        for (int s = 28; s >= 0; s -= 4) (void)uart_send_char(hex[(cr2  >> s) & 0xF]);
        (void)uart_send_string(" CFG1=");
        for (int s = 28; s >= 0; s -= 4) (void)uart_send_char(hex[(cfg1 >> s) & 0xF]);
        (void)uart_send_string(" CFG2=");
        for (int s = 28; s >= 0; s -= 4) (void)uart_send_char(hex[(cfg2 >> s) & 0xF]);
        (void)uart_send_string(" SR=");
        for (int s = 28; s >= 0; s -= 4) (void)uart_send_char(hex[(sr   >> s) & 0xF]);
        (void)uart_send_string("\r\n");
    }
    (void)uart_send_string("spi_tx: cstart\r\n");
#endif

    uint32_t i = 1U;
    while (i < size)
    {
        uint32_t timeout = 1000000U;
        while (!(SPI1_NS->SR & SR_TXP) && (--timeout != 0U))
        {
        }
        if (!timeout)
        {
#if DEBUG_ENABLED
            (void)uart_send_string("spi_tx: TXP timeout\r\n");
#endif
            return SPI_TIMEOUT;
        }
        /* MISRA C:2012 Rule 11.3 deviation: byte-width FIFO access required by STM32U5 SPI hardware */
        *((__IO uint8_t *)&SPI1_NS->TXDR) = data[i];
        i++;
    }

#if DEBUG_ENABLED
    (void)uart_send_string("spi_tx: bytes written, wait EOT\r\n");
#endif

    {
        uint32_t timeout = 1000000U;
        while (!(SPI1_NS->SR & SR_EOT) && (--timeout != 0U))
        {
        }
        if (!timeout)
        {
#if DEBUG_ENABLED
            (void)uart_send_string("spi_tx: EOT timeout\r\n");
#endif
            return SPI_TIMEOUT;
        }
    }

#if DEBUG_ENABLED
    (void)uart_send_string("spi_tx: done\r\n");
#endif
    SPI1_NS->IFCR = 0x1FF8U;

    return SPI_OK;
}

spi_status_t spi1_receive(uint8_t *data, uint32_t size)
{
    if ((data == NULL) || (size == 0U))
    {
        return SPI_ERROR;
    }

    /* Same TSIZE/SPE sequence as spi1_transmit — TSIZE must be written while SPE=0 */
    SPI1_NS->CR1 &= ~CR1_SPE;
    SPI1_NS->IFCR = 0x1FF8U;
    SPI1_NS->CR2 = size;
    SPI1_NS->CR1 |= CR1_SPE; /* SPE = 1 */

    /* Pre-load first dummy byte before CSTART so TXFIFO is non-empty when clock starts */
    /* MISRA C:2012 Rule 11.3 deviation: byte-width FIFO access required by STM32U5 SPI hardware */
    *((__IO uint8_t *)&SPI1_NS->TXDR) = 0x00U;
    SPI1_NS->CR1 |= CR1_CSTART; /* CSTART */

    uint32_t remaining = size;
    while (remaining)
    {
        uint32_t timeout = 1000000U;
        while (!(SPI1_NS->SR & SR_RXP) && (--timeout != 0U))
        {
        }
        if (!timeout)
        {
#if DEBUG_ENABLED
            (void)uart_send_string("spi_rx: RXP timeout\r\n");
#endif
            return SPI_TIMEOUT;
        }
        /* MISRA C:2012 Rule 11.3 deviation: byte-width FIFO access required by STM32U5 SPI hardware */
        *data++ = *((__IO uint8_t *)&SPI1_NS->RXDR);
        remaining--;

        /* Queue next dummy byte to keep pipeline full (skip after last byte) */
        if (remaining > 0U)
        {
            timeout = 1000000U;
            while (!(SPI1_NS->SR & SR_TXP) && (--timeout != 0U))
            {
            }
            if (!timeout)
            {
#if DEBUG_ENABLED
                (void)uart_send_string("spi_rx: TXP timeout\r\n");
#endif
                return SPI_TIMEOUT;
            }
            /* MISRA C:2012 Rule 11.3 deviation: byte-width FIFO access required by STM32U5 SPI hardware */
            *((__IO uint8_t *)&SPI1_NS->TXDR) = 0x00U;
        }
    }

    {
        uint32_t timeout = 1000000U;
        while (!(SPI1_NS->SR & SR_EOT) && (--timeout != 0U))
        {
        }
        if (!timeout)
        {
#if DEBUG_ENABLED
            (void)uart_send_string("spi_rx: EOT timeout\r\n");
#endif
            return SPI_TIMEOUT;
        }
    }

    SPI1_NS->IFCR = 0x1FF8U;

    return SPI_OK;
}

void cs_enable(void)
{
    GPIOD_NS->BRR = (1U << 14U);
    __DSB(); /* flush AHB2 write before APB2 CSTART can fire */
}

void cs_disable(void)
{
    GPIOD_NS->BSRR = (1U << 14U);
    __DSB();
}

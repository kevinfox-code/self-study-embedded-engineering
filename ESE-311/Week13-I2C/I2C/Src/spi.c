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
        SSM bit 26 for software slave management (SSM=1 means software controls the NSS pin, SSM=0 means hardware controls the NSS pin)
   
    SPI Configuration Register 1 (SPI_CFG1):
        MBR[2:0] bits 2:0 for baud rate control

    SPI Control Register 1 (SPI_CR1) bits:
        SSI bit 12 is value to be forced onto the NSS pin when SSM=1 (software slave management enabled)


    SPI Status Register (SPI_SR) bits:
        RXWNE bit 15 for rxFIFO not empty (RXWNE=1 at least 4 bytes in buffer)
        
    SPI Data Register (SPI_DR) bits:
*/
#include "spi.h"

#define SPI1EN (1U << 12) // bit 12 for SPI1EN in RCC_APB2ENR
#define GPIODEN (1U << 3) // bit 3 for GPIODEN in RCC_AHB2ENR1
#define GPIOAEN (1U << 0) // bit 0 for GPIOAEN in RCC_AHB2ENR1
#define SR_TXP    (1U<<1)    /* Tx-packet space available  */
#define SR_RXP    (1U<<0)    /* Rx-packet available        */
#define SR_TXC    (1U<<12)   /* TxFIFO transmission complete */

void spi_gpio_init(void)
{
    /* Enable clock access to GPIOA
     * NOTE: On the U575, GPIO clocks are on AHB2ENR1, not AHB1ENR */
    RCC_NS->AHB2ENR1 |= GPIOAEN;

    /* Set PA5, PA6, PA7 mode to alternate function */
    /* PA5 */
    GPIOA_NS->MODER &= ~(1U<<10);
    GPIOA_NS->MODER |=  (1U<<11);
    /* PA6 */
    GPIOA_NS->MODER &= ~(1U<<12);
    GPIOA_NS->MODER |=  (1U<<13);
    /* PA7 */
    GPIOA_NS->MODER &= ~(1U<<14);
    GPIOA_NS->MODER |=  (1U<<15);

    /* Set PD14 as output pin (chip select) */
    RCC_NS->AHB2ENR1 |= GPIODEN;
    /* PD14 MODER bits: [29:28] -> set bit28=1, bit29=0 for General Purpose Output (01) */
    GPIOD_NS->MODER |=  (1U<<28);
    GPIOD_NS->MODER &= ~(1U<<29);

    /* Set PA5, PA6, PA7 alternate function to AF5 (SPI1)
     * AF5 = 0101 — same mapping as the F4 family */
    /* PA5 */
    GPIOA_NS->AFR[0] |=  (1U<<20);
    GPIOA_NS->AFR[0] &= ~(1U<<21);
    GPIOA_NS->AFR[0] |=  (1U<<22);
    GPIOA_NS->AFR[0] &= ~(1U<<23);
    /* PA6 */
    GPIOA_NS->AFR[0] |=  (1U<<24);
    GPIOA_NS->AFR[0] &= ~(1U<<25);
    GPIOA_NS->AFR[0] |=  (1U<<26);
    GPIOA_NS->AFR[0] &= ~(1U<<27);
    /* PA7 */
    GPIOA_NS->AFR[0] |=  (1U<<28);
    GPIOA_NS->AFR[0] &= ~(1U<<29);
    GPIOA_NS->AFR[0] |=  (1U<<30);
    GPIOA_NS->AFR[0] &= ~(1U<<31);
}

void spi1_config(void)
{
    /* Enable clock access to SPI1 module (APB2ENR bit 12 — same as F4) */
    RCC_NS->APB2ENR |= SPI1EN;

    /* --- All configuration must be done before setting SPE --- */

    /* Set baud rate prescaler to kernel clock / 4
     * MBR[2:0] in CFG1 bits [30:28]: 001 = /4 */
    SPI1_NS->CFG1 |=  (1U<<28);   /* MBR bit 28 = 1 */
    SPI1_NS->CFG1 &= ~(1U<<29);   /* MBR bit 29 = 0 */
    SPI1_NS->CFG1 &= ~(1U<<30);   /* MBR bit 30 = 0 */

    /* Set 8-bit data size
     * DSIZE[4:0] in CFG1 bits [4:0]: 00111 = 8 bits */
    SPI1_NS->CFG1 |= (7U<<0);

    /* Set FIFO threshold to 1 data frame (FTHLV = 0000 in bits [8:5]) */
    SPI1_NS->CFG1 &= ~(0xFU<<5);

    /* Set CPOL = 1 (clock idle high) — CFG2 bit 25 */
    SPI1_NS->CFG2 |= (1U<<25);

    /* Set CPHA = 1 (sample on second edge) — CFG2 bit 24 */
    SPI1_NS->CFG2 |= (1U<<24);

    /* Enable full-duplex mode — CFG2 COMM[1:0] bits [18:17] = 00 */
    SPI1_NS->CFG2 &= ~(3U<<17);

    /* Set MSB first — CFG2 LSBFRST bit 23 = 0 */
    SPI1_NS->CFG2 &= ~(1U<<23);

    /* Set master mode — CFG2 MASTER bit 22 = 1 */
    SPI1_NS->CFG2 |= (1U<<22);

    /* Enable software slave management — CFG2 SSM bit 26 = 1 */
    SPI1_NS->CFG2 |= (1U<<26);

    /* Disable NSS pulse management — CFG2 SSOM bit 30 = 0 */
    SPI1_NS->CFG2 &= ~(1U<<30);

    /* Set SSI = 1 to prevent mode fault in master mode — CR1 bit 12 */
    SPI1_NS->CR1 |= (1U<<12);

    /* Enable SPI peripheral — CR1 SPE bit 0 */
    SPI1_NS->CR1 |= (1U<<0);
}

void spi1_transmit(uint8_t *data, uint32_t size)
{
    uint32_t i = 0;

    /* Start the master transfer */
    SPI1_NS->CR1 |= (1U<<9);   /* CSTART */

    while(i < size)
    {
        /* Wait until TXP is set (space available in TxFIFO) */
        while(!(SPI1_NS->SR & SR_TXP)){}

        /* Write one byte to TXDR using byte-width access */
        *((__IO uint8_t*)&SPI1_NS->TXDR) = data[i];
        i++;
    }

    /* Wait until TxFIFO is empty and bus is idle (TXC flag) */
    while(!(SPI1_NS->SR & SR_TXC)){}

    /* Clear the overrun flag via IFCR (write 1 to OVRC, bit 6) */
    SPI1_NS->IFCR |= (1U<<6);
}

void spi1_receive(uint8_t *data, uint32_t size)
{
    /* Start the master transfer */
    SPI1_NS->CR1 |= (1U<<9);   /* CSTART */

    while(size)
    {
        /* Send dummy byte to generate clock pulses */
        *((__IO uint8_t*)&SPI1_NS->TXDR) = 0;

        /* Wait until RXP is set (data available in RxFIFO) */
        while(!(SPI1_NS->SR & SR_RXP)){}

        /* Read one byte from RXDR using byte-width access */
        *data++ = *((__IO uint8_t*)&SPI1_NS->RXDR);
        size--;
    }

    /* Wait until bus is idle, then clear overrun flag */
    while(!(SPI1_NS->SR & SR_TXC)){}
    SPI1_NS->IFCR |= (1U<<6);
}

void cs_enable(void)
{
    GPIOD_NS->ODR &= ~(1U<<14);
}

void cs_disable(void)
{
    GPIOD_NS->ODR |= (1U<<14);
}

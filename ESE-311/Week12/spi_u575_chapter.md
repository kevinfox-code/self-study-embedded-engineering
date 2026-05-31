# The STM32U5 SPI Peripherals

As with other peripherals, STM32 microcontrollers often include several SPI peripherals; the number varies depending on the specific model. The STM32U575 microcontroller has three SPI peripherals, namely the following:

- SPI1
- SPI2
- SPI3

SPI1 and SPI2 are full-featured instances, while SPI3 is a limited-featured instance. The key difference is that SPI1 and SPI2 support configurable data sizes from 4 to 32 bits with a 16×8-byte FIFO, while SPI3 supports only 8-bit or 16-bit data with a smaller 8×8-byte FIFO.

## Key Features

Here are some of the key features:

- **Full-duplex, half-duplex, and simplex communication:** Supports simultaneous two-way communication (full-duplex), one-way communication (simplex), or bidirectional communication over a single data line (half-duplex)
- **Master/slave configuration:** Each SPI peripheral can be configured as either a master or a slave device, and multimaster topologies are supported
- **Flexible data size:** SPI1 and SPI2 support data sizes from 4 to 32 bits; SPI3 supports 8 or 16 bits
- **High-speed communication:** Capable of operating at speeds up to 80 MHz in master mode (kernel clock / 2) on a 160 MHz device
- **FIFO-based data buffering:** Dedicated transmit (SPI_TXDR) and receive (SPI_RXDR) data registers backed by hardware FIFOs, with configurable threshold levels
- **Direct Memory Access (DMA) support:** DMA support for efficient data transfer without CPU intervention
- **NSS pin management:** Hardware or software management of the NSS pin, with configurable polarity and timing for multi-slave configurations
- **Cyclic Redundancy Check (CRC) calculation:** Built-in hardware CRC calculation for data integrity verification
- **Dual clock domain:** The peripheral kernel clock is independent from the APB bus clock, enabling autonomous operation in Stop modes

## Key SPI Registers

The STM32U575 uses a newer SPI peripheral IP than the F4 family. The register set is significantly redesigned: configuration is split across dedicated registers, and the transmit and receive data registers are now separate. To get SPI up and running we need to configure several registers. Let's break down the main registers, starting with the Control Register 1.

### SPI Control Register 1 (SPI_CR1)

On the U575, SPI_CR1 is a runtime control register rather than a configuration register. Most of the mode, format, and clock settings that lived in CR1 on the F4 have moved to SPI_CFG1 and SPI_CFG2. Key bits in SPI_CR1 include the following:

- **SPE (bit 0):** Enables the SPI peripheral. Set to 1 to activate SPI. Configuration registers (CFG1, CFG2) are write-protected while SPE is set, so all configuration must be done before enabling.
- **MASRX (bit 8):** Master automatic suspend on RxFIFO full. When set, the master pauses the clock before an overrun occurs.
- **CSTART (bit 9):** Master transfer start. In master mode with TSIZE = 0 (continuous transfer), write 1 to this bit to begin or continue the transfer. It is cleared by hardware when end-of-transfer (EOT) is detected.
- **CSUSP (bit 10):** Master suspend request. Writing 1 suspends the transfer at the end of the current frame.
- **SSI (bit 12):** Internal slave select level. When software slave management is enabled (SSM = 1 in CFG2), this bit drives the internal NSS signal. Set SSI = 1 in master mode to prevent a spurious mode fault.

### SPI Configuration Register 1 (SPI_CFG1)

SPI_CFG1 holds data-path and clock configuration. It is write-protected while SPE = 1. Key bits include the following:

- **DSIZE[4:0] (bits 4:0):** Number of bits per data frame. For 8-bit transfers, write 0b00111 (decimal 7). For 16-bit, write 0b01111 (decimal 15).
- **FTHLV[3:0] (bits 8:5):** FIFO threshold level — the number of data frames that constitute one "packet." For simple single-byte polling, set to 0000 (1 data frame).
- **TXDMAEN (bit 15) / RXDMAEN (bit 14):** Enable DMA requests for the transmit and receive paths respectively.
- **MBR[2:0] (bits 30:28):** Master baud rate prescaler. Divides the SPI kernel clock:
  - 000: kernel clock / 2
  - 001: kernel clock / 4
  - 010: kernel clock / 8
  - 011: kernel clock / 16
  - (and so on, doubling each step up to /256)

### SPI Configuration Register 2 (SPI_CFG2)

SPI_CFG2 holds the communication protocol and pin configuration. It is write-protected while SPE = 1. Key bits include the following:

- **CPHA (bit 24):** Clock phase. 0 = data sampled on the first clock edge; 1 = data sampled on the second clock edge.
- **CPOL (bit 25):** Clock polarity. 0 = clock idle low; 1 = clock idle high.
- **SSM (bit 26):** Software slave management. Setting this bit to 1 enables software control of the internal NSS signal via the SSI bit in CR1.
- **LSBFRST (bit 23):** Data frame format. 0 = MSB transmitted first; 1 = LSB transmitted first.
- **MASTER (bit 22):** SPI role. 1 = master; 0 = slave.
- **COMM[1:0] (bits 18:17):** Communication mode. 00 = full-duplex; 01 = simplex transmit; 10 = simplex receive; 11 = half-duplex.
- **SP[2:0] (bits 21:19):** Serial protocol. 000 = Motorola (standard SPI); 001 = TI mode.
- **SSOE (bit 29):** NSS output enable (hardware NSS management).

### SPI Status Register (SPI_SR)

The SPI_SR register provides real-time status updates. On the U575, several flag names have changed compared to the F4. Key bits include the following:

- **RXP (bit 0):** Rx-packet available. Set when the RxFIFO contains at least one complete data packet ready to read. This replaces the F4's RXNE flag.
- **TXP (bit 1):** Tx-packet space available. Set when the TxFIFO has enough room for at least one data packet. This replaces the F4's TXE flag.
- **EOT (bit 3):** End of transfer. Set when the full transfer defined by TSIZE is complete.
- **UDR (bit 5):** Underrun. Set when the slave transmitter did not have data ready.
- **OVR (bit 6):** Overrun. Set when incoming data was not read in time.
- **CRCE (bit 7):** CRC error detected.
- **MODF (bit 9):** Mode fault, typically from an NSS conflict.
- **TXC (bit 12):** TxFIFO transmission complete. Set when the TxFIFO is empty and the bus is idle. Use this to confirm all data has left the shift register before deasserting the CS pin. This replaces the role of the F4's BSY flag.

> **Important:** On the U575, status flags are **not** cleared by reading the data registers. They must be cleared by writing 1 to the corresponding bit in the **SPI_IFCR** (interrupt/status flags clear register).

### SPI Transmit and Receive Data Registers (SPI_TXDR / SPI_RXDR)

Unlike the F4, which used a single SPI_DR register for both transmit and receive, the U575 has separate registers:

- **SPI_TXDR (offset 0x020):** Write-only transmit data register. Writing to this register pushes data into the TxFIFO. For 8-bit transfers, access it as a byte (`uint8_t` pointer) to avoid inadvertently pushing multiple bytes.
- **SPI_RXDR (offset 0x030):** Read-only receive data register. Reading from this register pops data from the RxFIFO. Again, use byte-width access for 8-bit transfers.

With these registers in mind, we're now ready to develop the SPI driver. Let's jump into that in the next section.

---

## Developing the SPI Driver

Create a copy of your previous project in your IDE and rename this copied project to SPI. Next, create a new file named `spi.c` in the `Src` folder and another file named `spi.h` in the `Inc` folder. Populate your `spi.c` file with the following code:

```c
#include "spi.h"

#define SPI1EN    (1U<<12)   /* APB2ENR bit 12  */
#define GPIOAEN   (1U<<0)    /* AHB2ENR1 bit 0  */

/* SR flag bits (U575) */
#define SR_TXP    (1U<<1)    /* Tx-packet space available  */
#define SR_RXP    (1U<<0)    /* Rx-packet available        */
#define SR_TXC    (1U<<12)   /* TxFIFO transmission complete */

void spi_gpio_init(void)
{
    /* Enable clock access to GPIOA
     * NOTE: On the U575, GPIO clocks are on AHB2ENR1, not AHB1ENR */
    RCC->AHB2ENR1 |= GPIOAEN;

    /* Set PA5, PA6, PA7 mode to alternate function */
    /* PA5 */
    GPIOA->MODER &= ~(1U<<10);
    GPIOA->MODER |=  (1U<<11);
    /* PA6 */
    GPIOA->MODER &= ~(1U<<12);
    GPIOA->MODER |=  (1U<<13);
    /* PA7 */
    GPIOA->MODER &= ~(1U<<14);
    GPIOA->MODER |=  (1U<<15);

    /* Set PA9 as output pin (chip select) */
    GPIOA->MODER |=  (1U<<18);
    GPIOA->MODER &= ~(1U<<19);

    /* Set PA5, PA6, PA7 alternate function to AF5 (SPI1)
     * AF5 = 0101 — same mapping as the F4 family */
    /* PA5 */
    GPIOA->AFR[0] |=  (1U<<20);
    GPIOA->AFR[0] &= ~(1U<<21);
    GPIOA->AFR[0] |=  (1U<<22);
    GPIOA->AFR[0] &= ~(1U<<23);
    /* PA6 */
    GPIOA->AFR[0] |=  (1U<<24);
    GPIOA->AFR[0] &= ~(1U<<25);
    GPIOA->AFR[0] |=  (1U<<26);
    GPIOA->AFR[0] &= ~(1U<<27);
    /* PA7 */
    GPIOA->AFR[0] |=  (1U<<28);
    GPIOA->AFR[0] &= ~(1U<<29);
    GPIOA->AFR[0] |=  (1U<<30);
    GPIOA->AFR[0] &= ~(1U<<31);
}
```

Next, we have the function for configuring the SPI parameters:

```c
void spi1_config(void)
{
    /* Enable clock access to SPI1 module (APB2ENR bit 12 — same as F4) */
    RCC->APB2ENR |= SPI1EN;

    /* --- All configuration must be done before setting SPE --- */

    /* Set baud rate prescaler to kernel clock / 4
     * MBR[2:0] in CFG1 bits [30:28]: 001 = /4 */
    SPI1->CFG1 |=  (1U<<28);   /* MBR bit 28 = 1 */
    SPI1->CFG1 &= ~(1U<<29);   /* MBR bit 29 = 0 */
    SPI1->CFG1 &= ~(1U<<30);   /* MBR bit 30 = 0 */

    /* Set 8-bit data size
     * DSIZE[4:0] in CFG1 bits [4:0]: 00111 = 8 bits */
    SPI1->CFG1 |= (7U<<0);

    /* Set FIFO threshold to 1 data frame (FTHLV = 0000 in bits [8:5]) */
    SPI1->CFG1 &= ~(0xFU<<5);

    /* Set CPOL = 1 (clock idle high) — CFG2 bit 25 */
    SPI1->CFG2 |= (1U<<25);

    /* Set CPHA = 1 (sample on second edge) — CFG2 bit 24 */
    SPI1->CFG2 |= (1U<<24);

    /* Enable full-duplex mode — CFG2 COMM[1:0] bits [18:17] = 00 */
    SPI1->CFG2 &= ~(3U<<17);

    /* Set MSB first — CFG2 LSBFRST bit 23 = 0 */
    SPI1->CFG2 &= ~(1U<<23);

    /* Set master mode — CFG2 MASTER bit 22 = 1 */
    SPI1->CFG2 |= (1U<<22);

    /* Enable software slave management — CFG2 SSM bit 26 = 1 */
    SPI1->CFG2 |= (1U<<26);

    /* Set SSI = 1 to prevent mode fault in master mode — CR1 bit 12 */
    SPI1->CR1 |= (1U<<12);

    /* Enable SPI peripheral — CR1 SPE bit 0 */
    SPI1->CR1 |= (1U<<0);
}
```

```c
void spi1_transmit(uint8_t *data, uint32_t size)
{
    uint32_t i = 0;

    /* Start the master transfer */
    SPI1->CR1 |= (1U<<9);   /* CSTART */

    while(i < size)
    {
        /* Wait until TXP is set (space available in TxFIFO) */
        while(!(SPI1->SR & SR_TXP)){}

        /* Write one byte to TXDR using byte-width access */
        *((__IO uint8_t*)&SPI1->TXDR) = data[i];
        i++;
    }

    /* Wait until TxFIFO is empty and bus is idle (TXC flag) */
    while(!(SPI1->SR & SR_TXC)){}

    /* Clear the overrun flag via IFCR (write 1 to OVRC, bit 6) */
    SPI1->IFCR |= (1U<<6);
}
```

Here is the function for receiving data:

```c
void spi1_receive(uint8_t *data, uint32_t size)
{
    /* Start the master transfer */
    SPI1->CR1 |= (1U<<9);   /* CSTART */

    while(size)
    {
        /* Send dummy byte to generate clock pulses */
        *((__IO uint8_t*)&SPI1->TXDR) = 0;

        /* Wait until RXP is set (data available in RxFIFO) */
        while(!(SPI1->SR & SR_RXP)){}

        /* Read one byte from RXDR using byte-width access */
        *data++ = *((__IO uint8_t*)&SPI1->RXDR);
        size--;
    }
}
```

Finally, the functions for controlling the CS pin are unchanged:

```c
void cs_enable(void)
{
    GPIOA->ODR &= ~(1U<<9);
}

void cs_disable(void)
{
    GPIOA->ODR |= (1U<<9);
}
```

---

## Defined Macros

Let's break down the meaning of the macros and their functions:

```c
#define SPI1EN   (1U<<12)
#define GPIOAEN  (1U<<0)
#define SR_TXP   (1U<<1)
#define SR_RXP   (1U<<0)
#define SR_TXC   (1U<<12)
```

- **SPI1EN:** `(1U<<12)`, sets bit 12. Enables the clock for SPI1 in RCC_APB2ENR. The bit position is the same as the F411.
- **GPIOAEN:** `(1U<<0)`, sets bit 0. Enables the clock for GPIOA in RCC_AHB2ENR1. Note the register name change from the F4's RCC_AHB1ENR.
- **SR_TXP:** `(1U<<1)`. Tx-packet space available flag in SPI_SR — replaces the F4's TXE flag. The bit position happens to be the same (bit 1).
- **SR_RXP:** `(1U<<0)`. Rx-packet available flag in SPI_SR — replaces the F4's RXNE flag. Again the same bit position (bit 0).
- **SR_TXC:** `(1U<<12)`. TxFIFO transmission complete flag — replaces the F4's BSY flag. The bit position is **different** (bit 12 vs. bit 7 on the F4).

---

## GPIO Initialization for SPI

```c
RCC->AHB2ENR1 |= GPIOAEN;
```

This enables the clock for GPIOA. On the U575, GPIO peripheral clocks live in **RCC_AHB2ENR1** (offset 0x08C), not RCC_AHB1ENR as on the F4. The bit position for GPIOAEN is still bit 0.

The pin mode and alternate function configuration that follows is identical to the F4 code. PA5 (SCK), PA6 (MISO), and PA7 (MOSI) are all set to alternate function mode with AF5, which maps to SPI1 on both devices. PA9 is configured as a general-purpose output for the CS line.

---

## SPI1 Configuration

```c
RCC->APB2ENR |= SPI1EN;
```

This enables the clock for SPI1 via bit 12 of APB2ENR — identical to the F411.

The biggest difference from the F4 driver is that configuration is now split across SPI_CFG1 and SPI_CFG2 rather than all living in SPI_CR1. **All configuration registers must be set before SPE is asserted.**

```c
SPI1->CFG1 |=  (1U<<28);
SPI1->CFG1 &= ~(1U<<29);
SPI1->CFG1 &= ~(1U<<30);
```

These lines configure the baud rate prescaler. The MBR[2:0] field occupies bits [30:28] of SPI_CFG1. Setting it to `001` divides the SPI kernel clock by 4. This is the CFG1 equivalent of the F4's BR[2:0] field in CR1.

```c
SPI1->CFG1 |= (7U<<0);
```

Sets DSIZE[4:0] to `00111` (decimal 7), selecting 8-bit data frames. On the F4 this was a single bit (DFF) in CR1; on the U575 it is a 5-bit field in CFG1 supporting widths from 4 to 32 bits.

```c
SPI1->CFG2 |= (1U<<25);   /* CPOL */
SPI1->CFG2 |= (1U<<24);   /* CPHA */
```

Sets clock polarity and phase. These bits have moved from CR1 on the F4 to CFG2 on the U575, but their meaning is identical: CPOL=1 means the clock idles high, CPHA=1 means data is sampled on the second clock edge.

```c
SPI1->CFG2 &= ~(3U<<17);  /* COMM = 00, full-duplex */
SPI1->CFG2 &= ~(1U<<23);  /* LSBFRST = 0, MSB first */
SPI1->CFG2 |=  (1U<<22);  /* MASTER = 1             */
SPI1->CFG2 |=  (1U<<26);  /* SSM = 1                */
```

These lines configure the communication mode, bit order, master role, and software slave management — again moved from CR1 to CFG2 on the U575, but functionally equivalent.

```c
SPI1->CR1 |= (1U<<12);   /* SSI = 1 */
SPI1->CR1 |= (1U<<0);    /* SPE = 1 */
```

SSI is now in CR1 (bit 12) on the U575, just as it was on the F4. SPE is also bit 0 of CR1 on both devices. Configuration is locked once SPE is set.

---

## Transmitting Data with SPI

```c
SPI1->CR1 |= (1U<<9);   /* CSTART */
```

On the U575, the master does not start clocking automatically on the first write to TXDR. You must set the CSTART bit to begin the transfer. There is no equivalent step on the F4.

```c
while(!(SPI1->SR & SR_TXP)){}
```

Waits until there is space in the TxFIFO. SR_TXP (bit 1) is the U575 equivalent of the F4's TXE flag and occupies the same bit position.

```c
*((__IO uint8_t*)&SPI1->TXDR) = data[i];
```

Writes one byte to the transmit data register. Because SPI1 and SPI2 have 32-bit-wide TXDR registers, a plain 32-bit write would push four bytes into the FIFO at once. Casting to `__IO uint8_t*` forces a byte-width bus transaction, pushing exactly one byte. There is no equivalent concern on the F4 since SPI_DR was effectively 8 bits wide in 8-bit mode.

```c
while(!(SPI1->SR & SR_TXC)){}
```

Waits until the TxFIFO is empty **and** the shift register has finished clocking out the last bit. SR_TXC (bit 12) serves the same purpose as polling the BSY flag (bit 7) on the F4, but note the different bit position.

```c
SPI1->IFCR |= (1U<<6);   /* Clear OVRC */
```

Clears the overrun flag. On the F4, reading SPI_DR and SPI_SR was sufficient to clear OVR. On the U575, flags are cleared by writing 1 to the corresponding bit in SPI_IFCR. Bit 6 of IFCR is OVRC, which clears the OVR flag in SPI_SR.

---

## SPI Data Reception

```c
SPI1->CR1 |= (1U<<9);   /* CSTART */
```

Same as transmit — the master transfer must be explicitly started.

```c
*((__IO uint8_t*)&SPI1->TXDR) = 0;
```

Sends a dummy byte to generate the clock pulses needed to shift in data from the slave. The byte-width cast is required here for the same reason as in the transmit function.

```c
while(!(SPI1->SR & SR_RXP)){}
```

Waits until the RxFIFO contains at least one complete data packet. SR_RXP (bit 0) replaces the F4's RXNE flag at the same bit position.

```c
*data++ = *((__IO uint8_t*)&SPI1->RXDR);
```

Reads one byte from the receive data register using byte-width access.

---

## CS Management

The CS pin control functions are identical to the F4 version, since PA9 is configured as a standard GPIO output on both devices:

```c
GPIOA->ODR &= ~(1U<<9);   /* cs_enable  — pull low  */
GPIOA->ODR |=  (1U<<9);   /* cs_disable — pull high */
```

---

## The Header File

```c
#ifndef SPI_H_
#define SPI_H_

#include "stm32u5xx.h"   /* Changed from stm32f4xx.h */
#include <stdint.h>

void spi_gpio_init(void);
void spi1_config(void);
void spi1_transmit(uint8_t *data, uint32_t size);
void spi1_receive(uint8_t *data, uint32_t size);
void cs_enable(void);
void cs_disable(void);

#endif
```

The only change to the header is the include file: `stm32u5xx.h` replaces `stm32f4xx.h` to pull in the U575 register definitions and peripheral base addresses.

---

## Summary of Key Differences from the F411

| Aspect | STM32F411 | STM32U575 |
|---|---|---|
| SPI instances | 5 (SPI1–SPI5) | 3 (SPI1–SPI3) |
| GPIOA clock register | `RCC->AHB1ENR` | `RCC->AHB2ENR1` |
| SPI1 clock register | `RCC->APB2ENR` bit 12 | `RCC->APB2ENR` bit 12 ✓ |
| CPOL / CPHA location | `SPI_CR1` bits 1, 0 | `SPI_CFG2` bits 25, 24 |
| Master mode bit | `SPI_CR1` bit 2 | `SPI_CFG2` bit 22 |
| Baud rate field | `SPI_CR1` BR[2:0] bits 5:3 | `SPI_CFG1` MBR[2:0] bits 30:28 |
| Data size field | `SPI_CR1` DFF bit 11 | `SPI_CFG1` DSIZE[4:0] bits 4:0 |
| SSM / SSI bits | `SPI_CR1` bits 9, 8 | SSM → `SPI_CFG2` bit 26; SSI → `SPI_CR1` bit 12 |
| Full-duplex mode | `SPI_CR1` BIDIMODE=0 | `SPI_CFG2` COMM=00 |
| Transfer start | Automatic on first write | Must set `SPI_CR1` CSTART |
| TX ready flag | `SPI_SR` TXE (bit 1) | `SPI_SR` TXP (bit 1) |
| RX ready flag | `SPI_SR` RXNE (bit 0) | `SPI_SR` RXP (bit 0) |
| Bus idle flag | `SPI_SR` BSY (bit 7) | `SPI_SR` TXC (bit 12) |
| Data register | `SPI_DR` (single, R/W) | `SPI_TXDR` (write) / `SPI_RXDR` (read) |
| Clear OVR flag | Read `SPI_DR`, read `SPI_SR` | Write 1 to `SPI_IFCR` bit 6 |
| Max master speed | 42 MHz | 80 MHz |

/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: STM32U575 I2C1 on PB8/PB9 (AF4), using the HSI16 kernel clock.
 */
#include "i2c.h"
#include "stm32u575xx.h"
#define POLL_LIMIT 100000U
#define CLEAR_FLAGS (I2C_ICR_STOPCF | I2C_ICR_NACKCF | I2C_ICR_BERRCF | \
                     I2C_ICR_ARLOCF | I2C_ICR_OVRCF)
static int initialized;

static i2c_status_t wait_flag(uint32_t mask, int set)
{
    for (uint32_t n = 0; n < POLL_LIMIT; ++n) {
        uint32_t status = I2C1_NS->ISR;
        if (status & I2C_ISR_ARLO) return I2C_ARBITRATION_LOST;
        if (status & I2C_ISR_BERR) return I2C_BUS_ERROR;
        if (status & I2C_ISR_OVR) return I2C_OVERRUN;
        if (status & I2C_ISR_NACKF) return I2C_NACK;
        if (((status & mask) != 0U) == set) return I2C_OK;
    }
    return I2C_TIMEOUT;
}

static i2c_status_t finish(i2c_status_t result)
{
    if (result != I2C_OK) {
        /* Never issue STOP after losing ownership of the bus. */
        if (result != I2C_ARBITRATION_LOST && (I2C1_NS->ISR & I2C_ISR_BUSY)) {
            I2C1_NS->CR2 |= I2C_CR2_STOP;
            for (uint32_t n = 0; n < POLL_LIMIT; ++n)
                if (I2C1_NS->ISR & I2C_ISR_STOPF) break;
        }
        /* Reset the transfer state, including a pending TX byte. A slave
         * physically holding SDA low still requires external recovery. */
        I2C1_NS->CR1 &= ~I2C_CR1_PE;
        I2C1_NS->CR2 = 0;
        I2C1_NS->CR1 |= I2C_CR1_PE;
    }
    I2C1_NS->ICR = CLEAR_FLAGS;
    return result;
}

static i2c_status_t begin(uint8_t address, const void *data, size_t count)
{
    if (!initialized || address > 0x7fU || !data || !count || count > 255U)
        return I2C_INVALID_ARGUMENT;
    /* A busy bus may belong to another master: do not force a STOP. */
    return wait_flag(I2C_ISR_BUSY, 0);
}

static uint32_t transfer(uint8_t address, size_t count)
{
    return ((uint32_t)address << 1) |
           ((uint32_t)count << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
}

i2c_status_t i2c1_init(uint32_t bus_hz)
{
    if (bus_hz != 100000U && bus_hz != 400000U) return I2C_INVALID_ARGUMENT;
    initialized = 0;
    RCC_NS->CR |= RCC_CR_HSION;
    uint32_t n;
    for (n = 0; n < POLL_LIMIT; ++n)
        if (RCC_NS->CR & RCC_CR_HSIRDY) break;
    if (n == POLL_LIMIT) return I2C_TIMEOUT;
    RCC_NS->AHB2ENR1 |= RCC_AHB2ENR1_GPIOBEN;
    RCC_NS->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    (void)RCC_NS->APB1ENR1;
    RCC_NS->APB1RSTR1 |= RCC_APB1RSTR1_I2C1RST;
    RCC_NS->APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;
    RCC_NS->CCIPR1 = (RCC_NS->CCIPR1 & ~RCC_CCIPR1_I2C1SEL) |
                        RCC_CCIPR1_I2C1SEL_1;
    GPIOB_NS->OTYPER |= (1U << 8) | (1U << 9);
    GPIOB_NS->PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB_NS->OSPEEDR |= (3U << 16) | (3U << 18);
    GPIOB_NS->AFR[1] = (GPIOB_NS->AFR[1] & ~0xffU) | 0x44U;
    GPIOB_NS->MODER = (GPIOB_NS->MODER & ~((3U << 16) | (3U << 18))) |
                       (2U << 16) | (2U << 18);
    I2C1_NS->CR1 = 0; /* Analog filter on, digital filter off. */
    /* HSI16: tI2CCLK=62.5 ns. Conservative timing, including edge/filter
     * overhead: standard mode PRESC=3, SCLDEL=4, SDADEL=2, H=19, L=19;
     * fast mode PRESC=0, SCLDEL=7, SDADEL=3, H=15, L=23.
     * Assumes rise/fall <=100 ns, analog filter 50..260 ns. */
    I2C1_NS->TIMINGR = bus_hz == 100000U ? 0x30421313U : 0x00730f17U;
    I2C1_NS->ICR = CLEAR_FLAGS;
    I2C1_NS->CR1 = I2C_CR1_PE;
    initialized = 1;
    return I2C_OK;
}

i2c_status_t i2c1_write(uint8_t address, const uint8_t *data, size_t count)
{
    i2c_status_t status = begin(address, data, count);
    if (status != I2C_OK) return status;
    I2C1_NS->CR2 = transfer(address, count) | I2C_CR2_AUTOEND;
    for (size_t i = 0; i < count; ++i) {
        status = wait_flag(I2C_ISR_TXIS, 1);
        if (status != I2C_OK) return finish(status);
        I2C1_NS->TXDR = data[i];
    }
    return finish(wait_flag(I2C_ISR_STOPF, 1));
}

i2c_status_t i2c1_read_register(uint8_t address, uint8_t reg,
                                 uint8_t *data, size_t count)
{
    i2c_status_t status = begin(address, data, count);
    if (status != I2C_OK) return status;
    I2C1_NS->CR2 = transfer(address, 1); /* No STOP before repeated START. */
    status = wait_flag(I2C_ISR_TXIS, 1);
    if (status != I2C_OK) return finish(status);
    I2C1_NS->TXDR = reg;
    status = wait_flag(I2C_ISR_TC, 1);
    if (status != I2C_OK) return finish(status);
    I2C1_NS->CR2 = transfer(address, count) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND;
    for (size_t i = 0; i < count; ++i) {
        status = wait_flag(I2C_ISR_RXNE, 1);
        if (status != I2C_OK) return finish(status);
        data[i] = (uint8_t)I2C1_NS->RXDR;
    }
    return finish(wait_flag(I2C_ISR_STOPF, 1));
}

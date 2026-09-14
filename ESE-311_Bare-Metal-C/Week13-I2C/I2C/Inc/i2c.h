/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: Polling I2C1 master API; addresses are unshifted 7-bit values.
 */
#ifndef I2C_H
#define I2C_H
#include <stdint.h>
#include <stddef.h>
typedef enum {
    I2C_OK = 0, I2C_INVALID_ARGUMENT, I2C_TIMEOUT, I2C_NACK,
    I2C_BUS_ERROR, I2C_ARBITRATION_LOST, I2C_OVERRUN, I2C_DEVICE_ID
} i2c_status_t;
/* Single caller, blocking transfers, 1..255 bytes; no DMA or interrupts. */
i2c_status_t i2c1_init(uint32_t bus_hz);
i2c_status_t i2c1_write(uint8_t address, const uint8_t *data, size_t count);
i2c_status_t i2c1_read_register(uint8_t address, uint8_t reg,
                                 uint8_t *data, size_t count);
#endif

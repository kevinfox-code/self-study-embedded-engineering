/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: ADXL345 I2C accelerometer API.
 */
#ifndef ADXL345_H
#define ADXL345_H
#include "i2c.h"
#ifndef ADXL345_I2C_ADDRESS
#define ADXL345_I2C_ADDRESS 0x53U /* ALT ADDRESS low; high selects 0x1d. */
#endif
#define ADXL345_REG_DEVID 0x00U
#define ADXL345_REG_BW_RATE 0x2cU
#define ADXL345_REG_POWER_CTL 0x2dU
#define ADXL345_REG_DATA_FORMAT 0x31U
#define ADXL345_REG_DATA_START 0x32U
#define ADXL345_G_PER_LSB 0.0039f

i2c_status_t adxl_init(void);
i2c_status_t adxl_write(uint8_t address, uint8_t value);
/* On failure, rxdata is unchanged. */
i2c_status_t adxl_read(uint8_t address, uint8_t *rxdata);
#endif

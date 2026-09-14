/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: ADXL345 identity check, full-resolution +/-4 g configuration and burst read.
 */
#include "adxl345.h"
#include <string.h>
i2c_status_t adxl_write(uint8_t address, uint8_t value)
{
    const uint8_t data[] = {address, value};
    return i2c1_write(ADXL345_I2C_ADDRESS, data, sizeof data);
}
i2c_status_t adxl_read(uint8_t address, uint8_t *rxdata)
{
    if (!rxdata) return I2C_INVALID_ARGUMENT;
    uint8_t data[6];
    i2c_status_t status = i2c1_read_register(ADXL345_I2C_ADDRESS, address, data, sizeof data);
    if (status == I2C_OK) memcpy(rxdata, data, sizeof data);
    return status;
}
i2c_status_t adxl_init(void)
{
    uint8_t id;
    i2c_status_t status = i2c1_read_register(ADXL345_I2C_ADDRESS,
                                           ADXL345_REG_DEVID, &id, 1);
    if (status != I2C_OK) return status;
    if (id != 0xe5U) return I2C_DEVICE_ID;
    status = adxl_write(ADXL345_REG_POWER_CTL, 0); /* Standby. */
    if (status != I2C_OK) return status;
    status = adxl_write(ADXL345_REG_DATA_FORMAT, 0x09U); /* FULL_RES, +/-4 g. */
    if (status != I2C_OK) return status;
    status = adxl_write(ADXL345_REG_BW_RATE, 0x0aU); /* 100 Hz. */
    if (status != I2C_OK) return status;
    return adxl_write(ADXL345_REG_POWER_CTL, 0x08U); /* Measurement mode. */
}

/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: Host tests for ADXL345 protocol and error propagation.
 */
#include "adxl345.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static uint8_t identity = 0xe5;
static int writes, fail_write = -1;
static i2c_status_t read_result = I2C_OK;
static const uint8_t expected[][2] = {{0x2d, 0}, {0x31, 9}, {0x2c, 10}, {0x2d, 8}};
i2c_status_t i2c1_write(uint8_t address, const uint8_t *data, size_t count)
{
    assert(address == ADXL345_I2C_ADDRESS && count == 2);
    assert(writes < 4 && memcmp(data, expected[writes], 2) == 0);
    return writes++ == fail_write ? I2C_NACK : I2C_OK;
}
i2c_status_t i2c1_read_register(uint8_t address, uint8_t reg, uint8_t *data, size_t count)
{
    assert(address == ADXL345_I2C_ADDRESS);
    if (reg == ADXL345_REG_DEVID) {
        assert(count == 1);
        *data = identity;
    } else {
        assert(reg == ADXL345_REG_DATA_START && count == 6);
        const uint8_t sample[] = {0xff, 0xff, 0, 0x80, 0xff, 0x7f};
        memcpy(data, sample, 6);
    }
    return read_result;
}
int main(void)
{
    assert(adxl_init() == I2C_OK && writes == 4);
    for (int i = 0; i < 4; ++i) {
        writes = 0; fail_write = i;
        assert(adxl_init() == I2C_NACK && writes == i + 1);
    }
    writes = 0; identity = 0;
    assert(adxl_init() == I2C_DEVICE_ID && writes == 0);
    read_result = I2C_TIMEOUT;
    assert(adxl_init() == I2C_TIMEOUT && writes == 0);
    uint8_t data[6] = {1, 2, 3, 4, 5, 6};
    const uint8_t original[] = {1, 2, 3, 4, 5, 6};
    assert(adxl_read(ADXL345_REG_DATA_START, data) == I2C_TIMEOUT);
    assert(memcmp(data, original, 6) == 0);
    assert(adxl_read(ADXL345_REG_DATA_START, NULL) == I2C_INVALID_ARGUMENT);
    read_result = I2C_OK;
    assert(adxl_read(ADXL345_REG_DATA_START, data) == I2C_OK);
    assert(data[0] == 0xff && data[3] == 0x80 && data[5] == 0x7f);
    puts("ADXL345 tests passed");
}

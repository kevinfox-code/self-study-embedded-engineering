/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: ADXL345 accelerometer driver API and register map for the SPI-attached sensor.
 */
#ifndef ADXL345_H
#define ADXL345_H

#include "spi.h"
#include <stdint.h>

#define ADXL345_REG_DEVID       (0x00) // Register address for device ID
#define ADXL345_REG_DATA_FORMAT (0x31) // Register address for data format (range settings)
#define ADXL345_REG_POWER_CTL   (0x2D) // Register address for power control (measurement mode)
#define ADXL345_REG_DATA_START  (0x32) // Register address for start of data

#define ADXL345_RANGE_4G          (0x01) // Range setting for ±4g
#define ADXL345_RESET             (0x00) // Reset value
#define ADXL345_MEASURE_BIT       (0x08) // Measure bit
#define ADXL345_MULTI_BYTE_ENABLE (0x40) // Multi-byte enable bit
#define ADXL345_READ_OPERATION    (0x80) // Read operation bit

void adxl_init(void);
void adxl_read(uint8_t address, uint8_t *rxdata);

#endif
/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: SPI1 master driver API — GPIO and peripheral init, polled transmit/receive, and software
 *              chip-select control.
 */
#ifndef SPI_H
#define SPI_H

#include "stm32u575xx.h"
#include <stdint.h>

typedef enum { SPI_OK = 0, SPI_TIMEOUT = 1, SPI_ERROR = 2 } spi_status_t;

void spi_gpio_init(void);
void spi1_config(void);
spi_status_t spi1_transmit(const uint8_t *data, uint32_t size);
spi_status_t spi1_receive(uint8_t *data, uint32_t size);
void cs_enable(void);
void cs_disable(void);

#endif /* SPI_H */

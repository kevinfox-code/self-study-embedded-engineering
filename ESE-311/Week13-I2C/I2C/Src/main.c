/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Application entry point — carried over from the Week 12 SPI project as the starting point
 *              for the I2C exercise; currently still drives the ADXL345 over SPI.
 */
#include "main.h"
#include "uart.h"
#include "gpio.h"
#include "systick.h"
#include "spi.h"
#include "adxl345.h"
#include <stdint.h>
#include <stdio.h>

int16_t accel_x, accel_y, accel_z;
float accel_x_g, accel_y_g, accel_z_g;

uint8_t data_buffer[6];

static void print_i16(int16_t v)
{
    char buf[6];
    int len = 0;
    uint16_t u;
    if (v < 0) {
        (void)uart_send_char('-');
        u = (uint16_t)(-(int32_t)v);
    } else {
        u = (uint16_t)v;
    }
    if (u == 0) { (void)uart_send_char('0'); return; }
    while (u) { buf[len++] = '0' + (u % 10); u /= 10; }
    for (int i = len - 1; i >= 0; i--) (void)uart_send_char(buf[i]);
}

static void blink(int count, uint32_t ms)
{
    for (int i = 0; i < count; i++)
    {
        blue_led_off();
        systick_msec_delay(ms);
        blue_led_on();
        systick_msec_delay(ms);
    }
}

int main(void)
{
    /* Checkpoint 0: MCU alive */
    led_init();
    blue_led_on();

    /* Checkpoint 1: SysTick + LED working */
    blink(1, 300);

    /* Checkpoint 2: uart_init done */
    (void)uart_init();
    blink(2, 200);

    /* Checkpoint 3: spi_gpio_init done */
    spi_gpio_init();
    cs_disable(); /* deassert CS immediately so ADXL345 gets a clean high->low edge */
    blink(3, 200);

    /* Checkpoint 4: spi1_config done — read SR immediately and signal result */
    spi1_config();
    uint32_t sr_after_config = SPI1_NS->SR;
    uint32_t txp_set = (sr_after_config & (1U << 1)); /* TXP = bit 1 */

    if (txp_set)
    {
        /* TXP is 1: fast blinks = SPI ready */
        blink(4, 100);
    }
    else
    {
        /* TXP is 0: slow blinks = SPI NOT ready, SR value encoded in blinks */
        blink(10, 500);
    }

    /* Checkpoint 5: full adxl_init (includes the adxl_write calls) */
    (void)uart_send_string("adxl_init start\r\n");
    adxl_init();
    (void)uart_send_string("adxl_init done\r\n");
    blink(5, 200);

    while (1)
    {
        adxl_read(ADXL345_REG_DATA_START, data_buffer);

        accel_x = (int16_t)((data_buffer[1] << 8) | data_buffer[0]);
        accel_y = (int16_t)((data_buffer[3] << 8) | data_buffer[2]);
        accel_z = (int16_t)((data_buffer[5] << 8) | data_buffer[4]);

        accel_x_g = accel_x * 0.004f;
        accel_y_g = accel_y * 0.004f;
        accel_z_g = accel_z * 0.004f;

        (void)uart_send_string("raw:");
        for (int i = 0; i < 6; i++) {
            (void)uart_send_char(' ');
            (void)uart_send_char("0123456789ABCDEF"[(data_buffer[i] >> 4) & 0xF]);
            (void)uart_send_char("0123456789ABCDEF"[data_buffer[i] & 0xF]);
        }
        (void)uart_send_string("\r\n");

        (void)uart_send_string("X:"); print_i16(accel_x);
        (void)uart_send_string(" Y:"); print_i16(accel_y);
        (void)uart_send_string(" Z:"); print_i16(accel_z);
        (void)uart_send_string("\r\n");

        blue_led_toggle();
        systick_msec_delay(500);
    }

    return 0;
}

void Error_Handler(void)
{
    while (1)
    {
        /* Hang in error */
    }
}

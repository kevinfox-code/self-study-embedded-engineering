/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Stream ADXL345 acceleration over UART using I2C1.
 */
#include "main.h"
#include "uart.h"
#include "gpio.h"
#include "systick.h"
#include "i2c.h"
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
    led_init();
    (void)uart_init();
    i2c_status_t status = i2c1_init(100000U);
    if (status != I2C_OK) {
        (void)uart_send_string("I2C initialization failed\r\n");
        Error_Handler();
    }
    while ((status = adxl_init()) != I2C_OK) {
        (void)uart_send_string("ADXL345 initialization error: ");
        print_i16((int16_t)status);
        (void)uart_send_string("\r\n");
        blink(2, 250);
    }
    (void)uart_send_string("ADXL345 I2C ready\r\n");
    systick_msec_delay(20);

    while (1)
    {
        status = adxl_read(ADXL345_REG_DATA_START, data_buffer);
        if (status != I2C_OK) {
            (void)uart_send_string("I2C read error: ");
            print_i16((int16_t)status);
            (void)uart_send_string("\r\n");
            systick_msec_delay(500);
            continue;
        }

        accel_x = (int16_t)((data_buffer[1] << 8) | data_buffer[0]);
        accel_y = (int16_t)((data_buffer[3] << 8) | data_buffer[2]);
        accel_z = (int16_t)((data_buffer[5] << 8) | data_buffer[4]);

        accel_x_g = accel_x * ADXL345_G_PER_LSB;
        accel_y_g = accel_y * ADXL345_G_PER_LSB;
        accel_z_g = accel_z * ADXL345_G_PER_LSB;

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

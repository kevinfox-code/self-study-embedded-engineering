/**
 * @file drv8323_transport.c
 * @brief Fills drv8323_transport_t with HAL-backed SPI/GPIO callbacks.
 *
 * Layer: C  Includes constants.h (via board_support.h).
 */

#include "constants.h"
#include "board_support.h"
#include "foc/drv8323.h"

static int s_xfer16(uint16_t tx, uint16_t *rx)
{
    return bsp_spi_xfer16(tx, rx);
}

static void s_delay_us(uint32_t us)
{
    bsp_delay_us(us);
}

static void s_enable_pin(bool on)
{
    bsp_gpio_drv_enable(on);
}

static bool s_nfault_pin(void)
{
    return bsp_gpio_drv_nfault();
}

static const drv8323_transport_t s_transport = {
    s_xfer16, s_delay_us, s_enable_pin, s_nfault_pin
};

const drv8323_transport_t *drv8323_transport_get(void)
{
    return &s_transport;
}

#include "adx1345.h"
#include "uart.h"

void adxl_read(uint8_t address, uint8_t *rxdata)
{
    /*Set read operation*/
    address |= ADXL345_READ_OPERATION;
    /*Enable multi-byte*/
    address |= ADXL345_MULTI_BYTE_ENABLE;
    /*Pull cs line low to enable slave*/
    cs_enable();
    /*Send address*/
    (void)spi1_transmit(&address, 1U);
    /*Read 6 bytes */
    (void)spi1_receive(rxdata, 6U);
    /*Pull cs line high to disable slave*/
    cs_disable();
}

void adxl_write(uint8_t address, uint8_t value)
{
    uint8_t data[2];
    /*Place address into buffer*/
    data[0] = address;
    /*Place data into buffer*/
    data[1] = value;
    /*Pull cs line low to enable slave*/
    cs_enable();
    /*Transmit data and address*/
    (void)spi1_transmit(data, 2U);
    /*Pull cs line high to disable slave*/
    cs_disable();
}

void adxl_init(void)
{
    (void)uart_send_string("write DATA_FORMAT\r\n");
    adxl_write(ADXL345_REG_DATA_FORMAT, ADXL345_RANGE_4G);
    (void)uart_send_string("write POWER_CTL reset\r\n");
    adxl_write(ADXL345_REG_POWER_CTL, ADXL345_RESET);
    (void)uart_send_string("write POWER_CTL measure\r\n");
    adxl_write(ADXL345_REG_POWER_CTL, ADXL345_MEASURE_BIT);
    (void)uart_send_string("adxl writes done\r\n");
}

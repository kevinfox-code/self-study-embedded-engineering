/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Implements the temperature sensor subscriber — listens on TOPIC_TICK_100MS and publishes a simulated temperature reading on TOPIC_TEMPERATURE at 10 Hz.
 */

#include "sensor.h"
#include "pubsub.h"

/*
 * Returns a temperature reading in tenths of a degree Celsius.
 * On real hardware this would read from an ADC or an I2C sensor.
 */
static int16_t ReadTemperatureDegC10(void)
{
    return 215;  /* 21.5 °C */
}

void SENSOR_Init(void) {}

void SENSOR_Update(void)
{
    PUBSUB_Data data;
    data.temperature_decidegc = ReadTemperatureDegC10();
    PUBSUB_Publish(TOPIC_TEMPERATURE, data);
}

/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Declares the simulated temperature sensor module — SENSOR_Update() reads (or simulates) a temperature value and publishes it on TOPIC_TEMPERATURE for any interested subscriber.
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Simulated temperature sensor subscriber/publisher.
 *
 * SENSOR_Update() reads the hardware (or simulation) and publishes
 * TOPIC_TEMPERATURE.  It is meant to be called from a TOPIC_TICK_100MS
 * subscriber so temperature is sampled at 10 Hz.
 */
void SENSOR_Init(void);
void SENSOR_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H__ */


#ifndef __SENSOR_APP_H__
#define __SENSOR_APP_H__

#include <kernel.h>
#include <drivers/sensor.h>
#include <stdint.h>

int avg_float_sensor_readings(float *avg, const struct device *dev, int channel, int num, k_timeout_t delay);
int avg_u16_sensor_readings(uint16_t *avg, const struct device *dev, int channel, int num, k_timeout_t delay);
int avg_int_sensor_readings(uint16_t *avg, const struct device *dev, int channel, int num, k_timeout_t delay);

#endif /* __SENSOR_APP_H__ */

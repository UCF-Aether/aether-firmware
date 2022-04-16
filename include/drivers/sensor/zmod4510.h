
#ifndef ZEPHYR_INCLUDE_DRIVERS_SENSOR_ZMOD4510_H_
#define ZEPHYR_INCLUDE_DRIVERS_SENSOR_ZMOD4510_H_

#include <device.h>
#include <drivers/sensor.h>

enum zmod4510_sensor_channel {
  /** FAST_AQI stands for a 1-minute average of the Air Quality Index according to the EPA standard
   * based on ozone */
  ZMOD4510_SENSOR_CHAN_FAST_AQI = SENSOR_CHAN_PRIV_START,
  /** EPA_AQI stands for the Air Quality Index according to the EPA standard based on ozone */
  ZMOD4510_SENSOR_CHAN_AQI,
  /** O3 levels in parts per billion (ppb) */
  ZMOD4510_SENSOR_CHAN_O3, 
  /** NO2 levels in parts per billion (ppb) */
  ZMOD4510_SENSOR_CHAN_NO2, 
};

int calc_oaq(const struct device *dev, float humidity_pct, float temperature_degc);

#endif /* ZEPHYR_INCLUDE_DRIVERS_SENSOR_ZMOD4510_H_ */

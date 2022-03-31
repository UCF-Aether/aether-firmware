#include <drivers/sensor/zmod4510.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(zmod4510_sandbox);

void main() {
  struct sensor_value aqi, humidity, temp;
  const struct device *zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
  const struct device *bme = DEVICE_DT_GET(DT_NODELABEL(bme680));

  if (!device_is_ready(zmod)) {
    printk("zmod not ready\n");
    return;
  }

  if (!device_is_ready(bme)) {
    printk("bme not ready\n");
    return;
  }

  while (1) {
    sensor_sample_fetch(zmod);
    sensor_channel_get(zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_AQI, &aqi); 

    sensor_sample_fetch(bme);
    sensor_channel_get(bme, SENSOR_CHAN_HUMIDITY, &humidity);
    sensor_channel_get(bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);

    // Warn just to differentiate the logs
    LOG_WRN("aqi=%f, humidity=%f, temp=%f\n", 
        sensor_value_to_double(&aqi),
        sensor_value_to_double(&humidity),
        sensor_value_to_double(&temp));
    k_msleep(1000);
  }
}

#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>
#include <cayenne.h>

LOG_MODULE_DECLARE(aether);


#define BME_SLEEP 15000

void bme_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value temp, press, humidity, gas_res;
  struct reading reading;
  const struct device *dev_bme = DEVICE_DT_GET(DT_NODELABEL(bme680));
  struct k_msgq *msgq = (struct k_msgq *) _msgq;

  /* Check if the BME688 is ready */
  if (!device_is_ready(dev_bme)) {
    LOG_ERR("BME688 is not ready!\n");
    return;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {
    //disable_sleep();

    LOG_INF("Fetching data");
    /* Read data from BME688 */
    sensor_sample_fetch(dev_bme);
    sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity);
    sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res);

    reading.chan = CAYENNE_CHANNEL_BME;

    reading.type = CAYENNE_TYPE_TEMP;
    reading.val.f = (float) sensor_value_to_double(&temp);
    LOG_INF("Temperature = %.03f C", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PRESSURE;
    reading.val.f = (float) sensor_value_to_double(&press);
    LOG_INF("Pressure = %.03f hPa", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_HUMIDITY;
    reading.val.f = (float) sensor_value_to_double(&humidity);
    LOG_INF("Humidity = %.03f%%", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_GAS_RES;
    reading.val.f = (float) sensor_value_to_double(&gas_res);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    //enable_sleep();
    k_msleep(BME_SLEEP);
  }
}

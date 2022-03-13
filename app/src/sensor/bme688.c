#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>
#include "bme688.h"
#include "cayenne.h"

LOG_MODULE_DECLARE(aether);


// TODO: make configurable - nvm
#define BME_SLEEP 3000

#ifdef BME_REAL_DATA

#define GET_SENSOR() DEVICE_DT_GET(DT_NODELABEL(bme680));

#else

// TODO: Create virtual (mock) sensor
// https://docs.zephyrproject.org/latest/reference/drivers/index.html?highlight=device#c.DEVICE_DEFINE

#endif

// TODO: passing in config context
void bme_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value temp, press, humidity, gas_res;
  struct reading reading;
  const struct device *dev_bme = GET_SENSOR();
  struct k_msgq *msgq = (struct k_msgq *) _msgq;

  /* Check if the BME688 is ready */
  if (!device_is_ready(dev_bme)) {
    LOG_ERR("BME688 is not ready!\n");
    return;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {

    /* Read data from BME688 */
    sensor_sample_fetch(dev_bme);
    sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity);
    sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res);

    reading.chan = CAYENNE_CHANNEL_BME;

    reading.type = CAYENNE_TYPE_TEMP;
    reading.val.f = (float) sensor_value_to_double(&temp);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PRESSURE;
    reading.val.f = (float) sensor_value_to_double(&press);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_HUMIDITY;
    reading.val.f = (float) sensor_value_to_double(&humidity);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_GAS_RES;
    reading.val.f = (float) sensor_value_to_double(&gas_res);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    k_msleep(BME_SLEEP);
  }
}
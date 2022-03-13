#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include "bme688.h"
#include "cayenne.h"


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
  struct device *dev_bme = GET_SENSOR();
  struct k_msgq *msgq = (k_msgq *) _msgq;

  /* Check if the BME688 is ready */
  if (!device_is_ready(dev_bme)) {
    printk("BME688 is not ready!\n");
    return NULL;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {

    /* Read data from BME688 */
    sensor_sample_fetch(dev_bme);
    sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity);
    sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res);

    printf("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
        temp.val1, temp.val2, press.val1, press.val2,
        humidity.val1, humidity.val2, gas_res.val1,
        gas_res.val2);


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

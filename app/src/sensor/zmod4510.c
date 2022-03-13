#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/sensor/zmod4510.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>
#include "bme688.h"
#include "cayenne.h"

LOG_MODULE_DECLARE(aether);


// TODO: make configurable - nvm
#define ZMOD_SLEEP 3000

#ifdef ZMOD_REAL_DATA

#define GET_SENSOR() DEVICE_DT_GET(DT_NODELABEL(zmod4510));

#else

// TODO: Create virtual (mock) sensor
// https://docs.zephyrproject.org/latest/reference/drivers/index.html?highlight=device#c.DEVICE_DEFINE

#endif

// TODO: passing in config context
void zmod_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value fast_aqi, o3_ppb;
  struct reading reading;
  const struct device *dev_zmod = GET_SENSOR();
  struct k_msgq *msgq = (struct k_msgq *) _msgq;

  /* Check if the BME688 is ready */
  if (!device_is_ready(dev_zmod)) {
    LOG_ERR("ZMOD4510 is not ready!\n");
    return;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {

    /* Read data from BME688 */
    sensor_sample_fetch(dev_zmod);
    sensor_channel_get(dev_zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_FAST_AQI, &fast_aqi);
    sensor_channel_get(dev_zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_O3, &o3_ppb);


    reading.chan = CAYENNE_CHANNEL_ZMOD;

    reading.type = CAYENNE_TYPE_O3;
    reading.val.f = (float) sensor_value_to_double(&o3_ppb);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_FAST_AQI;
    reading.val.u16 = fast_aqi.val1;
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    k_msleep(ZMOD_SLEEP);
  }
}
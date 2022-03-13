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
#define SPS_SLEEP 3000

#ifdef SPS_REAL_DATA

#define GET_SENSOR() DEVICE_DT_GET(DT_NODELABEL(sps30));

#else

// TODO: Create virtual (mock) sensor
// https://docs.zephyrproject.org/latest/reference/drivers/index.html?highlight=device#c.DEVICE_DEFINE

#endif

// TODO: passing in config context
void sps_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value pm1p0, pm2p5, pm10p0;
  struct reading reading;
  const struct device *dev_sps = GET_SENSOR();
  struct k_msgq *msgq = (struct k_msgq *) _msgq;

  if (!device_is_ready(dev_sps)) {
    LOG_ERR("SPS30 is not ready!\n");
    return;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {

    sensor_sample_fetch(dev_sps);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_1_0, &pm1p0);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_2_5, &pm2p5);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_10, &pm10p0);

    reading.chan = CAYENNE_CHANNEL_SPS;

    reading.type = CAYENNE_TYPE_PM_1_0;
    reading.val.f = (float) sensor_value_to_double(&pm1p0);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PM_2_5;
    reading.val.f = (float) sensor_value_to_double(&pm2p5);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PM_10;
    reading.val.f = (float) sensor_value_to_double(&pm10p0);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    k_msleep(SPS_SLEEP);
  }
}

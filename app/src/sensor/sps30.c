#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>
#include <cayenne.h>
#include <pm/device.h>
#include <pm/device_runtime.h>
#include <drivers/gpio.h>

LOG_MODULE_REGISTER(sps30_loop, CONFIG_SENSOR_LOG_LEVEL);

#define SPS_SLEEP 15000

#define PWR_5V_DOMAIN DT_NODELABEL(pwr_5v_domain)


void sps_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value pm1p0, pm2p5, pm10p0;
  struct reading reading;
  const struct device *dev_sps = DEVICE_DT_GET(DT_NODELABEL(sps30));
  struct k_msgq *msgq = (struct k_msgq *) _msgq;
  static const struct device *pwr_5v_domain = DEVICE_DT_GET(DT_NODELABEL(pwr_5v_domain));

  pm_device_runtime_enable(pwr_5v_domain);
  pm_device_runtime_enable(pwr_5v_domain);

  //printk("dev->state->initialzed=%d  dev->state->init_res=%d\n", dev_sps->state->initialized, dev_sps->state->init_res);

  // if (!device_is_ready(dev_sps)) {
  //   LOG_ERR("SPS30 is not ready!\n" );
  //   return;
  // }

  while (1) {

    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_RESUME);
    pm_device_action_run(dev_sps, PM_DEVICE_ACTION_RESUME);

    /* Allow time for device to turn on */
    k_msleep(2000);

    sensor_sample_fetch(dev_sps);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_1_0, &pm1p0);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_2_5, &pm2p5);
    sensor_channel_get(dev_sps, SENSOR_CHAN_PM_10, &pm10p0);

    reading.chan = CAYENNE_CHANNEL_SPS;

    reading.type = CAYENNE_TYPE_PM_1_0;
    reading.val.f = (float) sensor_value_to_double(&pm1p0);
    LOG_DBG("PM 1p0=%f", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PM_2_5;
    reading.val.f = (float) sensor_value_to_double(&pm2p5);
    LOG_DBG("PM 2p5=%f", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_PM_10;
    reading.val.f = (float) sensor_value_to_double(&pm10p0);
    LOG_DBG("PM 10p0=%f", reading.val.f);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    /* Allow time for sensor measurement */
    k_msleep(2000);

    pm_device_action_run(dev_sps, PM_DEVICE_ACTION_SUSPEND);
    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_SUSPEND);

    k_msleep(SPS_SLEEP);
  }
}

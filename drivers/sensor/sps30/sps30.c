#define DT_DRV_COMPAT sensirion_sps30

#include <stdint.h>
#include <device.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <init.h>
#include <logging/log.h>
#include <kernel.h>

#include "sps30.h"
#include "vendor/sps30.h"

LOG_MODULE_REGISTER(sps30, CONFIG_SENSOR_LOG_LEVEL);


static int sps30_init(const struct device *dev) {
  const int num_retries = 5;
  int retries_left;

  for (retries_left = num_retries - 1; retries_left >= 0; retries_left--) {
    if (sps30_probe() == 0) {
      // TODO: power management?
      sps30_start_measurement();
      return 0;
    }

    LOG_INF("Sensor probing failed. Retries left: %d", retries_left);
    k_msleep(500);
  }

  return -EINVAL;
}

static int sps30_sample_fetch(const struct device *dev, enum sensor_channel chan) {
  struct sps30_measurement m;
  struct sps30_data *data = (struct sps30_data *) dev->data;

  int ret = sps30_read_measurement(&m);
  if (ret < 0) {
    LOG_ERR("Error reading measurement: %d", ret);
    return -EINVAL;
  }

  data->mc_1p0 = m.mc_1p0;
  data->mc_2p5 = m.mc_2p5;
  data->mc_4p0 = m.mc_4p0;
  data->mc_10p0 = m.mc_10p0;
  data->nc_1p0 = m.mc_1p0;
  data->nc_2p5 = m.mc_2p5;
  data->nc_4p0 = m.mc_4p0;
  data->nc_10p0 = m.mc_10p0;
  data->typical_particle_size = m.typical_particle_size;

  return 0;
}

static int sps30_channel_get(const struct device *dev, enum sensor_channel chan,
				struct sensor_value *val) {

  struct sps30_data *data = (struct sps30_data *) dev->data;
  int ret = 0;

  switch (chan) {
  case SENSOR_CHAN_PM_1_0:
    sensor_value_from_double(val, (double) data->mc_1p0); 
    break;
  case SENSOR_CHAN_PM_2_5:
    sensor_value_from_double(val, (double) data->mc_2p5); 
    break;
  case SENSOR_CHAN_PM_10:
    sensor_value_from_double(val, (double) data->mc_10p0); 
    break;
  default:
    ret = -EINVAL;
  }

  return ret;
}

static const struct sensor_driver_api sps30_api_funcs = {
  .sample_fetch = sps30_sample_fetch,
  .channel_get = sps30_channel_get,
};

static struct sps30_data sps30_data;

static const struct sps30_config sps30_config = {
  .bus = I2C_DT_SPEC_INST_GET(0),
};

DEVICE_DT_INST_DEFINE(0, sps30_init, NULL, &sps30_data, &sps30_config, POST_KERNEL,
    CONFIG_SENSOR_INIT_PRIORITY, &sps30_api_funcs);

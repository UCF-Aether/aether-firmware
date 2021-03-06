
#include <stdint.h>
#define DT_DRV_COMPAT sensirion_sps30

#include <device.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <init.h>
#include <logging/log.h>
#include <kernel.h>
#include <pm/pm.h>
#include <pm/device.h>
#include <pm/device_runtime.h>
#include "sps30.h"
#include "vendor/sps30.h"
#include <drivers/gpio.h>

LOG_MODULE_REGISTER(sps30, CONFIG_SENSOR_LOG_LEVEL);

static struct gpio_dt_spec sps_gpio_power = GPIO_DT_SPEC_GET(DT_NODELABEL(sps_power_pins), gpios);

int sps30_init(const struct device *dev) {
  const int num_retries = 10;
  int retries_left;
  int ret;

  struct sps30_config *config = (struct sps30_config *) dev->config;

  sensirion_i2c_set_bus((struct device *) config->bus.bus);

  if (!device_is_ready(config->bus.bus)) {
    LOG_ERR("i2c bus isn't ready!");
    return -EINVAL;
  }

    /* Configure SPS30 GPIO pin */
  ret = gpio_pin_configure_dt(&sps_gpio_power, GPIO_OUTPUT);
  if (ret) {
    printk("Error %d: failed to configure pin %d\n", ret, sps_gpio_power.pin);
    return -EINVAL;
  }

  /* Enable SPS30 GPIO pin */
  ret = gpio_pin_set_dt(&sps_gpio_power, 1);
  if (ret) {
    printk("Error %d: failed to set pin %d\n", ret, sps_gpio_power.pin);
    return -EINVAL;
  }
    
  for (retries_left = num_retries - 1; retries_left >= 0; retries_left--) {
    if (sps30_probe() == 0) {
      // TODO: power management?
      LOG_DBG("SPS probed");
      sps30_start_measurement();
      /* Enable SPS30 GPIO pin */

      return 0;
    }

    LOG_INF("Sensor probing failed. Retries left: %d", retries_left);
    k_msleep(100);
  }

  return -EINVAL;
}

static int sps30_sample_fetch(const struct device *dev, enum sensor_channel chan) {
  struct sps30_measurement m;
  struct sps30_data *data = (struct sps30_data *) dev->data;
  int ret;

    /* Configure SPS30 GPIO pin */
  // ret = gpio_pin_configure_dt(&sps_gpio_power, GPIO_OUTPUT);
  // if (ret) {
  //   printk("Error %d: failed to configure pin %d\n", ret, sps_gpio_power.pin);
  //   return -EINVAL;
  // }

  /* Enable SPS30 GPIO pin */
  // ret = gpio_pin_set_dt(&sps_gpio_power, 1);
  // if (ret) {
  //   printk("Error %d: failed to set pin %d\n", ret, sps_gpio_power.pin);
  //   return -EINVAL;
  // }

  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    LOG_ERR("Error reading measurement: %d", ret);
          /* Enable SPS30 GPIO pin */

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

static int sps30_pm_action(const struct device *dev,
	enum pm_device_action action)
{
	int rc = 0;

	switch (action) {
	case PM_DEVICE_ACTION_RESUME:
		/* Switch power on */
    sps30_init(dev);
    LOG_WRN("sps30 resume");
		break;
	case PM_DEVICE_ACTION_SUSPEND:
    LOG_WRN("sps30 suspend");


		break;
	case PM_DEVICE_ACTION_TURN_ON:

    // k_msleep(1000);
    //
    sps30_init(dev);
    LOG_WRN("sps30 turn on");
		break;
	case PM_DEVICE_ACTION_TURN_OFF:
    LOG_WRN("sps30 turn off");
		break;
	default:
		rc = -ENOTSUP;
	}

	return rc;
}


static const struct sensor_driver_api sps30_api_funcs = {
  .sample_fetch = sps30_sample_fetch,
  .channel_get = sps30_channel_get,
};

struct sps30_data sps30_data;

static const struct sps30_config sps30_config = {
  .bus = I2C_DT_SPEC_INST_GET(0),
};

// PM_DEVICE_DT_INST_DEFINE(0, sps30_pm_action);
DEVICE_DT_INST_DEFINE(0, sps30_init, NULL, &sps30_data, &sps30_config, POST_KERNEL,
    CONFIG_SENSOR_INIT_PRIORITY, &sps30_api_funcs);

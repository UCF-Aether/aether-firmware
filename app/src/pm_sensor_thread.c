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
#include "pm.h"
#include "lora.h"
#include <drivers/sensor/sps30.h>

LOG_MODULE_REGISTER(pm_sensor_thread, CONFIG_LOG_DEFAULT_LEVEL);

// #define SPS_SLEEP 900000
#define SPS_SLEEP 30000
#define NUM_READINGS 20
#define SPS_READING_DELAY_MS 5000
#define NUM_SKIP_READINGS 5

// #define PWR_5V_DOMAIN DT_NODELABEL(pwr_5v_domain)

// static const struct device *pwr_5v_domain = DEVICE_DT_GET(DT_NODELABEL(pwr_5v_domain));
const struct device *dev_sps = DEVICE_DT_GET(DT_NODELABEL(sps30));

void pm_sensor_thread(void *arg1, void *arg2, void *arg3) {
  struct sensor_value pm1p0, pm2p5, pm10p0;
  struct reading reading;
  float pm2_5_sum = 0, pm10_sum = 0;
  enum pm_device_state state;
  int ret, fail_count = 0;

  //pm_device_runtime_enable(dev_sps);
  //pm_device_runtime_enable(pwr_5v_domain);

  //pm_device_state_get(pwr_5v_domain, &state);
  LOG_WRN("domain: %d", state);
  // if (!device_is_ready(dev_sps)) {
  //   LOG_ERR("SPS30 is not ready!\n" );
  //   return;
  // }

  static struct gpio_dt_spec sps_gpio_power = GPIO_DT_SPEC_GET(DT_NODELABEL(sps_power_pins), gpios);

  /* Configure SPS30 GPIO pin */
  ret = gpio_pin_configure_dt(&sps_gpio_power, GPIO_OUTPUT);
  if (ret) {
    printk("Error %d: failed to configure pin %d\n", ret, sps_gpio_power.pin);
    return -EINVAL;
  }

  while (1) {
    disable_sleep();

  /* Enable SPS30 GPIO pin */
    ret = gpio_pin_set_dt(&sps_gpio_power, 1);
    if (ret) {
      printk("Error %d: failed to set pin %d\n", ret, sps_gpio_power.pin);
    }
    else {
      sps30_init(dev_sps);

      /* Allow time for device to warm up */
      k_msleep(30000);

      pm2_5_sum = 0, pm10_sum = 0;
      fail_count = 0;

      for (int i = 0; i < NUM_READINGS + NUM_SKIP_READINGS; i++) {
        ret = sensor_sample_fetch(dev_sps);
        if (i < NUM_SKIP_READINGS) {
          k_msleep(SPS_READING_DELAY_MS);
          continue;
        }

        if (ret != 0) {
          continue;
          fail_count++;
        }
        sensor_channel_get(dev_sps, SENSOR_CHAN_PM_2_5, &pm2p5);
        sensor_channel_get(dev_sps, SENSOR_CHAN_PM_10, &pm10p0);
        printk("pm2.5 %f\n", sensor_value_to_double(&pm2p5));
        printk("pm10 %f\n", sensor_value_to_double(&pm10p0));

        pm2_5_sum += (float) sensor_value_to_double(&pm2p5);
        pm10_sum += (float) sensor_value_to_double(&pm10p0);

        k_msleep(SPS_READING_DELAY_MS);
      }

      if (fail_count < NUM_READINGS) {
        reading.chan = CAYENNE_CHANNEL_SPS;

        reading.type = CAYENNE_TYPE_PM_2_5;
        reading.val.f = pm2_5_sum / (NUM_READINGS - fail_count);
        LOG_DBG("PM 2p5=%f", reading.val.f);
        LOG_INF("PM 2.5 = %.03f um", reading.val.f);
        lorawan_schedule(&reading);

        reading.type = CAYENNE_TYPE_PM_10;
        reading.val.f = pm10_sum / (NUM_READINGS - fail_count);
        LOG_DBG("PM 10p0=%f", reading.val.f);
        LOG_INF("PM 10 = %.03f um", reading.val.f);
        lorawan_schedule(&reading);
      }

    }

    gpio_pin_set_dt(&sps_gpio_power, 0);
    enable_sleep();
    k_msleep(SPS_SLEEP);
  }
}

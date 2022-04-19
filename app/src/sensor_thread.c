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
#include <drivers/sensor/zmod4510.h>
#include "lora.h"
#include "pm.h"

void zmod_sample(struct k_work *work);
void zmod_submit_sample(struct k_timer *timer);

LOG_MODULE_REGISTER(sensor_thread, CONFIG_LOG_DEFAULT_LEVEL);

K_TIMER_DEFINE(zmod_sampling_timer, zmod_submit_sample, NULL);
K_WORK_DEFINE(zmod_sample_work, zmod_sample);

#define ZMOD_SAMPLING_DURATION_MS 30000
#define ZMOD_SAMPLING_DELAY_MS 1980
#define IGNORE_ZMOD_STABILIZATION true
// #define SLEEP_MS 900000
#define SLEEP_MS (60000 * 15)

const struct device *dev_zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
const struct device *dev_bme = DEVICE_DT_GET(DT_NODELABEL(bme680));

void sensor_thread(void *arg1, void *arg2, void *arg3) {
  int ret;
  struct sensor_value temp_degc_sv, press_pa_sv, humidity_pct_sv, 
                      gas_res_ohm_sv, o3_ppb_sv, no2_ppb_sv, aqi_sv, fast_aqi_sv;
  float temp_degc = 20, humidity_pct = 50;
  struct reading rd;

  if (!device_is_ready(dev_zmod)) {
    LOG_ERR("ZMOD4510 is not ready!");
    return;
  }

  if (!device_is_ready(dev_bme)) {
    LOG_ERR("BME688 is not ready!");
    return;
  }

  while (true) {
    disable_sleep();

    rd.chan = CAYENNE_CHANNEL_BME;
    ret = sensor_sample_fetch(dev_bme);
    if (ret) {
      LOG_ERR("Unable to fetch BME688 readings");
    }
    else {

      ret = sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp_degc_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_TEMP;
        rd.val.f = sensor_value_to_double(&temp_degc_sv);
        temp_degc = rd.val.f;
        LOG_INF("TEMPERATURE (degC) = %f", rd.val.f);
        lorawan_schedule(&rd);
      }

      ret = sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press_pa_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_PRESSURE;
        rd.val.f = sensor_value_to_double(&press_pa_sv);
        LOG_INF("PRESSURE (kPa) = %f", rd.val.f);
        lorawan_schedule(&rd);
      }

      ret = sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity_pct_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_HUMIDITY;
        rd.val.f = sensor_value_to_double(&humidity_pct_sv);
        humidity_pct = rd.val.f;
        LOG_INF("HUMIDITY (%) = %f", rd.val.f);
        lorawan_schedule(&rd);
      }

      ret = sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res_ohm_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_GAS_RES;
        rd.val.f = sensor_value_to_double(&gas_res_ohm_sv);
        LOG_INF("GAS RES (Ohm) = %f", rd.val.f);
        lorawan_schedule(&rd);
      }
    }

    rd.chan = CAYENNE_CHANNEL_ZMOD;
    LOG_INF("Starting zmod4510 sampling timer");
    k_timer_start(&zmod_sampling_timer, K_NO_WAIT, K_MSEC(ZMOD_SAMPLING_DELAY_MS));
    k_msleep(ZMOD_SAMPLING_DURATION_MS);
    k_timer_stop(&zmod_sampling_timer);
    LOG_INF("Finished zmod4510 sampling");

    // Use previous values from bme if bme fails for whaever reason.
    ret = calc_oaq(dev_zmod, humidity_pct, temp_degc);
    if (ret == -EINPROGRESS && !IGNORE_ZMOD_STABILIZATION) {
      LOG_WRN("zmod4510 still stabilizing");
    }
    else if (ret != 0 && ret != -EINPROGRESS) {
      LOG_ERR("zmod4510 oaq algo error! %d", ret);
    }
    else {
      if (ret == -EINPROGRESS)
        LOG_WRN("zmod4510 still stabilizing. Skipping...");
      ret = sensor_channel_get(dev_zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_O3, &o3_ppb_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_O3;
        rd.val.f = sensor_value_to_double(&o3_ppb_sv) / 1000;
        LOG_INF("O3 (ppm) = %f", rd.val.f);
        lorawan_schedule(&rd);
      }
      // sensor_channel_get(dev_zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_NO2, &no2_ppb_sv);

      ret = sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_AQI, &aqi_sv);
      if (ret == 0) {
        rd.type = CAYENNE_TYPE_AQI;
        rd.val.u16 = sensor_value_to_double(&aqi_sv);
        LOG_INF("ZMOD AQI (Index 0-500) = %d", rd.val.u16);
        lorawan_schedule(&rd);
      }
    }

    enable_sleep();
    k_msleep(SLEEP_MS);
  }
}

void zmod_sample(struct k_work *work) {
  sensor_sample_fetch(dev_zmod);
}

void zmod_submit_sample(struct k_timer *timer) {
  k_work_submit(&zmod_sample_work);
}

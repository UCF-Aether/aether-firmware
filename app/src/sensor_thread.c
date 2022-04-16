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

LOG_MODULE_REGISTER(sensor_thread, CONFIG_LOG_DEFAULT_LEVEL);

#define IGNORE_ZMOD_STABILIZATION true
#define SLEEP_MS 15000

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
    ret = sensor_sample_fetch(dev_zmod);
    if (ret) {
      LOG_ERR("Unable to fetch ZMOD4510 readings");
    }
    else {
      // Use previous values from bme if bme fails for whaever reason.
      ret = calc_oaq(dev_zmod, humidity_pct, temp_degc);
      if (ret == -EINPROGRESS && !IGNORE_ZMOD_STABILIZATION) {
        LOG_WRN("zmod4510 still stabilizing");
      }
      else if (ret != 0 && ret != -EINPROGRESS) {
        LOG_ERR("zmod4510 sensor fetch error! %d", ret);
      }
      else {
        ret = sensor_channel_get(dev_zmod, (enum sensor_channel) ZMOD4510_SENSOR_CHAN_O3, &o3_ppb_sv);
        if (ret == 0) {
          rd.type = CAYENNE_TYPE_O3;
          rd.val.f = sensor_value_to_double(&o3_ppb_sv);
          LOG_INF("O3 (ppb) = %f", rd.val.f);
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
    }

    enable_sleep();
    k_msleep(SLEEP_MS);
  }
}

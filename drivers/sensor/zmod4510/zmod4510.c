#define DT_DRV_COMPAT renesas_zmod4510

#include "zmod4510.h"
#include "oaq_2nd_gen.h"
#include "zmod4510_config_oaq2.h"
#include "zmod4xxx.h"
#include "zmod4xxx_types.h"
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <init.h>
#include <kernel.h>
#include <stdio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(zmod4510, CONFIG_SENSOR_LOG_LEVEL);

int8_t zmod4510_reg_read(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf,
                          uint8_t len);
int8_t zmod4510_reg_write(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf,
                           uint8_t len);

void zmod4510_delay(uint32_t ms) {
  k_sleep(K_MSEC(ms));
}

static int zmod4510_chip_init(const struct device *dev) {
  struct zmod4510_data *data = (struct zmod4510_data *) dev->data;
  zmod4xxx_dev_t zmod_dev = data->zmod_dev;
  int ret;

  zmod_dev.i2c_addr = ZMOD4510_I2C_ADDR;
  zmod_dev.pid = ZMOD4510_PID;
  zmod_dev.init_conf = &zmod_oaq_sensor_type[INIT];
  zmod_dev.meas_conf = &zmod_oaq_sensor_type[MEASURE];
  zmod_dev.prod_data = data->prod_data;
  zmod_dev.read = zmod4510_reg_read;
  zmod_dev.write = zmod4510_reg_write;
  zmod_dev.delay_ms = zmod4510_delay;

  LOG_DBG("Reading sensor data");
  ret = zmod4xxx_read_sensor_info(&zmod_dev);
  if (ret) {
    LOG_ERR("Error reading sensor data: 0x%x", ret);
    return ret;
  }

  // ret = zmod4xxx_read_tracking_number(&zmod_dev, &data->tracking_number);
  // if (ret) {
  //   LOG_ERR("Error reading tracking number: 0x%x", ret);
  //   return ret;
  // }

  LOG_DBG("Preparing sensor");
  ret = zmod4xxx_prepare_sensor(&zmod_dev);
  if (ret) {
    LOG_ERR("Error preparing sensor: 0x%x", ret);
    return ret;
  }

  LOG_DBG("Initializing OAQ v2");
  ret = init_oaq_2nd_gen(&data->algo_handle, &zmod_dev);
  if (ret) {
    LOG_ERR("Error initializing the OAQ v2 algorithm: 0x%x", ret);
    return ret;
  }

  return 0;
}

static int zmod4510_init(const struct device *dev) {
  const struct zmod4510_config *config = dev->config;

  if (!device_is_ready(config->bus.bus)) {
    LOG_ERR("I2C master %s not ready", config->bus.bus->name);
    return -EINVAL;
  }

  if (zmod4510_chip_init(dev) < 0) {
    return -EINVAL;
  }

  return 0;
}

static int zmod4510_channel_get(const struct device *dev,
                                enum sensor_channel chan,
                                struct sensor_value *val) {
  // struct zmod4510_data *data = dev->data;
  //
  // switch (chan) {
  // // case SENSOR_CHAN_RMOX:
  // // 	/*
  // // 	 * data->calc_temp has a resolution of 0.01 degC.
  // // 	 * So 5123 equals 51.23 degC.
  // // 	 */
  // // 	val->val1 = (int32_t) data->calc_temp;
  // // 	val->val2 = 0;
  // // 	break;
  // case SENSOR_CHAN_VOC:
  //   /*
  //    * data->calc_press has a resolution of 1 Pa.
  //    * So 96321 equals 96.321 kPa.
  //    */
  //   val->val1 = (int32_t)data->(algo_results.O3_conc_ppb);
  //   val->val2 = 0;
  //   break;
  // // case SENSOR_CHAN_FAST_AQI:
  // // 	/*
  // // 	 * data->calc_humidity has a resolution of 0.001 %RH.
  // // 	 * So 46333 equals 46.333 %RH.
  // // 	 */
  // // 	val->val1 = (int32_t) data->(algo_results.FAST_AQI);
  // // 	val->val2 = 0;
  // // 	break;
  // // case SENSOR_CHAN_AQI:
  // // 	/*
  // // 	 * data->calc_gas_resistance has a resolution of 1 ohm.
  // // 	 * So 100000 equals 100000 ohms.
  // // 	 */
  // // 	val->val1 = (int32_t) data->(algo_results.EPA_AQI);
  // // 	val->val2 = 0;
  // // 	break;
  // default:
  //   return -EINVAL;
  // }

  return 0;
}

static int zmod4510_sample_fetch(const struct device *dev,
                                 enum sensor_channel chan) {
  // struct zmod4510_data *data = dev->data;
  // int ret;
  //
  // __ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL);
  //
  // ret = zmod4xxx_read_adc_result(data->zmod_dev, data->adc_result);
  //
  // /* Humidity and temperature measurements are needed for ambient compensation.
  //  *  It is highly recommented to have a real humidity and temperature sensor
  //  *  for these values! */
  // data->humidity_pct = 50.0;     // 50% RH
  // data->temperature_degc = 20.0; // 20 degC
  //
  // // get sensor results with API
  // ret = calc_oaq_2nd_gen(data->algo_handle, data->zmod_dev, data.adc_result,
  //                        data.humidity_pct, data.temperature_degc,
  //                        data->algo_results);
  //
  // if ((ret != OAQ_2ND_GEN_OK) && (ret != OAQ_2ND_GEN_STABILIZATION)) {
  //   printf("Error %d when calculating algorithm!\n", ret);
  //   return ret;
  //   /* OAQ 2nd Gen algorithm skips first samples for sensor stabilization */
  // }

  return 0;
}

static const struct sensor_driver_api zmod4510_api_funcs = {
    .sample_fetch = zmod4510_sample_fetch,
    .channel_get = zmod4510_channel_get,
};

static struct zmod4510_data zmod4510_data;

static const struct zmod4510_config zmod4510_config = {
    .bus = I2C_DT_SPEC_INST_GET(0),
};

int8_t zmod4510_reg_read(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf,
                          uint8_t len) {
  return i2c_burst_read_dt(&zmod4510_config.bus, reg_addr, data_buf, len);
}

int8_t zmod4510_reg_write(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf,
                           uint8_t len) {
  return i2c_burst_write_dt(&zmod4510_config.bus, reg_addr, data_buf, len);
}

DEVICE_DT_INST_DEFINE(0, zmod4510_init, NULL, &zmod4510_data, &zmod4510_config,
                      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
                      &zmod4510_api_funcs);

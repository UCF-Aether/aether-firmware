#ifndef __SENSOR_ZMOD4510_H__
#define __SENSOR_ZMOD4510_H__
#include <device.h>
#include <drivers/i2c.h>

#include "zmod4510_config_oaq2.h"
#include "zmod4xxx_types.h"
#include "zmod4xxx.h"
#include "oaq_2nd_gen.h"

struct zmod4510_data {
  zmod4xxx_dev_t zmod_dev;
  oaq_2nd_gen_handle_t algo_handle;
  oaq_2nd_gen_results_t algo_results;
  uint8_t adc_result[ZMOD4510_ADC_DATA_LEN];
  float humidity_pct;
  float temperature_degc;
  uint8_t prod_data[ZMOD4510_PROD_DATA_LEN];
  uint8_t tracking_number[6];
};

struct zmod4510_config {
  struct i2c_dt_spec bus;
};

#endif /* __SENSOR_ZMOD4510_H__ */

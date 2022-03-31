#include <stdint.h>
#define DT_DRV_COMPAT renesas_zmod4510

#include "zmod4510.h"
#include "vendor/oaq_2nd_gen.h"
#include "vendor/zmod4510_config_oaq2.h"
#include "vendor/zmod4xxx.h"
#include "vendor/zmod4xxx_types.h"
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <device.h>
#include <init.h>
#include <kernel.h>
#include <stdio.h>
#include <drivers/sensor/zmod4510.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(zmod4510, CONFIG_SENSOR_LOG_LEVEL);

int8_t zmod4510_reg_read(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf, uint8_t len);
int8_t zmod4510_reg_write(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf, uint8_t len);

#ifdef CONFIG_ZMOD4510_HUMIDITY
const static struct device *humidity_sensor = DEVICE_DT_GET(DT_INST_PROP(0, humidity_sensor));
#endif /* CONFIG_ZMOD4510_HUMIDITY */

#ifdef CONFIG_ZMOD4510_TEMPERATURE
const static struct device *temp_sensor = DEVICE_DT_GET(DT_INST_PROP(0, temperature_sensor));
#endif /* CONFIG_ZMOD4510_TEMPERATURE */

void zmod4510_delay(uint32_t ms)
{
	k_sleep(K_MSEC(ms));
}

static int zmod4510_chip_init(const struct device *dev)
{
	struct zmod4510_data *data = (struct zmod4510_data *)dev->data;
	zmod4xxx_dev_t *zmod_dev = &data->zmod_dev;
	int ret;

	zmod_dev->i2c_addr = ZMOD4510_I2C_ADDR;
	zmod_dev->pid = ZMOD4510_PID;
	zmod_dev->init_conf = &zmod_oaq_sensor_type[INIT];
	zmod_dev->meas_conf = &zmod_oaq_sensor_type[MEASURE];
	zmod_dev->prod_data = data->prod_data;
	zmod_dev->read = zmod4510_reg_read;
	zmod_dev->write = zmod4510_reg_write;
	zmod_dev->delay_ms = zmod4510_delay;

	LOG_DBG("Reading sensor data");
	ret = zmod4xxx_read_sensor_info(zmod_dev);
	if (ret) {
		LOG_ERR("Error reading sensor data: %d", ret);
		return ret;
	}

	ret = zmod4xxx_read_tracking_number(zmod_dev, (uint8_t *) &data->tracking_number);
	if (ret) {
	  LOG_ERR("Error reading tracking number: %d", ret);
	  return ret;
	}
  LOG_HEXDUMP_DBG(data->prod_data, sizeof(data->prod_data), "Product data");

	LOG_DBG("Preparing sensor");
	ret = zmod4xxx_prepare_sensor(zmod_dev);
	if (ret) {
		LOG_ERR("Error preparing sensor: %d", ret);
		return ret;
	}

	LOG_DBG("Initializing OAQ v2");
	ret = init_oaq_2nd_gen(&data->algo_handle, zmod_dev);
	if (ret) {
		LOG_ERR("Error initializing the OAQ v2 algorithm: %d", ret);
		return ret;
	}

  LOG_HEXDUMP_DBG(data->algo_handle.gcda, sizeof(data->algo_handle.gcda), "gcda");
  LOG_DBG("initial o3_conc_ppb: %f", data->algo_handle.O3_conc_ppb);
  LOG_DBG("mox_er: %d", zmod_dev->mox_er);
  LOG_DBG("mox_lr: %d", zmod_dev->mox_lr);
  LOG_HEXDUMP_DBG(zmod_dev->meas_conf, sizeof(zmod4xxx_conf), "measuring config");

  // TODO: remove me after testing
  data->algo_handle.stabilization_sample = 10;

	return 0;
}

static int zmod4510_init(const struct device *dev)
{
	const struct zmod4510_config *config = dev->config;

	if (!device_is_ready(config->bus.bus)) {
		LOG_ERR("I2C master %s not ready", config->bus.bus->name);
		return -EINVAL;
	}

	if (zmod4510_chip_init(dev) < 0) {
		LOG_ERR("Failed to initialize");
		return -EINVAL;
	}

	return 0;
}

static int zmod4510_channel_get(const struct device *dev, enum sensor_channel chan,
				struct sensor_value *val)
{
	struct zmod4510_data *data = dev->data;

  // TODO: GAS RES channel
	switch ((enum zmod4510_sensor_channel) chan) {
	case ZMOD4510_SENSOR_CHAN_O3:
		val->val1 = (int32_t)data->algo_results.O3_conc_ppb;
		val->val2 = 0;
		break;
	case ZMOD4510_SENSOR_CHAN_FAST_AQI:
		val->val1 = (int32_t)data->algo_results.FAST_AQI;
		val->val2 = 0;
		break;
	case ZMOD4510_SENSOR_CHAN_AQI:
		val->val1 = (int32_t)data->algo_results.EPA_AQI;
		val->val2 = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

// Must wait 1980ms before taking next measurement
static int zmod4510_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const uint8_t POLL_DELAY = 15;
	struct zmod4510_data *data = (struct zmod4510_data *) dev->data;
	zmod4xxx_dev_t *zmod_dev = &data->zmod_dev;
	uint8_t zmod4510_status;

	int ret, polling_counter = 0;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL);

#ifdef CONFIG_ZMOD4510_HUMIDITY
  struct sensor_value humidity, temp;
  sensor_sample_fetch(humidity_sensor);
  sensor_channel_get(humidity_sensor, SENSOR_CHAN_HUMIDITY, &humidity);
  LOG_INF("humidity=%f%%", sensor_value_to_double(&humidity));
#endif /* CONFIG_ZMOD4510_HUMIDITY */

#ifdef CONFIG_ZMOD4510_TEMPERATURE
  sensor_sample_fetch(temp_sensor);
  sensor_channel_get(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  LOG_INF("temperature=%f C", sensor_value_to_double(&temp));
#endif /* CONFIG_ZMOD4510_TEMPERATURE */

  LOG_DBG("Starting sample fetch");
  ret = zmod4xxx_start_measurement(zmod_dev);
  if (ret) {
    LOG_ERR("Failed starting sensor measurement: %d", ret);
    return -EINVAL;
  }

  LOG_DBG("Waiting for reading");
	// TODO: interrupt based reading
	do {
    ret = zmod4xxx_read_status(zmod_dev, &zmod4510_status);
		if (ret) {
			LOG_ERR("Failed reading sensor status: %d", zmod4510_status);
			return -EINVAL;
		}

		polling_counter++;
		zmod_dev->delay_ms(POLL_DELAY);
	} while ((zmod4510_status & STATUS_SEQUENCER_RUNNING_MASK) &&
		 (polling_counter <= ZMOD4510_OAQ2_COUNTER_LIMIT));

  LOG_DBG("polling counter: %d", polling_counter);
  // Check if timeout occured
  if (ZMOD4510_OAQ2_COUNTER_LIMIT <= polling_counter) {
    ret = zmod4xxx_check_error_event(zmod_dev);
    if (ret) {
      LOG_ERR("Failed checking error event: %d", ret);
      return -EINVAL;
    }

    LOG_ERR("Gas read timeout %d", ERROR_GAS_TIMEOUT);
  }

  LOG_DBG("Reading adc results");
	ret = zmod4xxx_read_adc_result(zmod_dev, data->adc_result);
  if (ret) {
    LOG_ERR("Failed rading ADC results: %d", ret);
    return -EINVAL;
  }
  LOG_HEXDUMP_DBG(data->adc_result, ZMOD4510_ADC_DATA_LEN, "adc results");

#ifdef CONFIG_ZMOD4510_HUMIDITY
  data->humidity_pct = sensor_value_to_double(&humidity);
#else
	data->humidity_pct = 50.0; // 50% RH
#endif /* CONFIG_ZMOD4510_HUMIDITY */

#ifdef CONFIG_ZMOD4510_TEMPERATURE
  data->temperature_degc = sensor_value_to_double(&temp);
#else
	data->temperature_degc = 20.0; // 20 degC
#endif /* CONFIG_ZMOD4510_TEMPERATURE */

  float rmox;
  ret = zmod4xxx_calc_rmox(zmod_dev, data->adc_result, &rmox);
  if (ret) {
    LOG_ERR("Unable to calculate rmox: %d", ret);
  }
  LOG_DBG("rmox: %f", rmox);

  LOG_DBG("Calculating oaq");
	// get sensor results with API
	ret = calc_oaq_2nd_gen(&data->algo_handle, zmod_dev, data->adc_result,
			       data->humidity_pct, data->temperature_degc, &data->algo_results);

	if ((ret != OAQ_2ND_GEN_OK) && (ret != OAQ_2ND_GEN_STABILIZATION)) {
		LOG_ERR("Error %d when calculating algorithm!\n", ret);
		return ret;
	}

  if (ret == OAQ_2ND_GEN_STABILIZATION) {
    LOG_DBG("Warming up - %d readings until stabilization", data->algo_handle.stabilization_sample);
  }
  else {
    LOG_DBG("Stabilized");
  }

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

int8_t zmod4510_reg_read(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf, uint8_t len)
{
  LOG_DBG("addr: %d, reg_addr: %d, data_buf: 0x%p, len: %d", addr, reg_addr, data_buf, len);
	return i2c_burst_read_dt(&zmod4510_config.bus, reg_addr, data_buf, len);
}

int8_t zmod4510_reg_write(uint8_t addr, uint8_t reg_addr, uint8_t *data_buf, uint8_t len)
{
	return i2c_burst_write_dt(&zmod4510_config.bus, reg_addr, data_buf, len);
}

DEVICE_DT_INST_DEFINE(0, zmod4510_init, NULL, &zmod4510_data, &zmod4510_config, POST_KERNEL,
		      CONFIG_SENSOR_INIT_PRIORITY, &zmod4510_api_funcs);

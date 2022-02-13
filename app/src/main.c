/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>, Paul Wood <>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include <lorawan/lorawan.h>
//#include <include/pm/device.h>
#include <stdio.h>
#include <zephyr.h>

// #include "zmod4510_config_oaq2.h"
// #include "zmod4xxx.h"
// #include "zmod4xxx_cleaning.h"
// #include "zmod4xxx_hal.h"
// #include "oaq_2nd_gen.h"


/***************** LoRaWAN Configuration Paramters *******************/

/* Enable or disable LoRaWAN for testing purposes */
//#define ENABLE_LORAWAN
/* Switch between using ABP or OTAA parameters */
#define USE_ABP

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
	     "No default LoRa radio specified in DT");
#define DEFAULT_RADIO DT_LABEL(DEFAULT_RADIO_NODE)

#ifndef USE_ABP
/* OTAA Parameters */
#define LORAWAN_APP_EUI			{ 0x2C, 0xF7, 0xF1, 0x20, 0x32, 0x30,\
					  0x4D, 0xE8 }
#define LORAWAN_JOIN_EUI		{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,\
					  0x00, 0x06 }
#define LORAWAN_APP_KEY			{ 0xB4, 0x4E, 0x83, 0x6D, 0x99, 0x06,\
					  0x79, 0xC8, 0x78, 0x03, 0x1A, 0x32,\
					  0x60, 0x51, 0x11, 0xC8 }

#else
/* ABP Parameters */
#define LORAWAN_DEV_ADDR 0x260CFF4F;
#define LORAWAN_JOIN_EUI		{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,\
					  0x00, 0x06 }
#define LORAWAN_DEV_EUI			{ 0x2C, 0xF7, 0xF1, 0x20, 0x32, 0x30,\
					  0x4D, 0xE8}					  
#define LORAWAN_APP_EUI			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
					  0x00, 0x00}
#define LORAWAN_APP_SKEY		{ 0x88, 0x03, 0x0F, 0x9D, 0x09, 0x0D,\
					  0x09, 0xC0, 0x44, 0x63, 0x20, 0x4F, 0xC0, 0xE8,\
					  0x2C, 0xF5}
#define LORAWAN_NWK_SKEY		{ 0x06, 0xB1, 0x13, 0xEE, 0x65, 0x09,\
					  0xAA, 0xFB, 0x91, 0x75, 0xC2, 0x6E, 0xF2, 0x67,\
					  0x91, 0x92}
#endif

/*********************************************************************/

/********************** Other Defines ********************************/

#define DELAY K_MSEC(10000)

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

LOG_MODULE_REGISTER(lorawan_class_a);

/*********************************************************************/


static void dl_callback(uint8_t port, bool data_pending,
			int16_t rssi, int8_t snr,
			uint8_t len, const uint8_t *data)
{
	LOG_INF("Port %d, Pending %d, RSSI %ddB, SNR %ddBm", port, data_pending, rssi, snr);
	if (data) {
		LOG_HEXDUMP_INF(data, len, "Payload: ");
	}
}

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
	uint8_t unused, max_size;

	lorawan_get_payload_sizes(&unused, &max_size);
	LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

#ifndef USE_ABP

int init_lorawan_otaa()
{
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_APP_EUI;
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;
	int ret = 1;

	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	};

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
		return ret;
	}

	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return ret;
	}

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	join_cfg.mode = LORAWAN_ACT_OTAA;
	join_cfg.dev_eui = dev_eui;
	join_cfg.otaa.join_eui = join_eui;
	join_cfg.otaa.app_key = app_key;
	join_cfg.otaa.nwk_key = app_key;

	LOG_INF("Joining network over OTAA");
	ret = lorawan_join(&join_cfg);
	if (ret < 0) {
		LOG_ERR("lorawan_join_network failed: %d", ret);
		return ret;
	}

	return ret;
}

#else

int init_lorawan_abp()
{
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;

	uint32_t dev_addr = LORAWAN_DEV_ADDR;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI;
	uint8_t app_eui[] = LORAWAN_APP_EUI;
	uint8_t app_skey[] = LORAWAN_APP_SKEY;
	uint8_t nwk_skey[] = LORAWAN_NWK_SKEY;
	
	int ret = 1;

	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	};

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_ERR("%s Device not found", DEFAULT_RADIO);
		return ret;
	}

	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return ret;
	}

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	join_cfg.mode = LORAWAN_ACT_ABP;
	join_cfg.dev_eui = dev_eui;
	join_cfg.abp.dev_addr = dev_addr;
	join_cfg.abp.app_eui = app_eui;
	join_cfg.abp.app_skey = app_skey;
	join_cfg.abp.nwk_skey = nwk_skey;

	LOG_INF("Joining network over ABP");
	ret = lorawan_join(&join_cfg);
	if (ret < 0) {
		LOG_ERR("lorawan_join_network failed: %d", ret);
		return ret;
	}

	return ret;
}

#endif


void main(void)
{
	int ret;
	/* Initialize LoRaWAN if we are using it */
	// #ifdef ENABLE_LORAWAN
	// int ret;
	//
	// /* Use LoRaWAN OTAA or ABP */
	// #ifndef USE_ABP
	// ret = init_lorawan_otaa();
	// #else
	// ret = init_lorawan_abp();
	// #endif
	//
	// if (ret < 0)
	// 	return;
	// else
	// 	LOG_INF("Join Success!");
	//
	// #endif

	const struct device *dev_zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
	struct sensor_value fast_aqi, o3_ppb, aqi;

	// int8_t zret;
	// oaq_2nd_gen_handle_t algo_handle;
	// oaq_2nd_gen_results_t algo_results;

	// zmod4xxx_dev_t zmod_dev;
	// uint8_t adc_result[ZMOD4510_ADC_DATA_LEN];

	// float humidity_pct;
	// float temperature_degc;

	// zret = init_oaq_2nd_gen(&algo_handle, &zmod_dev);

	// Initialze the BME688
	// const struct device *dev_bme = DEVICE_DT_GET(DT_INST(0, bosch_bme680));
	// struct sensor_value temp, press, humidity, gas_res;

  while (!device_is_ready(dev_zmod)) {
    LOG_INF("Checking if the device is ready");
    k_sleep(K_MSEC(1000));
  }
	ret = device_is_ready(dev_zmod);
	if (!ret) {
		printk("ZMOD4510 is not ready! Error: %d\n", ret);
		return;
	}

	// Check if the BME688 is ready
	// if (!device_is_ready(dev_bme)) {
	// 	printk("BME688 is not ready!\n");
	// 	return;
	// }

	#ifdef ENABLE_LORAWAN
	LOG_INF("Sending data...");
	#endif
	
	while (1)
	{
		sensor_sample_fetch(dev_zmod);
		sensor_channel_get(dev_zmod, SENSOR_CHAN_VOC, &o3_ppb);
		// sensor_channel_get(dev_zmod, SENSOR_CHAN_FAST_AQI, &fast_aqi);
		// sensor_channel_get(dev_zmod, SENSOR_CHAN_AQI, &aqi);

		//pm_device_action_run(dev_bme, PM_DEVICE_ACTION_TURN_ON);

		//zret = zmod4xxx_read_adc_result(&zmod_dev, adc_result);


        /* Humidity and temperature measurements are needed for ambient compensation.
        *  It is highly recommented to have a real humidity and temperature sensor
        *  for these values! */
        // humidity_pct = 50.0; // 50% RH
        // temperature_degc = 20.0; // 20 degC

		// //get sensor results with API
		// zret = calc_oaq_2nd_gen(&algo_handle, &zmod_dev, adc_result, humidity_pct,
		// 						temperature_degc, &algo_results);

        // if ((zret != OAQ_2ND_GEN_OK) && (zret != OAQ_2ND_GEN_STABILIZATION)) {
        //     printf("Error %d when calculating algorithm, exiting program!\n",
        //            zret);
        //     return;
        //     /* OAQ 2nd Gen algorithm skips first samples for sensor stabilization */
        // } else {
		printf("*********** Measurements ***********\n");
		// for (int i = 0; i < 8; i++) {
		//     printf(" Rmox[%d] = ", i);
		//     printf("%.3f kOhm\n", algo_results.rmox[i] / 1e3);
		// }
		printf(" O3_conc_ppb = %d\n", o3_ppb.val1);
		// printf(" Fast AQI = %d\n", fast_aqi.val1);
		// printf(" EPA AQI = %d\n", aqi.val1);

		printf("************************************\n");
        

		// sensor_sample_fetch(dev_bme);
		// sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		// sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press);
		// sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity);
		// sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res);

		// printf("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
		// 		temp.val1, temp.val2, press.val1, press.val2,
		// 		humidity.val1, humidity.val2, gas_res.val1,
		// 		gas_res.val2);
		
		//pm_device_action_run(dev_bme, PM_DEVICE_ACTION_TURN_OFF);

		#ifdef ENABLE_LORAWAN

		char data[] = {'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd'};

		ret = lorawan_send(2, data, sizeof(data), LORAWAN_MSG_CONFIRMED);

		/*
		 * Note: The stack may return -EAGAIN if the provided data
		 * length exceeds the maximum possible one for the region and
		 * datarate. But since we are just sending the same data here,
		 * we'll just continue.
		 */
		if (ret == -EAGAIN) {
			LOG_ERR("lorawan_send failed: %d. Continuing...", ret);
			k_sleep(DELAY);
			continue;
		}

		if (ret < 0) {
			LOG_ERR("lorawan_send failed: %d", ret);
			return;
		}

		LOG_INF("Data sent!");
		#endif
		
		k_sleep(DELAY);
	}
}

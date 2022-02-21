/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>
 *					  Paul Wood <>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Zephyr Headers */
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/sensor/zmod4510.h>
#include <kernel.h>
#include <logging/log.h>
#include <lorawan/lorawan.h>
#include <stdio.h>
#include <zephyr.h>

/* Aether Headers */
#include "cayenne.h"
#include "lora.h"
#include "delays.h"
#include "threads.h"

/*************************** Sensor Configuration *****************************/

/* Flags for enabling sensors */
#define ENABLE_ZMOD
#define ENABLE_BME
#define ENABLE_PM

/* Flags to use real or fake sensor data */
//#define ZMOD_REAL_DATA
//#define BME_REAL_DATA
//#define PM_REAL_DATA
//#define LORA_REAL_DATA

/**************************** Other Defines ***********************************/

LOG_MODULE_REGISTER(lorawan_class_a);

K_FIFO_DEFINE(lora_send_fifo);

/******************************************************************************/

static void dl_callback(uint8_t port, bool data_pending,
			int16_t rssi, int8_t snr,
			uint8_t len, const uint8_t *data)
{
	LOG_INF("Port %d, Pending %d, RSSI %ddB, SNR %ddBm", 
			port, data_pending, rssi, snr);

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

/************************** Thread Entry Functions ****************************/

void bme_entry_point(void *arg1, void *arg2, void *arg3) {

	#ifdef BME_REAL_DATA

	/* Declare bme688 device and associated sensor values */
  	const struct device *dev_bme = DEVICE_DT_GET(DT_NODELABEL(bme680));
	struct sensor_value temp, press, humidity, gas_res;

	/* Check if the BME688 is ready */
	if (!device_is_ready(dev_bme)) {
		printk("BME688 is not ready!\n");
		return;
	}

	while (1) {

		/* Read data from BME688 */
		sensor_sample_fetch(dev_bme);
		sensor_channel_get(dev_bme, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(dev_bme, SENSOR_CHAN_PRESS, &press);
		sensor_channel_get(dev_bme, SENSOR_CHAN_HUMIDITY, &humidity);
		sensor_channel_get(dev_bme, SENSOR_CHAN_GAS_RES, &gas_res);

		printf("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
				temp.val1, temp.val2, press.val1, press.val2,
				humidity.val1, humidity.val2, gas_res.val1,
				gas_res.val2);

		k_yield();
	}

	#else

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		k_busy_wait((uint32_t) 500000);
		printf("T: 33.33; P: 44.44; H: 55.55; G: 4444.4444\n");

		intptr_t packet[10];

		packet[1] = 8;
		packet[2] = 20; /* temp.val1 */
		packet[3] = 30;
		packet[4] = 22;
		packet[5] = 34;
		packet[6] = 66;
		packet[7] = 99;
		packet[8] = 31;
		packet[9] = 11;

		k_fifo_put(&lora_send_fifo, packet);

		k_yield();
	}

	#endif
}

void zmod_entry_point(void *arg1, void *arg2, void *arg3) {

	#ifdef ZMOD_REAL_DATA
	int ret;
	const struct device *dev_zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
	struct sensor_value fast_aqi, o3_ppb;

  	/* Check if the ZMOD4510 is ready */
	if (!device_is_ready(dev_zmod)) {
		printk("ZMOD4510 is not ready!");
		return;
	}

	while (1) {
		/* Read data from ZMOD4510 */
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_FAST_AQI, &fast_aqi);
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_O3, &o3_ppb);

		printf("fast aqi: %d", fast_aqi.val1);
		printf("o3 (ppb): %d", o3_ppb.val1);
		k_yield();
	}

	#else

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		k_busy_wait((uint32_t) 500000);
		printf("fast aqi: %d ", 25);
		printf("o3 (ppb): %d\n", 100);
		k_yield();
	}

	#endif
}

void pm_entry_point(void *arg1, void *arg2, void *arg3) {

	#ifdef PM_REAL_DATA
	#else

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		k_busy_wait((uint32_t) 500000);
		printf("PM sensor sample\n");
		k_yield();
	}
	#endif
}

void lora_entry_point(void *arg1, void *arg2, void *arg3) {
	int ret;
	
	#ifdef LORA_REAL_DATA

	#ifdef USE_ABP
	ret = init_lorawan_abp();
	#else
	ret = init_lorawan_otaa();
	#endif

	if (ret < 0)
		return;
	else
		LOG_INF("Join Success!");

	while (1) {
		char data[] = {'h', 'e', 'l', 'l', 'o', 'w', 'o', 'r', 'l', 'd'};

		ret = lorawan_send(2, data, sizeof(data), LORAWAN_MSG_CONFIRMED);
		if (ret == -EAGAIN) {
			LOG_ERR("lorawan_send failed: %d. Continuing...", ret);
			k_sleep(DELAY);
			continue;
		}

		if (ret < 0) {
			LOG_ERR("lorawan_send failed: %d", ret);
			return;
		}

		k_yield();
	}

	#else
	int data_item;

	/* Simulate sending off LoRa packets */
	while (1) {
		intptr_t *data_item;

		data_item = k_fifo_get(&lora_send_fifo, FIFO_DELAY);

		/* If there is nothing in the FIFO, yield the thread */
		while ((data_item = k_fifo_get(&lora_send_fifo, K_NO_WAIT)) == NULL) {
			k_yield();
		}


		printf("Sending message: %d, %d\n", data_item[1], data_item[2]);
		k_busy_wait((uint32_t) 500000);
		printf("Message sent!\n");
		k_yield();
	}

	#endif
}

void usb_entry_point(void *arg1, void *arg2, void *arg3) {

	while (1) {
		printf("Waiting for USB commands...\n");
		k_yield();
	}
}

void main() 
{
	/* Initialize Threads */
	k_tid_t bme_tid = k_thread_create(&bme_thread_data, bme_stack_area,
									K_THREAD_STACK_SIZEOF(bme_stack_area),
									bme_entry_point,
									NULL, NULL, NULL,
									NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_tid_t zmod_tid = k_thread_create(&zmod_thread_data, zmod_stack_area,
									K_THREAD_STACK_SIZEOF(zmod_stack_area),
									zmod_entry_point,
									NULL, NULL, NULL,
									NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_tid_t pm_tid = k_thread_create(&pm_thread_data, pm_stack_area,
									K_THREAD_STACK_SIZEOF(pm_stack_area),
									pm_entry_point,
									NULL, NULL, NULL,
									NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_tid_t lora_tid = k_thread_create(&lora_thread_data, lora_stack_area,
									K_THREAD_STACK_SIZEOF(lora_stack_area),
									lora_entry_point,
									NULL, NULL, NULL,
									NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_tid_t usb_tid = k_thread_create(&usb_thread_data, usb_stack_area,
									K_THREAD_STACK_SIZEOF(usb_stack_area),
									usb_entry_point,
									NULL, NULL, NULL,
									NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_fifo_init(&lora_send_fifo);

	#ifdef ENABLE_BME
	k_thread_start(&bme_thread_data);
	#endif

	#ifdef ENABLE_ZMOD
	k_thread_start(&zmod_thread_data);
	#endif

	#ifdef ENABLE_PM
	k_thread_start(&pm_thread_data);
	#endif

	#ifdef ENABLE_LORAWAN
	k_thread_start(&lora_thread_data);
	#endif

	k_thread_start(&usb_thread_data);
}
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
#include <stdlib.h>
#include <zephyr.h>

/* Aether Headers */
#include "cayenne.h"
#include "lora.h"
#include "delays.h"
#include "threads.h"

/*************************** Sensor Configuration *****************************/

/* Sleep time in between sensor readings */
#define ZMOD_SLEEP 3000
#define BME_SLEEP 3000
#define PM_SLEEP 3000

/**************************** Other Defines ***********************************/

LOG_MODULE_REGISTER(aether);
#define ONE 1
K_FIFO_DEFINE(lora_send_fifo);
// K_SEM_DEFINE(fifo_sem, ONE, ONE);

/*************************** Global Declarations ******************************/
k_tid_t bme_tid, zmod_tid, pm_tid, lora_tid, usb_tid;

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

struct lorawan_downlink_cb downlink_cb = {
  .port = LW_RECV_PORT_ANY,
  .cb = dl_callback
};

#ifndef USE_ABP
int init_lorawan_otaa()
{
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_APP_EUI;
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;
	int ret = 1;


	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		LOG_INF("%s Device not found", DEFAULT_RADIO);
		return ret;
	}

	ret = lorawan_start();
	if (ret < 0) {
		LOG_INF("lorawan_start failed: %d", ret);
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
		LOG_INF("lorawan_join_network failed: %d", ret);
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

	lora_dev = device_get_binding(DEFAULT_RADIO);
	if (!lora_dev) {
		printf("%s Device not found\n", DEFAULT_RADIO);
		return ret;
	}

	ret = lorawan_start();
	if (ret < 0) {
		printf("lorawan_start failed: %d\n", ret);
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

	printf("Joining network over ABP\n");
	ret = lorawan_join(&join_cfg);
	if (ret < 0) {
		printf("lorawan_join_network failed: %d\n", ret);
		return ret;
	}

	return ret;
}
#endif

/**************************** Payload Functions *******************************/

void create_cayenne_segment_2int(intptr_t *packet, int val1, int val2, int index, 
							int channel, int dtype)
{
	int i;

	packet[index] = channel;
	packet[index+1] = dtype;

	uint8_t *val_bytes1 = (uint8_t *) &val1;
	for (i = 0; i < 4; i++)
		packet[index+2+i] = val_bytes1[i];

	uint8_t *val_bytes2 = (uint8_t *) &val2;
	for (i = 0; i < 4; i++)
		packet[index+6+i] = val_bytes2[i];
}

void create_cayenne_segment_1int(intptr_t *packet, int val1, int index, 
									int int_size, int channel, int dtype)
{
	int i;

	packet[index] = channel;
	packet[index+1] = dtype;

	uint8_t *val_bytes1 = (uint8_t *) &val1;
	for (i = 0; i < int_size; i++)
		packet[index+2+i] = val_bytes1[i];
}

void create_bme_payload(intptr_t *t_packet, intptr_t *p_packet, intptr_t *h_packet,
						intptr_t *g_packet,int t1, int t2, int p1, int p2,
						int h1, int h2, int g1, int g2)
{
	create_cayenne_segment_2int(t_packet, t1, t2, 1, CAYENNE_CHANNEL_BME, CAYENNE_TYPE_TEMP_ZEPHYR);
	create_cayenne_segment_2int(p_packet, p1, p2, 1, CAYENNE_CHANNEL_BME, CAYENNE_TYPE_PRESSURE_ZEPHYR);
	create_cayenne_segment_2int(h_packet, h1, h2, 1, CAYENNE_CHANNEL_BME, CAYENNE_TYPE_HUMIDITY_ZEPHYR);
	create_cayenne_segment_2int(g_packet, g1, g2, 1, CAYENNE_CHANNEL_BME, CAYENNE_TYPE_GAS_RES_ZEPHYR);
}

void create_zmod_payload(intptr_t *o3_packet, intptr_t *fast_aqi_packet, int o3_val, int fast_aqi_val)
{
	create_cayenne_segment_1int(o3_packet, o3_val, 1, 4, CAYENNE_CHANNEL_ZMOD, CAYENNE_TYPE_O3_PPB);
	create_cayenne_segment_1int(fast_aqi_packet, fast_aqi_val, 1, 2, CAYENNE_CHANNEL_ZMOD, CAYENNE_TYPE_FAST_AQI);
}

/************************** Thread Entry Functions ****************************/

void bme_entry_point(void *arg1, void *arg2, void *arg3) {
	LOG_MODULE_DECLARE(aether);
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
		intptr_t packet[CAYENNE_TOTAL_SIZE_BME+1];

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

		create_bme_payload(packet, temp.val1, temp.val2, press.val1, press.val2,
						   	humidity.val1, humidity.val2, gas_res.val1,
							gas_res.val2);
		
		LOG_INF("BME packet: ");
		for (int i = 1; i < CAYENNE_TOTAL_SIZE_BME; i++)
			if (i%10 == 0)
				printf("%d | ", (int) packet[i]);
			else
				printf("%d ", (int) packet[i]);
		printf("\n");

		/* Only one thread can access the fifo at a time */
		k_fifo_put(&lora_send_fifo, packet);

		k_msleep(BME_SLEEP);
	}

	#else

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		intptr_t t_packet[CAYENNE_TOTAL_SIZE_TEMP_ZEPHYR+1];
		intptr_t p_packet[CAYENNE_TOTAL_SIZE_PRESSURE_ZEPHYR+1];
		intptr_t h_packet[CAYENNE_TOTAL_SIZE_HUMIDITY_ZEPHYR+1];
		intptr_t g_packet[CAYENNE_TOTAL_SIZE_GAS_RES_ZEPHYR+1];
		int i;

		int t1 = 12, t2 = 13, p1 = 33, p2 = 55, h1 = 32, h2=34, g1=66, g2=99;

		k_busy_wait((uint32_t) 500000);

		printf("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
				t1, t2, p1, p2, h1, h2, g1, g2);

		create_bme_payload(t_packet, p_packet, h_packet, g_packet,
							t1, t2, p1, p2, h1, h2, g1, g2);

		LOG_INF("BME packet: ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_TEMP_ZEPHYR; i++)
			printf("%02x ", (int) t_packet[i]);
		printf("| ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_PRESSURE_ZEPHYR; i++)
			printf("%02x ", (int) p_packet[i]);
		printf("| ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_HUMIDITY_ZEPHYR; i++)
			printf("%02x ", (int) h_packet[i]);
		printf("| ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_GAS_RES_ZEPHYR; i++)
			printf("%02x ", (int) g_packet[i]);
		printf("|\n");

		/* Only one thread can access the fifo at a time */
		k_fifo_put(&lora_send_fifo, t_packet);
		k_fifo_put(&lora_send_fifo, p_packet);
		k_fifo_put(&lora_send_fifo, h_packet);
		k_fifo_put(&lora_send_fifo, g_packet);

		k_msleep(BME_SLEEP);
	}

	#endif
}

void zmod_entry_point(void *arg1, void *arg2, void *arg3) {
	LOG_MODULE_DECLARE(aether);
	#ifdef ZMOD_REAL_DATA

	const struct device *dev_zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
	struct sensor_value fast_aqi, o3_ppb;

  	/* Check if the ZMOD4510 is ready */
	if (!device_is_ready(dev_zmod)) {
		printk("ZMOD4510 is not ready!");
		return;
	}

	while (1) {
		intptr_t packet[CAYENNE_TOTAL_SIZE_ZMOD+1];
		uint8_t o3_packet[CAYENNE_SIZE_O3_PPB+2];
		uint8_t fast_aqi_packet[CAYENNE_SIZE_FAST_AQI+2];

		/* Read data from ZMOD4510 */
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_FAST_AQI, &fast_aqi);
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_O3, &o3_ppb);

		printf("fast aqi: %d", fast_aqi.val1);
		printf("o3 (ppb): %d\n", o3_ppb.val1);

		create_zmod_payload(packet, o3_ppb.val1, fast_aqi.val1);



		LOG_INF("ZMOD packet: ");
		for (int i = 1; i < CAYENNE_TOTAL_SIZE_ZMOD; i++)
			if (i == 6 || i == CAYENNE_TOTAL_SIZE_ZMOD-1)
				printf("%02x | ", (int) packet[i]);
			else
				printf("%02x ", (int) packet[i]);
		printf("\n");

		/* Only one thread can access the fifo at a time */
		k_fifo_put(&lora_send_fifo, packet);

		k_msleep(ZMOD_SLEEP);
	}

	#else

	int o3_ppb = 4423;
	int fast_aqi = 100;

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		intptr_t o3_packet[CAYENNE_TOTAL_SIZE_O3_PPB+1];
		intptr_t fast_aqi_packet[CAYENNE_TOTAL_SIZE_FAST_AQI+1];
		int i;

		k_busy_wait((uint32_t) 500000);
		printf("fast aqi: %d ", fast_aqi);
		printf("o3 (ppb): %d\n", o3_ppb);

		create_zmod_payload(o3_packet, fast_aqi_packet, o3_ppb, fast_aqi);

		LOG_INF("ZMOD packet: ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_O3_PPB; i++)
			printf("%02x ", (int) o3_packet[i]);
		printf("| ");
		for (i = 1; i <= CAYENNE_TOTAL_SIZE_FAST_AQI; i++)
			printf("%02x ", (int) fast_aqi_packet[i]);
		printf("|\n");

		/* Only one thread can access the fifo at a time */
		k_fifo_put(&lora_send_fifo, o3_packet);
		k_fifo_put(&lora_send_fifo, fast_aqi_packet);

		k_msleep(ZMOD_SLEEP);
	}

	#endif
}

void pm_entry_point(void *arg1, void *arg2, void *arg3) {

	#ifdef PM_REAL_DATA
	#else

	/* Simulate reading data from sensor when no sensor connected */
	while (1) {
		//k_busy_wait((uint32_t) 500000);
		//printf("PM sensor sample\n");
		k_yield();
	}
	#endif
}

void lora_entry_point(void *arg1, void *arg2, void *arg3) {
	LOG_MODULE_DECLARE(aether);
	int ret;

	#ifdef LORA_REAL_DATA

	/* Forever attempt to join a LoRaWAN network, yield after failed attempt */
	while (1) {

		#ifdef USE_ABP
		ret = init_lorawan_abp();
		#else
		ret = init_lorawan_otaa();
		#endif

		if (ret < 0) {
			printf("Join Failed!\n");
			k_msleep(3000);
			continue;
		} else {
			printf("Join Success!\n");
			break;
		}

	}

	while (1) {
		intptr_t *data_item;
		printf("==================================\nMain loop lora\n");

		/* If there is nothing in the FIFO, yield the thread */
		while (1) {
			data_item = k_fifo_get(&lora_send_fifo, K_NO_WAIT);

			if (data_item == NULL) {
				printf("fifo empty!\n");
				k_yield();
			} else {
				break;
			}
		}

		/* Get the size of the packet based on the first channel byte. */
		uint8_t data_item_len = 0;
		if (data_item[1] == CAYENNE_CHANNEL_BME) {
			data_item_len = CAYENNE_TOTAL_SIZE_BME;
		} else if (data_item[1] == CAYENNE_CHANNEL_ZMOD) {
			data_item_len = CAYENNE_TOTAL_SIZE_ZMOD;
		} else if (data_item[1] == CAYENNE_CHANNEL_PM) {
			data_item_len = CAYENNE_CHANNEL_PM;
		}

		/* Convert the packet into a send-able packet composed of bytes */
		uint8_t *send_data = malloc(data_item_len);
		for (int i = 1; i <= data_item_len; i++) {
			send_data[i-1] = data_item[i];
		}

		while (1) {

			ret = lorawan_send(2, send_data, data_item_len, LORAWAN_MSG_CONFIRMED);
			if (ret == -EAGAIN) {
				LOG_ERR("lorawan_send failed: %d. Continuing...\n", ret);
				k_sleep(DELAY);
				continue;
			}

			if (ret < 0) {
				printf("lorawan_send failed: %d\n", ret);
				break;
			}

			LOG_INF("LoRa Sending message: ");
			for (int i = 0; i < data_item_len; i++)
				printf("%d ", send_data[i]);
			printf("\n");

			break;
		}

		printf("Free send data!\n==================================\n");
		free(send_data);
	
		k_yield();
	}

	#else
	int data_item;

	/* Simulate sending off LoRa packets */
	while (1) {
		intptr_t *data_item;
		uint8_t data_item_len = 0;

		/* If there is nothing in the FIFO, yield the thread */
		while ((data_item = k_fifo_get(&lora_send_fifo, K_NO_WAIT)) == NULL) {
			k_yield();
		}

		/* Get the size of the packet based on the sensor data type byte. */
		switch (data_item[2]) {
			case CAYENNE_TYPE_TEMP_ZEPHYR:
				data_item_len = CAYENNE_TOTAL_SIZE_TEMP_ZEPHYR;
				break;
			case CAYENNE_TYPE_PRESSURE_ZEPHYR:
				data_item_len = CAYENNE_TOTAL_SIZE_PRESSURE_ZEPHYR;
				break;
			case CAYENNE_TYPE_HUMIDITY_ZEPHYR:
				data_item_len = CAYENNE_TOTAL_SIZE_HUMIDITY_ZEPHYR;
				break;
			case CAYENNE_TYPE_GAS_RES_ZEPHYR:
				data_item_len = CAYENNE_TOTAL_SIZE_GAS_RES_ZEPHYR;
				break;
			case CAYENNE_TYPE_FAST_AQI:
				data_item_len = CAYENNE_TOTAL_SIZE_FAST_AQI;
				break;
			case CAYENNE_TYPE_AQI:
				data_item_len = CAYENNE_TOTAL_SIZE_AQI;
				break;
			case CAYENNE_TYPE_O3_PPB:
				data_item_len = CAYENNE_TOTAL_SIZE_O3_PPB;
				break;
			default:
				data_item_len = 0;
				break;
		}

		/* Convert the packet into a send-able packet composed of bytes */
		uint8_t *send_data = malloc(data_item_len);
		for (int i = 1; i <= data_item_len; i++) {
			send_data[i-1] = data_item[i];
		}

		printf("<%llu> LoRa sending message: ", k_uptime_get());
		for (int i = 0; i < data_item_len; i++)
			printf("%02x ", send_data[i]);
		
		printf("  Data item lengeth: %d", data_item_len);
		printf("\n");

		free(send_data);

		k_busy_wait((uint32_t) 500000);
		printf("Message sent!\n");
		k_yield();
	}

	#endif
}

void usb_entry_point(void *arg1, void *arg2, void *arg3) {

	while (1) {
		//printf("Waiting for USB commands...\n");
		k_yield();
	}
}

void main() 
{
	/* Initialize Threads */
	bme_tid = k_thread_create(&bme_thread_data, bme_stack_area,
								K_THREAD_STACK_SIZEOF(bme_stack_area),
								bme_entry_point,
								NULL, NULL, NULL,
								NORMAL_PRIORITY, 0, K_NO_WAIT);

	zmod_tid = k_thread_create(&zmod_thread_data, zmod_stack_area,
								K_THREAD_STACK_SIZEOF(zmod_stack_area),
								zmod_entry_point,
								NULL, NULL, NULL,
								NORMAL_PRIORITY, 0, K_NO_WAIT);

	pm_tid = k_thread_create(&pm_thread_data, pm_stack_area,
								K_THREAD_STACK_SIZEOF(pm_stack_area),
								pm_entry_point,
								NULL, NULL, NULL,
								NORMAL_PRIORITY, 0, K_NO_WAIT);

	lora_tid = k_thread_create(&lora_thread_data, lora_stack_area,
								K_THREAD_STACK_SIZEOF(lora_stack_area),
								lora_entry_point,
								NULL, NULL, NULL,
								NORMAL_PRIORITY, 0, K_NO_WAIT);

	usb_tid = k_thread_create(&usb_thread_data, usb_stack_area,
								K_THREAD_STACK_SIZEOF(usb_stack_area),
								usb_entry_point,
								NULL, NULL, NULL,
								NORMAL_PRIORITY, 0, K_NO_WAIT);

	k_fifo_init(&lora_send_fifo);

	#ifdef ENABLE_BME
	//k_thread_start(&bme_thread_data);
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

	#ifdef ENABLE_USB
	k_thread_start(&usb_thread_data);
	#endif
}

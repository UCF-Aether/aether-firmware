/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>
 *					  Paul Wood <>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Cayenne channel ids */
#define CAYENNE_CHANNEL_BME     0
#define CAYENNE_CHANNEL_ZMOD    1
#define CAYENNE_CHANNEL_PM      2

/* Cayenne total packet size */
#define CAYENNE_TOTAL_SIZE_BME  16
#define CAYENNE_TOTAL_SIZE_ZMOD 6
#define CAYENNE_TOTAL_SIZE_PM   0

/* Cayenne data types */
#define CAYENNE_TYPE_TEMP_ZEPHYR        0
#define CAYENNE_TYPE_PRESSURE_ZEPHYR    1
#define CAYENNE_TYPE_HUMIDITY_ZEPHYR    2
#define CAYENNE_TYPE_GAS_RES_ZEPHYR     3
#define CAYENNE_TYPE_FAST_AQI           4
#define CAYENNE_TYPE_AQI                5
#define CAYENNE_TYPE_O3_PPB             6

/* Cayenne data type data lengths */
#define CAYENNE_SIZE_TEMP_ZEPHYR        2
#define CAYENNE_SIZE_PRESSURE_ZEPHYR    2
#define CAYENNE_SIZE_HUMIDITY_ZEPHYR    2
#define CAYENNE_SIZE_GAS_RES_ZEPHYR     2
#define CAYENNE_SIZE_FAST_AQI           1
#define CAYENNE_SIZE_AQI                1
#define CAYENNE_SIZE_O3_PPB             2
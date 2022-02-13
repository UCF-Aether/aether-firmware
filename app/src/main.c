/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>, Paul Wood <>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/sensor.h>
#include <drivers/sensor/zmod4510.h>
#include <logging/log.h>
#include <lorawan/lorawan.h>
//#include <include/pm/device.h>
#include <stdio.h>
#include <zephyr.h>
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

LOG_MODULE_REGISTER(aether);

void main(void) {
	int ret;
	const struct device *dev_zmod = DEVICE_DT_GET(DT_NODELABEL(zmod4510));
	struct sensor_value fast_aqi, o3_ppb, aqi;

  while (!device_is_ready(dev_zmod)) {
    LOG_INF("Checking if the device is ready");
    k_sleep(K_MSEC(1000));
  }
	ret = device_is_ready(dev_zmod);
	if (!ret) {
		printk("ZMOD4510 is not ready! Error: %d\n", ret);
		return;
	}
  LOG_INF("zmod4510 is ready");

	while (1) {
    LOG_INF("fucky");
		if (sensor_sample_fetch(dev_zmod)) {
      LOG_ERR("Failed reading sensor!");
      k_sleep(K_MSEC(5000));
      return;
    }
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_FAST_AQI, &fast_aqi);
		sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_O3, &o3_ppb);
    LOG_INF("fast aqi: %d", fast_aqi.val1);
    LOG_INF("o3 (ppb): %d", o3_ppb.val1);
    k_sleep(K_MSEC(1980));
	}
}

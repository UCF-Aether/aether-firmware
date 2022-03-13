/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>
 *            Paul Wood <>
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
#include "sensor/cayenne.h"
#include "sensor/bme688.h"
#include "sensor/zmod4510.h"
#include "sensor/sps30.h"
#include "io/lora.h"
#include "io/usb.h"


LOG_MODULE_REGISTER(aether);


#define BME_STACK_SIZE    1024
#define ZMOD_STACK_SIZE   1024
#define PM_STACK_SIZE     1024
#define LORA_STACK_SIZE   1024
#define USB_STACK_SIZE    1024

#define BME_PRIORITY    5
#define ZMOD_PRIORITY   5
#define PM_PRIORITY     5
#define LORA_PRIORITY   5
#define USB_PRIORITY    5


// TODO: add shit back

K_MSGQ_DEFINE(lora_msgq, sizeof(struct reading), 16, 4);


K_THREAD_STACK_DEFINE(bme_stack_area, BME_STACK_SIZE);
K_THREAD_STACK_DEFINE(zmod_stack_area, ZMOD_STACK_SIZE);
K_THREAD_STACK_DEFINE(pm_stack_area, PM_STACK_SIZE);
K_THREAD_STACK_DEFINE(lora_stack_area, LORA_STACK_SIZE);
K_THREAD_STACK_DEFINE(usb_stack_area, USB_STACK_SIZE);


struct k_thread bme_thread_data;
struct k_thread zmod_thread_data;
struct k_thread pm_thread_data;
struct k_thread lora_thread_data;
struct k_thread usb_thread_data;


void main() 
{
  k_tid_t bme_tid, zmod_tid, pm_tid, lora_tid, usb_tid;


  /* Initialize Threads */
  bme_tid = k_thread_create(&bme_thread_data, bme_stack_area,
                K_THREAD_STACK_SIZEOF(bme_stack_area),
                bme_entry_point,
                &lora_msgq, NULL, NULL,
                BME_PRIORITY, 0, K_NO_WAIT);

  zmod_tid = k_thread_create(&zmod_thread_data, zmod_stack_area,
                K_THREAD_STACK_SIZEOF(zmod_stack_area),
                zmod_entry_point,
                &lora_msgq, NULL, NULL,
                ZMOD_PRIORITY, 0, K_NO_WAIT);

  pm_tid = k_thread_create(&pm_thread_data, pm_stack_area,
                K_THREAD_STACK_SIZEOF(pm_stack_area),
                sps_entry_point,
                &lora_msgq, NULL, NULL,
                PM_PRIORITY, 0, K_NO_WAIT);

  lora_tid = k_thread_create(&lora_thread_data, lora_stack_area,
                K_THREAD_STACK_SIZEOF(lora_stack_area),
                lora_entry_point,
                &lora_msgq, NULL, NULL,
                LORA_PRIORITY, 0, K_NO_WAIT);

  // usb_tid = k_thread_create(&usb_thread_data, usb_stack_area,
  //               K_THREAD_STACK_SIZEOF(usb_stack_area),
  //               usb_entry_point,
  //               NULL, NULL, NULL,
  //               USB_PRIORITY, 0, K_NO_WAIT);

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

  // #ifdef ENABLE_USB
  // k_thread_start(&usb_thread_data);
  // #endif
}

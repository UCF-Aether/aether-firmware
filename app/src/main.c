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
#include "cayenne.h"
#include "lora.h"
#include "delays.h"
#include "threads.h"

LOG_MODULE_REGISTER(aether);
#define ONE 1
K_FIFO_DEFINE(lora_send_fifo);
// K_SEM_DEFINE(fifo_sem, ONE, ONE);

/*************************** Global Declarations ******************************/
k_tid_t bme_tid, zmod_tid, pm_tid, lora_tid, usb_tid;

/******************************************************************************/

// TODO: add shit back


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

  #ifdef ENABLE_USB
  k_thread_start(&usb_thread_data);
  #endif
}

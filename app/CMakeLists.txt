# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aether)

FILE(GLOB app_sources src/main.c src/pm.c)
FILE(GLOB io_sources src/io/*.c)
target_sources(app PRIVATE ${app_sources})
target_sources_ifdef(CONFIG_SENSOR app PRIVATE src/pm_sensor_thread.c src/sensor_thread.c)
target_sources_ifdef(CONFIG_SHELL app PRIVATE src/usb.c)
target_sources_ifdef(CONFIG_LORAWAN app PRIVATE src/lora.c)

add_definitions(
  -DENABLE_ZMOD, 
  -DENABLE_BME,
  -DENABLE_PM,
  -DENABLE_USB,
  -DUSE_ABP
  -DBME_REAL_DATA
  -DZMOD_REAL_DATA
  -DSPS_REAL_DATA
  -DLORA_REAL_DATA
)

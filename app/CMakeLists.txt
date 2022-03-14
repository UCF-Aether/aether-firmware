# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aether)

FILE(GLOB app_sources src/*.c)
FILE(GLOB sensor_entry_sources src/sensor/*.c)
FILE(GLOB io_entry_sources src/io/*.c)
target_sources(app PRIVATE ${app_sources} ${sensor_entry_sources} ${io_entry_sources})

add_definitions(
  -DENABLE_ZMOD, 
  -DENABLE_BME,
  -DENABLE_PM,
  -DENABLE_USB,
  -DUSE_ABP
)

#add_definitions(-DBME_REAL_DATA)
#add_definitions(-DZMOD_REAL_DATA)
#add_definitions(-DZMOD_REAL_DATA, -DBME_REAL_DATA)
# add_definitions(-DZMOD_REAL_DATA, -DBME_REAL_DATA, -DSPS_REAL_DATA, -DLORA_REAL_DATA)

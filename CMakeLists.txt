# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lorawan)

FILE(GLOB app_sources src/*.c)
#target_link_libraries(app PRIVATE /Users/ianw/Aether/Aether-Firmware-Zephyr/src/lib_oaq_2nd_gen.a /Users/ianw/Aether/Aether-Firmware-Zephyr/src/lib_zmod4xxx_cleaning.a)
target_sources(app PRIVATE ${app_sources})


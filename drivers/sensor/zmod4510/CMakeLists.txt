#
# Copyright (c) 2022 Ian Wallace
#
# SPDX-License-Identifier: Apache-2.0
#

zephyr_library()

zephyr_library_sources(zmod4510.c vendor/zmod4xxx.c)
# add_library(zmod4xxx STATIC zmod4xxx.c)
# zephyr_link_libraries(zmod4xxx)
zephyr_library_import(oaq_2nd_gen "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lib_oaq_2nd_gen.a")
# zephyr_library_import(zmod4xxx_cleaning "${CMAKE_CURRENT_SOURCE_DIR}/lib_zmod4xxx_cleaning.a")

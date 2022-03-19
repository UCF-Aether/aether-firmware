/*
 * Aether Sensor Node Firmware
 *
 * Copyright (c) 2022 Ian Wallace <ian.wallace@knights.ucf.edu>
 *					  Paul Wood <>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CAYENNE_H__
#define __CAYENNE_H__

#include <stdint.h>


enum cayenne_type {
  CAYENNE_TYPE_TEMP = 0,
  CAYENNE_TYPE_PRESSURE,
  CAYENNE_TYPE_HUMIDITY,
  CAYENNE_TYPE_GAS_RES,
  CAYENNE_TYPE_FAST_AQI,
  CAYENNE_TYPE_AQI,
  CAYENNE_TYPE_O3,
  CAYENNE_TYPE_PM_1_0,
  CAYENNE_TYPE_PM_2_5,
  CAYENNE_TYPE_PM_4_0,
  CAYENNE_TYPE_PM_10,

  CAYENNE_TYPE_MAX = CAYENNE_TYPE_PM_10,
};

enum cayenne_channel {
  CAYENNE_CHANNEL_BME = 0,
  CAYENNE_CHANNEL_ZMOD,
  CAYENNE_CHANNEL_SPS,
};

union reading_value {
  float f;
  uint8_t u8;
  uint16_t u16;
};

struct reading {
  enum cayenne_channel chan;
  enum cayenne_type type;
  union reading_value val;
};

/*
 * cayenne_packetize
 * 
 * Description: 
 *  Turns a reading struct into a cayenne packet format starting at buffer in big-endian format.
 * Returns: 
 *  The number of bytes written, -1 on error.
 */
int cayenne_packetize(uint8_t *buffer, struct reading *reading);


int get_reading_size(struct reading *reading);

char* cayenne_channel_name(enum cayenne_channel chan);
char* cayenne_type_name(enum cayenne_type type);

#endif /* __CAYENNE_H__ */

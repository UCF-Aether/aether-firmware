#include <stdint.h>
#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>
#include <cayenne.h>

LOG_MODULE_DECLARE(aether);


uint8_t type_sizes[CAYENNE_TYPE_MAX + 1] = {
  [CAYENNE_TYPE_TEMP]     = sizeof(float),
  [CAYENNE_TYPE_PRESSURE] = sizeof(float),
  [CAYENNE_TYPE_HUMIDITY] = sizeof(float),
  [CAYENNE_TYPE_GAS_RES]  = sizeof(float),
  [CAYENNE_TYPE_FAST_AQI] = sizeof(uint16_t),
  [CAYENNE_TYPE_AQI]      = sizeof(uint16_t),
  [CAYENNE_TYPE_O3]       = sizeof(float),
  [CAYENNE_TYPE_PM_1_0]   = sizeof(float),
  [CAYENNE_TYPE_PM_2_5]   = sizeof(float),
  [CAYENNE_TYPE_PM_4_0]   = sizeof(float),
  [CAYENNE_TYPE_PM_10]    = sizeof(float),
};


// TODO: unit test
int get_reading_size(struct reading *reading) {
  if (!reading) {
    return -1;
  }

  return 2 + type_sizes[reading->type];
}


// TODO: unit test
int cayenne_packetize(uint8_t *buffer, struct reading *reading) {
  uint8_t *val_bytes;
  uint8_t num_bytes;

  if (!buffer || !reading) {
    return -1;
  }

  *buffer = reading->chan;
  buffer++;

  *buffer = reading->type;
  buffer++;

  val_bytes = (uint8_t *) &reading->val;
  num_bytes = type_sizes[reading->type];

  for (int i = 0; i < num_bytes; i++) {
    *buffer = val_bytes[i]; // Little endian
    buffer++;
  }

  return 2 + num_bytes;
}


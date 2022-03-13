#include <stdint.h>
#include "cayenne.h"


uint8_t *type_sizes = {
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
  [CAYENNE_TYPE_PM_10_0]  = sizeof(float),
};


int get_reading_size(struct reading *reading) {
  if (reading == NULL) {
    return -1;
  }

  return 2 + type_sizes[reading.type];
}

int cayenne_packetize(uint8_t *buffer, struct reading *reading) {
  uint8_t *val_bytes;
  uint8_t num_bytes;

  if (buffer == NULL || reading == NULL) {
    return -1;
  }

  *buffer = reading.chan;
  buffer++;

  *buffer = reading.type;
  buffer++;

  val_bytes = (uint8_t *) &reading.val;
  num_bytes = type_sizes[reading.type];

  for (int i = 0; i < num_bytes; i++) {
    *buffer = val_bytes[ num_bytes - i - 1];  // Big endian - copy msb first
    buffer++;
  }

  return 2 + num_bytes;
}


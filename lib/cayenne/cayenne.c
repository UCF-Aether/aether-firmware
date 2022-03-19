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

char *cayenne_type_names[CAYENNE_TYPE_MAX + 1] = {
  [CAYENNE_TYPE_TEMP]     = "TEMPERATURE",
  [CAYENNE_TYPE_PRESSURE] = "PRESSURE",
  [CAYENNE_TYPE_HUMIDITY] = "HUMIDITY",
  [CAYENNE_TYPE_GAS_RES]  = "GAS_RES",
  [CAYENNE_TYPE_FAST_AQI] = "FAST_AQI",
  [CAYENNE_TYPE_AQI]      = "AQI",
  [CAYENNE_TYPE_O3]       = "O3",
  [CAYENNE_TYPE_PM_1_0]   = "PM1.0",
  [CAYENNE_TYPE_PM_2_5]   = "PM2.5",
  [CAYENNE_TYPE_PM_4_0]   = "PM4.0",
  [CAYENNE_TYPE_PM_10]    = "PM10",
};

char *cayenne_channel_names[] = {
  [CAYENNE_CHANNEL_BME] = "BME688",
  [CAYENNE_CHANNEL_ZMOD] = "ZOD4510",
  [CAYENNE_CHANNEL_SPS] = "SPS30",
};

char* cayenne_channel_name(enum cayenne_channel chan) {
  return cayenne_channel_names[chan];
}

char* cayenne_type_name(enum cayenne_type type) {
  return cayenne_type_names[type];
}

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
    *buffer = val_bytes[num_bytes - i - 1];
    buffer++;
  }

  return 2 + num_bytes;
}


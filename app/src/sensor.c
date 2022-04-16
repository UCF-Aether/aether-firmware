#include <kernel.h>
#include <drivers/sensor.h>
#include <stdint.h>

int avg_float_sensor_readings(float *avg, const struct device *dev, int channel, int num, k_timeout_t delay) {
  struct sensor_value sv;
  float sum = 0;
  int ret = 0;

  for (int i = 0; i < num; i++) {
    ret = sensor_sample_fetch(dev);
    if (ret) return ret;

    ret = sensor_channel_get(dev, channel, &sv);
    if (ret) return ret;

    sum += (float) sensor_value_to_double(&sv);

    if (!K_TIMEOUT_EQ(K_NO_WAIT, delay)) {
      k_sleep(delay);
    }
  }

  *avg = sum / num;
}

int avg_u16_sensor_readings(uint16_t *avg, const struct device *dev, int channel, int num, k_timeout_t delay) {
  struct sensor_value sv;
  uint16_t sum = 0;
  int ret = 0;

  for (int i = 0; i < num; i++) {
    ret = sensor_sample_fetch(dev);
    if (ret) return ret;

    ret = sensor_channel_get(dev, channel, &sv);
    if (ret) return ret;

    sum += sv.val1;

    if (!K_TIMEOUT_EQ(K_NO_WAIT, delay)) {
      k_sleep(delay);
    }
  }

  *avg = sum / num;
}

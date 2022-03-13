#include <kernel.h>
#include <device.h>
#include <drivers/sensor.h>
#include "bme688.h"
#include "cayenne.h"


// TODO: make configurable - nvm
#define ZMOD_SLEEP 3000

#ifdef ZMOD_REAL_DATA

#define GET_SENSOR() DEVICE_DT_GET(DT_NODELABEL(zmod4510));

#else

// TODO: Create virtual (mock) sensor
// https://docs.zephyrproject.org/latest/reference/drivers/index.html?highlight=device#c.DEVICE_DEFINE

#endif

// TODO: passing in config context
void zmod_entry_point(void *_msgq, void *arg2, void *arg3) {
  struct sensor_value fast_aqi, o3_ppb;
  struct reading reading;
  struct device *dev_zmod = GET_SENSOR();
  struct k_msgq *msgq = (k_msgq *) _msgq;

  /* Check if the BME688 is ready */
  if (!device_is_ready(dev_zmod)) {
    printk("BME688 is not ready!\n");
    return NULL;
  }

  /* Simulate reading data from sensor when no sensor connected */
  while (1) {

    /* Read data from BME688 */
    sensor_sample_fetch(dev_zmod);
    sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_FAST_AQI, &fast_aqi);
    sensor_channel_get(dev_zmod, ZMOD4510_SENSOR_CHAN_O3, &o3_ppb);

    printf("T: %d.%06d; P: %d.%06d; H: %d.%06d; G: %d.%06d\n",
        temp.val1, temp.val2, press.val1, press.val2,
        humidity.val1, humidity.val2, gas_res.val1,
        gas_res.val2);


    reading.chan = CAYENNE_CHANNEL_ZMOD;

    reading.type = CAYENNE_TYPE_O3;
    reading.val.f = (float) sensor_value_to_double(&o3_ppb);
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    reading.type = CAYENNE_TYPE_FAST_AQI;
    reading.val.u16 = fast_aqi.val1;
    k_msgq_put(msgq, &reading, K_NO_WAIT);

    k_msleep(ZMOD_SLEEP);
  }
}

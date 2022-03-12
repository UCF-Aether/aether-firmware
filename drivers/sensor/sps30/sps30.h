#ifndef __SENSOR_SPS30_H__
#define __SENSOR_SPS30_H__

#include <device.h>
#include <drivers/i2c.h>


struct sps30_data {
  float mc_1p0;
  float mc_2p5;
  float mc_4p0;
  float mc_10p0;
  float nc_0p5;
  float nc_1p0;
  float nc_2p5;
  float nc_4p0;
  float nc_10p0;
  float typical_particle_size;
};


struct sps30_config {
  struct i2c_dt_spec bus;
};

#endif /* __SENSOR_SPS30_H__ */

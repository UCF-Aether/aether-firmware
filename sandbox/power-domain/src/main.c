/*
 * Sandbox application to experiment with Zephyr and Aether features.
 *
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
//#include <drivers/power_domain.h>
#include <logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pm/device.h>
#include <pm/device_runtime.h>

//LOG_MODULE_REGISTER(sandbox);
LOG_MODULE_REGISTER(power_domain_gpios, LOG_LEVEL_INF);

#define PWR_5V_DOMAIN DT_NODELABEL(pwr_5v_domain)

static int dev_init(const struct device *dev)
{
    int ret;
    /* enable device runtime power management */
    ret = pm_device_runtime_enable(dev);
    if ((ret < 0) && (ret != -ENOSYS)) {
        return ret;
    }
	return 0;
}

static int domain_pm_action(const struct device *dev,
	enum pm_device_action action)
{
	int rc = 0;

	switch (action) {
	case PM_DEVICE_ACTION_RESUME:
		/* Switch power on */
		pm_device_children_action_run(dev, PM_DEVICE_ACTION_TURN_ON, NULL);
    printk("domain resume");
		break;
	case PM_DEVICE_ACTION_SUSPEND:
		pm_device_children_action_run(dev, PM_DEVICE_ACTION_TURN_OFF, NULL);
    printk("domain suspend");
		break;
	case PM_DEVICE_ACTION_TURN_ON:
    printk("domain turn on");
		break;
	case PM_DEVICE_ACTION_TURN_OFF:
    printk("domain turn off");
		break;
	default:
		rc = -ENOTSUP;
	}

	return rc;
}

void main() {
  int ret;

  
  static const struct device *pwr_5v_domain = DEVICE_DT_GET(DT_NODELABEL(pwr_5v_domain));

  pm_device_runtime_enable(pwr_5v_domain);

  // Device should be suspended now

  while (1) {
    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_RESUME);
    k_msleep(1000);
    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_SUSPEND);
    k_msleep(1000);
  }
}

/*
 * Sandbox application to experiment with Zephyr and Aether features.
 *
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pm/device.h>
#include <pm/device_runtime.h>

LOG_MODULE_REGISTER(sandbox);

#define PWR_5V_DOMAIN DT_NODELABEL(pwr_5v_domain)
//#define RED_LED_1 DT_NODELABEL(red_led_1)
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static int dev_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	return 0;
}

static int domain_pm_action(const struct device *dev,
	enum pm_device_action action)
{
	int rc = 0;

	switch (action) {
	case PM_DEVICE_ACTION_RESUME:
		/* Switch power on */
		pm_device_children_action_run(dev, PM_DEVICE_ACTION_RESUME, NULL);
		break;
	case PM_DEVICE_ACTION_SUSPEND:
		pm_device_children_action_run(dev, PM_DEVICE_ACTION_SUSPEND, NULL);
		break;
	case PM_DEVICE_ACTION_TURN_ON:
		__fallthrough;
	case PM_DEVICE_ACTION_TURN_OFF:
		break;
	default:
		rc = -ENOTSUP;
	}

	return rc;
}

static int led_pm_action(const struct device *dev,
                           enum pm_device_action action)
{
    switch (action) {
    case PM_DEVICE_ACTION_SUSPEND:
        /* suspend the device */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
        break;
    case PM_DEVICE_ACTION_RESUME:
        /* resume the device */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        break;
    case PM_DEVICE_ACTION_TURN_ON:
        /* configure the device into low power mode */
        
        break;
    case PM_DEVICE_ACTION_TURN_OFF:
        /* prepare the device for power down */
        
        break;
    default:
        return -ENOTSUP;
    }

    return 0;
}

void main() {
  int ret;

  
  static const struct device *pwr_5v_domain = DEVICE_DT_GET(DT_NODELABEL(pwr_5v_domain));

	if (!device_is_ready(led.port)) {
		return;
	}
  //gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);


  //pm_device_runtime_init_suspended(led.port);
  //pm_device_runtime_init_suspended(pwr_5v_domain);

  //pm_device_runtime_enable(pwr_5v_domain);
	//pm_device_runtime_enable(led.port);

  while (1) {
    //pm_device_runtime_get(led.port);
    //pm_device_runtime_get(pwr_5v_domain);
    //pm_device_action_run(led.port, PM_DEVICE_ACTION_RESUME);
    //k_msleep(10000);
    //k_busy_wait(K_MSEC(2000));
    pm_device_action_run(led.port, PM_DEVICE_ACTION_SUSPEND);
    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_SUSPEND);
    //k_busy_wait(K_MSEC(2000));
  }
}


PM_DEVICE_DT_DEFINE(PWR_5V_DOMAIN, domain_pm_action);
DEVICE_DT_DEFINE(PWR_5V_DOMAIN, dev_init, PM_DEVICE_DT_GET(PWR_5V_DOMAIN),
		 NULL, NULL, POST_KERNEL, 10, NULL);

PM_DEVICE_DT_DEFINE(LED0_NODE, led_pm_action);
DEVICE_DT_DEFINE(LED0_NODE, dev_init, PM_DEVICE_DT_GET(LED0_NODE),
		 NULL, NULL, POST_KERNEL, 20, NULL);
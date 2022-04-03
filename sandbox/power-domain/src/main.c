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

// gpio device
//#define GPIOB DT_NODELABEL(gpiob)

// LED device
#define RED_LED_1 DT_NODELABEL(red_led_1)

// declaring led device
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(RED_LED_1, gpios);

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
    //gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
		break;
	case PM_DEVICE_ACTION_SUSPEND:
		pm_device_children_action_run(dev, PM_DEVICE_ACTION_TURN_OFF, NULL);
    printk("domain suspend");
    //gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
		break;
	case PM_DEVICE_ACTION_TURN_ON:
		__fallthrough;
	case PM_DEVICE_ACTION_TURN_OFF:
    printk("domain turn off");
		break;
	default:
		rc = -ENOTSUP;
	}

	return rc;
}

// LED actions are NOT printing, that must mean domain_pm_action isnt calling led_pm_action for some reason.
static int led_pm_action(const struct device *dev,
                           enum pm_device_action action)
{
    switch (action) {
    case PM_DEVICE_ACTION_SUSPEND:
        /* suspend the device */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
        printk("led action suspend\n");
        break;
    case PM_DEVICE_ACTION_RESUME:
        /* resume the device */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        printk("led action resume\n");
        break;
    case PM_DEVICE_ACTION_TURN_ON:
        /* configure the device into low power mode */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        printk("led action turn on\n");
        break;
    case PM_DEVICE_ACTION_TURN_OFF:
        /* prepare the device for power down */
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
        printk("led action turn off\n");
        break;
    default:
        return -ENOTSUP;
    }

    return 0;
}

// Control the gpio pin instead? the led is gpio pin 5
// static int gpio_pm_action(const struct device *dev,
//                            enum pm_device_action action)
// {
//     switch (action) {
//     case PM_DEVICE_ACTION_SUSPEND:
//         /* suspend the device */
//         gpio_pin_configure(dev, 5, GPIO_OUTPUT_INACTIVE);
//         break;
//     case PM_DEVICE_ACTION_RESUME:
//         /* resume the device */
//         gpio_pin_configure(dev, 5, GPIO_OUTPUT_ACTIVE);
//         break;
//     case PM_DEVICE_ACTION_TURN_ON:
//         /* configure the device into low power mode */
//         gpio_pin_configure(dev, 5, GPIO_OUTPUT_ACTIVE);
//         break;
//     case PM_DEVICE_ACTION_TURN_OFF:
//         /* prepare the device for power down */
//         gpio_pin_configure(dev, 5, GPIO_OUTPUT_INACTIVE);
//         break;
//     default:
//         return -ENOTSUP;
//     }

//     return 0;
// }

void main() {
  int ret;

  
  static const struct device *pwr_5v_domain = DEVICE_DT_GET(DT_NODELABEL(pwr_5v_domain));

	// if (!device_is_ready(led.port)) {
	// 	return;
	// }
  //gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);


  pm_device_runtime_init_suspended(led.port);
  pm_device_runtime_init_suspended(pwr_5v_domain);

  pm_device_runtime_enable(pwr_5v_domain);
  pm_device_runtime_enable(led.port);

  // Device should be suspended now

  while (1) {

    // The correct PM_DEVICE_ACTION is being printed.
    pm_device_action_run(pwr_5v_domain, PM_DEVICE_ACTION_RESUME);
    pm_device_action_run(led.port, PM_DEVICE_ACTION_RESUME);

  }
}


PM_DEVICE_DT_DEFINE(PWR_5V_DOMAIN, domain_pm_action);
DEVICE_DT_DEFINE(PWR_5V_DOMAIN, dev_init, PM_DEVICE_DT_GET(PWR_5V_DOMAIN),
		 NULL, NULL, POST_KERNEL, 10, NULL);

PM_DEVICE_DT_DEFINE(RED_LED_1, led_pm_action);
DEVICE_DT_DEFINE(RED_LED_1, dev_init, PM_DEVICE_DT_GET(RED_LED_1),
		 NULL, NULL, POST_KERNEL, 20, NULL);

// PM_DEVICE_DT_DEFINE(GPIOB, gpio_pm_action);
// DEVICE_DT_DEFINE(GPIOB, dev_init, PM_DEVICE_DT_GET(GPIOB),
// 		 NULL, NULL, POST_KERNEL, 20, NULL);
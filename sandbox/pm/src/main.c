/*
 * Sandbox application to experiment with Zephyr and Aether features.
 *
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include <pm/pm.h>
#include <pm/device.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

LOG_MODULE_REGISTER(sandbox);

static struct gpio_dt_spec usb_detect = GPIO_DT_SPEC_GET(DT_NODELABEL(usb_wakeup), gpios);
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static struct gpio_callback usb_detect_cb_data;

const struct pm_state_info states[] = PM_STATE_INFO_LIST_FROM_DT_CPU(DT_NODELABEL(cpu0));

void usb_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  if (gpio_pin_get_dt(&usb_detect)) {
    pm_constraint_set(PM_STATE_SUSPEND_TO_IDLE);
    printk("usb inserted\n");
    gpio_pin_set_dt(&led, 1);
  }
  else {
    printk("usb removed\n");
    gpio_pin_set_dt(&led, 0);
    if (!pm_constraint_get(PM_STATE_SUSPEND_TO_IDLE))
      pm_constraint_release(PM_STATE_SUSPEND_TO_IDLE);
  }
}

void main() {
  int ret;

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret) {
    printk("Error %d: failed to configure pin %d\n", ret, led.pin);
    return;
  }

  if (!pm_device_wakeup_enable((struct device *) usb_detect.port, true)) {
    printk("Error: failed to enabled wakeup on %s pin %d\n",
        usb_detect.port->name,
        usb_detect.pin);
    return;
  }

  ret = gpio_pin_configure_dt(&usb_detect, GPIO_INPUT);
  if (ret) {
    printk("Error %d: failed to configure pin %d\n", ret, usb_detect.pin);
    return;
  }

  ret = gpio_pin_interrupt_configure_dt(&usb_detect, GPIO_INT_EDGE_BOTH);
  if (ret) {
    printk("Error %d: failed to configure interupt on %s pin %d\n", 
        ret, 
        usb_detect.port->name, 
        usb_detect.pin);
    return;
  }

  gpio_init_callback(&usb_detect_cb_data, usb_handler, BIT(usb_detect.pin));
  gpio_add_callback(usb_detect.port, &usb_detect_cb_data);
  printk("Set up usb detect at %s pin %d\n", usb_detect.port->name, usb_detect.pin);

  if (gpio_pin_get_dt(&usb_detect)) {
    pm_constraint_set(PM_STATE_SUSPEND_TO_IDLE);
    gpio_pin_set_dt(&led, 1);
  }
  else {
    gpio_pin_set_dt(&led, 0);
  }

  while (1) {
    LOG_INF("testing");
    k_msleep(500);
  }
}

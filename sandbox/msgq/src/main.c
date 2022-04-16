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

#define STACK_SIZE 1024
#define CONSUMER_PRIORITY 1
#define PRODUCER_PRIORITY 0

LOG_MODULE_REGISTER(sandbox);

K_MSGQ_DEFINE(msgq, sizeof(int32_t), 16, 4);

void consumer_entry(void *_msgq, void *arg2, void *arg3) {
  int32_t data;
  int ret;

  printk("starting consumer\n");
  k_msleep(1000);
  while (1) {
    ret = k_msgq_get(&msgq, &data, K_FOREVER);
    printk("consumer got %d ret=%d\n", data, ret);
  }
}

void producer_entry(void *arg1, void *arg2, void *arg3) {
  int32_t data;
  int ret;

  printk("starting producer\n");
  k_msleep(1000);
  while (1) {
    data = rand();
    ret = k_msgq_put(&msgq, &data, K_NO_WAIT);
    printk("producer put %d ret=%d\n", data, ret);
    k_msleep(1000);
  }
}


K_THREAD_DEFINE(consumer_tid, 1024,
                consumer_entry, NULL, NULL, NULL,
                CONSUMER_PRIORITY, 0, 0);

K_THREAD_DEFINE(producer_tid, 1024,
                producer_entry, NULL, NULL, NULL,
                PRODUCER_PRIORITY, 0, 0);

void main() {
  printk("Starting threads\n");
}

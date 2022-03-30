/*
 * Sandbox application to experiment with Zephyr and Aether features.
 *
 */

#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include <stdio.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(sandbox);

void main() {
  while (1) {
    LOG_INF("testing");
    k_msleep(1000);
  }
}

#include <stdint.h>
#include <kernel.h>
#include <device.h>
#include <lorawan/lorawan.h>
#include <logging/log.h>
#include <stdlib.h>
#include <zephyr.h>
#include "lora.h"
#include "../sensor/cayenne.h"

LOG_MODULE_DECLARE(aether);

// TODO: redo lora entry point
// TODO: change max packet size with lorawan_register_dr_changed_callback 

#define LORAWAN_RETRY_DELAY 1000
#define LORAWAN_DELAY       3000

#ifndef USE_ABP

static inline void set_join_cfg(struct lorawan_join_config *config) {
  uint32_t dev_addr = LORAWAN_DEV_ADDR;
  uint8_t dev_eui[] = LORAWAN_DEV_EUI;
  uint8_t app_eui[] = LORAWAN_APP_EUI;
  uint8_t app_skey[] = LORAWAN_APP_SKEY;
  uint8_t nwk_skey[] = LORAWAN_NWK_SKEY;

  config->mode = LORAWAN_ACT_ABP;
  config->dev_eui = dev_eui;
  config->abp.dev_addr = dev_addr;
  config->abp.app_eui = app_eui;
  config->abp.app_skey = app_skey;
  config->abp.nwk_skey = nwk_skey;
}


#else

static inline void set_join_cfg(struct lorawan_join_config *config) {
  uint8_t dev_eui[] = LORAWAN_APP_EUI;
  uint8_t join_eui[] = LORAWAN_JOIN_EUI;
  uint8_t app_key[] = LORAWAN_APP_KEY;

  config->mode = LORAWAN_ACT_OTAA;
  config->dev_eui = dev_eui;
  config->otaa.join_eui = join_eui;
  config->otaa.app_key = app_key;
  config->otaa.nwk_key = app_key;
}

#endif /* USE_ABP */


#ifdef LORA_REAL_DATA

static inline int send(const struct device *lora_dev, uint8_t *buffer, int buffer_len) {
  int ret;

  do {
    ret = lorawan_send(2, buffer, buffer_len, LORAWAN_MSG_CONFIRMED);
    if (ret == -EAGAIN) {
      LOG_ERR("lorawan_send failed: %d. Continuing...\n", ret);
      k_msleep(LORAWAN_RETRY_DELAY);
    }
  } while (ret == -EAGAIN);

  if (ret < 0) {
    LOG_ERR("lorawan_send failed: %d\n", ret);
    return -EINVAL;
  }

  // TODO: log dbg array
  return 0;
}

#else

static inline int send(const struct device *lora_dev, uint8_t *buffer, int buffer_len) {
  // TODO: log dbg array
  return 0;
}

#endif /* LORA_REAL_DATA */


static void dl_callback(uint8_t port, bool data_pending,
      int16_t rssi, int8_t snr,
      uint8_t len, const uint8_t *data)
{
  LOG_DBG("Port %d, Pending %d, RSSI %ddB, SNR %ddBm",
      port, data_pending, rssi, snr);

  if (data) {
    LOG_HEXDUMP_DBG(data, len, "Payload: ");
  }
}

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
  uint8_t unused, max_size;

  lorawan_get_payload_sizes(&unused, &max_size);
  LOG_DBG("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

struct lorawan_downlink_cb downlink_cb = {
  .port = LW_RECV_PORT_ANY,
  .cb = dl_callback
};

void lora_entry_point(void *_msgq, void *arg2, void *arg3) {
  const struct device *lora_dev;
  struct k_msgq *msgq = _msgq;
  struct lorawan_join_config join_cfg;
  struct reading reading;
  int ret = 0;

  // Lowest DR 
  //TODO: change based on DR
  uint8_t buffer[256];
  uint8_t dr_max_bytes = 11;   
  uint8_t num_bytes;

  lora_dev = device_get_binding(DEFAULT_RADIO);
  if (!lora_dev) {
    printk("%s Device not found\n", DEFAULT_RADIO);
    return;
  }

  set_join_cfg(&join_cfg);

  ret = lorawan_start();
  if (ret < 0) {
    printk("lorawan_start failed: %d\n", ret);
    return;
  }

  lorawan_register_downlink_callback(&downlink_cb);
  lorawan_register_dr_changed_callback(lorwan_datarate_changed);
  lorawan_enable_adr(true);

  LOG_INF("Joining lorawan network");
  ret = lorawan_join(&join_cfg);
  if (ret < 0) {
    LOG_ERR("lorawan_join_network failed: %d", ret);
    return;
  }

  // Main loop
  while (1) {
    num_bytes = 0;

    if (k_msgq_num_used_get(msgq) == 0) {
      k_yield();
    }

    // TODO: optimize reading msg queue to not peek then read - store last read if it doesn't fit
    while (num_bytes < dr_max_bytes && k_msgq_num_used_get(msgq) > 0) {
      k_msgq_peek(msgq, (void *) &reading);

      if (num_bytes + get_reading_size(&reading) <= dr_max_bytes) {
        k_msgq_get(msgq, (void *) &reading, K_NO_WAIT);
        num_bytes += cayenne_packetize(buffer + num_bytes, &reading);
      }
    }

    send(lora_dev, buffer, num_bytes);

    k_msleep(LORAWAN_DELAY);
  }
}

# Aether Firmware Zephyr Project
Initialized using the Zephyr LoRaWAN project

## Build Instructions
```
cd ~/zephyrproject/zephyr
rm -rf build
west build -b lora_e5_dev_board ~/aether-firmware
west flash
```
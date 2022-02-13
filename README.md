# Aether Firmware

## Prerequisites
Install a GNU ARM toolchain.

## Initial Setup
Before you can build this application, we must first setup a Zephyr envionment. This can be done as follows:
```
west init -m https://github.com/UCF-Aether/Aether-Firmware-Zephyr aether-workspace
cd aether-workspace
west update
```
Additional environment setup documentation can be found in the [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

## Build Instructions
To build the application, navigate to `/path/to/aether-workspace/zephyr`.

From there run:

`west build -p -b lora_e5_dev_board Aether-Firmware-Zephyr/app`

Finally, running `west flash` will flash the built program to the board.

To view the serial output, open a serial terminal with a baud rate of `115200`.

For example, `picocom -b 115200 /dev/<device_name>`

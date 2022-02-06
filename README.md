# Aether Firmware

## Prerequisites
Install a GNU ARM toolchain.

## Initial Setup
Before you can build this application, we must first setup a Zephyr envionment. This can be done as follows:
```
mkdir aether-workspace
cd aether-workspace
west init -m git@github.com:UCF-Aether/zephyr.git
west update
```
Additional environment setup documentation can be found in the [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

## Build Instructions
To build the application, navigate to `/path/to/aether-workspace/zephyr`.

From there run:

`west build -p auto -b lora_e5_dev_board /path/to/Aether-Firmware-Zephyr`

Finally, running `west flash` will flash the built program to the board.

To view the serial output, open a serial terminal with a baud rate of `115200`.

For example, `picocom -b 115200 /dev/<device_name>`

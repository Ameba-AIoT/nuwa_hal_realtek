hal_realtek — Ameba HAL for Zephyr
####################################

Peripheral driver library for Realtek Ameba SoCs, part of the
`Nuwa Zephyr SDK <https://github.com/Ameba-AIoT/nuwa>`_.

``hal_realtek`` is a Zephyr HAL module containing peripheral drivers,
Wi-Fi, Bluetooth, and USB stacks for Realtek Ameba series SoCs. It is
periodically synced from `ameba-rtos <https://github.com/Ameba-AIoT/ameba-rtos>`_.

Supported Chips
***************

- RTL872xD
- RTL8721Dx
- RTL8721F
- RTL8730E

Structure
*********

.. code-block:: text

   hal_realtek/
   ├── ameba/
   │   ├── <chip>/         SoC-specific peripheral drivers (GPIO, SPI, I2C, UART, …)
   │   ├── common/
   │   │   ├── wifi/       Wi-Fi protocol stack
   │   │   ├── bluetooth/  Bluetooth LE / Classic stack
   │   │   ├── usb/        USB device stack
   │   │   ├── rtk_coex/   Wi-Fi / Bluetooth coexistence
   │   │   └── os_wrapper/ OS abstraction layer
   │   └── scripts/        Toolchain and binary blob update scripts
   └── zephyr/             Zephyr module descriptor (module.yml, binary blobs)

Usage
*****

This repository is fetched automatically when initializing the Nuwa SDK:

.. code-block:: bash

   west init -m https://github.com/Ameba-AIoT/nuwa.git && west update

See the `Nuwa SDK <https://github.com/Ameba-AIoT/nuwa>`_ for build and usage documentation.

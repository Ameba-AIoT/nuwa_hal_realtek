# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)

add_subdirectory(os_wrapper)

add_subdirectory_ifdef(CONFIG_REALTEK_AMEBA_USB_DEVICE usb)
add_subdirectory_ifdef(CONFIG_REALTEK_AMEBA_ZEPHYR_WIFI wifi)

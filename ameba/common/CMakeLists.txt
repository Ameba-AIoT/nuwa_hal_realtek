# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)
add_subdirectory(os_wrapper)
add_subdirectory(crashdump)

zephyr_include_directories_ifdef(CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION loader/mcuboot)
zephyr_library_sources_ifdef(CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION
  loader/mcuboot/ameba_security_counter_utils.c
  loader/mcuboot/mcuboot_security_cnt.c
)

add_subdirectory_ifdef(CONFIG_REALTEK_AMEBA_ZEPHYR_USB usb)
add_subdirectory_ifdef(CONFIG_REALTEK_AMEBA_ZEPHYR_WIFI wifi)
add_subdirectory_ifdef(CONFIG_BT_COEXIST rtk_coex)
add_subdirectory_ifdef(CONFIG_REALTEK_AMEBA_ZEPHYR_BT bluetooth)

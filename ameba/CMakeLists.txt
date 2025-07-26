# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

add_subdirectory(common)

add_subdirectory_ifdef(CONFIG_SOC_AMEBAD amebad)
add_subdirectory_ifdef(CONFIG_SOC_AMEBADPLUS amebadplus)

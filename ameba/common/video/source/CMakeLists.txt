# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_library_sources(video_api.c)
zephyr_library_sources(voe_array.c)

# Both sensors are built in; ../include/sensor.h's USE_SENSOR selects
# which pair (iq_data/sensor_data) is actually used at compile time.
zephyr_library_sources(iq_gc2053.c)
zephyr_library_sources(sensor_gc2053.c)
zephyr_library_sources(iq_imx775.c)
zephyr_library_sources(sensor_imx775.c)
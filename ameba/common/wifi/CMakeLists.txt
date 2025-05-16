# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

if(NOT CONFIG_SINGLE_CORE_WIFI)

zephyr_compile_definitions(ZEPHYR_WIFI=1)
zephyr_compile_definitions(WIFI_ZEPHYR_TODO=1)

zephyr_include_directories(source)

zephyr_library_sources_ifdef(CONFIG_WIFI source/ameba_wificfg_dp.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/rtw_wakelock_zephyr.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/wifi_feature_dis_antdiv.c)

else()
zephyr_compile_definitions(ZEPHYR_WIFI=1)
zephyr_compile_definitions(WIFI_ZEPHYR_TODO=1)
target_link_libraries(app PRIVATE wifi_libs)
zephyr_include_directories(source)

zephyr_library_sources_ifdef(CONFIG_WIFI source/wifi_init_zephyr.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/ameba_wificfg.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/rtw_wakelock_zephyr.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/wifi_feature_dis_antdiv.c)
endif()
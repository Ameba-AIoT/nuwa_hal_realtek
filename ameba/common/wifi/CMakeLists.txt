# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

#project(wifi_library_project)

zephyr_library_named(wifi_lib)

set(SOURCE_FOLDER "${CMAKE_BINARY_DIR}/modules/realtek/ameba/common/wifi")
set(DEST_FOLDER "${CMAKE_BINARY_DIR}/../modules/hal/realtek/ameba/common")

add_custom_target(copy_wifi_lib ALL
    DEPENDS wifi_lib
    COMMENT "Dependency target to run copy command"
)

add_custom_command(
    TARGET wifi_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${SOURCE_FOLDER}/libwifi_lib.a
    ${DEST_FOLDER}/wifi_inic.a
    COMMENT "Copying libwifi_lib.a"
)

add_dependencies(copy_wifi_lib wifi_lib)

zephyr_compile_definitions(ZEPHYR_WIFI=1)
zephyr_compile_definitions(WIFI_ZEPHYR_TODO=1)

zephyr_include_directories(source)
zephyr_include_directories(source/common)

zephyr_include_directories(source/inic)
zephyr_include_directories(source/inic/ipc)
zephyr_include_directories(source/api)

zephyr_library_sources_ifdef(CONFIG_WIFI source/ameba_wificfg.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/common/rom_rtw_queue.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/common/rtw_atomic.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/common/rtw_wakelock_zephyr.c)

# api
zephyr_library_sources_ifdef(CONFIG_AS_INIC_AP source/api/wifi_ind.c)
zephyr_library_sources_ifdef(CONFIG_WIFI source/api/wifi_init_zephyr.c)

zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host_zephyr.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host_api.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host_api_basic.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host_api_ext.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_host_trx.c)
zephyr_library_sources_ifdef(CONFIG_REALTEK_AMEBA_WIFI_INIC source/inic/ipc/inic_ipc_msg_queue.c)


target_link_libraries(app PUBLIC wifi_lib)
#fast conn

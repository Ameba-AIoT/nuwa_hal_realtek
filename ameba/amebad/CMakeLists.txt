# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_BT_COEXIST)
  set(LIB_PATH ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/blobs/${CONFIG_SOC_SERIES}/lib/coex)
else()
  set(LIB_PATH ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/blobs/${CONFIG_SOC_SERIES}/lib)
endif()

zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA
  -Wl,--start-group
  ${LIB_PATH}/lib_wifi_rtk_app.a
  ${LIB_PATH}/lib_wifi_single_core.a
  -Wl,--end-group
  ${LIB_PATH}/lib_wifi_api.a
  ${LIB_PATH}/lib_rtw_event.a
  ${LIB_PATH}/lib_wpa_lite.a
  ${LIB_PATH}/lib_wifi_com_sec.a
)
zephyr_link_libraries_ifdef(CONFIG_BT_COEXIST ${LIB_PATH}/lib_coex.a)

zephyr_compile_options(-mcmse)

zephyr_include_directories(
  include
  source/cmsis
  source/misc
  source/swlib
  source/lib/include
  source/fwlib/include
  source/usrcfg/include
  source/app/monitor/include
)

# used by common
zephyr_library_sources(
  source/fwlib/ram_hp/ameba_system.c
  source/fwlib/ram_common/ameba_ipc_api.c
  source/fwlib/ram_common/ameba_arch.c
  source/swlib/log.c
  source/lib/chipinfo/ameba_rom_patch.c
)

# used by driver, need use zephyr_library_sources_ifdef
zephyr_library_sources_ifdef(CONFIG_ENTROPY_AMEBA_TRNG source/lib/chipinfo/ameba_rand2.c)
zephyr_library_sources_ifdef(CONFIG_ADC_AMEBA source/fwlib/ram_common/ameba_adc.c)
zephyr_library_sources_ifdef(CONFIG_SOC_FLASH_AMEBA source/fwlib/ram_common/ameba_flash_ram.c)
zephyr_library_sources_ifdef(CONFIG_DMA_AMEBA source/fwlib/ram_common/ameba_gdma_ram.c)
zephyr_library_sources_ifdef(CONFIG_I2C_AMEBA source/fwlib/ram_common/ameba_i2c.c)
zephyr_library_sources_ifdef(CONFIG_RTC_AMEBA source/fwlib/ram_common/ameba_rtc.c)
zephyr_library_sources_ifdef(CONFIG_SPI_AMEBA source/fwlib/ram_hp/ameba_ssi.c)
zephyr_library_sources_ifdef(CONFIG_COUNTER_TMR_AMEBA source/fwlib/ram_common/ameba_tim.c)
if(CONFIG_UART_AMEBA_LOGUART OR CONFIG_UART_AMEBA)
    zephyr_library_sources(source/fwlib/ram_common/ameba_uart.c)
endif()
zephyr_library_sources_ifdef(CONFIG_WDT_AMEBA source/fwlib/ram_common/ameba_wdg.c)
zephyr_library_sources_ifdef(CONFIG_WIFI_AMEBA source/misc/ameba_freertos_pmu.c)
zephyr_library_sources_ifdef(CONFIG_WIFI_AMEBA source/swlib/sscanf_minimal.c)

zephyr_link_libraries(
  gcc
  -T${CMAKE_CURRENT_SOURCE_DIR}/ld/ameba_rom_symbol_acut.ld
)

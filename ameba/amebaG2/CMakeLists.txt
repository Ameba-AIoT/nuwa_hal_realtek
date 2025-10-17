# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_compile_options(-mcmse)

set(LIB_PATH ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/blobs/${CONFIG_SOC_SERIES}/lib)
zephyr_link_libraries(${LIB_PATH}/lib_chipinfo.a)

zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_whc_ap.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_rtw_event.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_api.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wpa_lite.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_common.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_coex_api.a)
zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_rtk_app.a)

zephyr_include_directories(include)
zephyr_include_directories(source/fwlib/include)
zephyr_include_directories(source/fwlib/include/cmsis)
zephyr_include_directories(source/fwlib/include/rom)
zephyr_include_directories(source/app/monitor/include)

# # used by common
zephyr_library_sources(source/fwlib/ram_common/ameba_reset.c)
zephyr_library_sources(source/fwlib/ram_common/ameba_clk.c)
zephyr_library_sources(source/fwlib/ram_common/ameba_arch.c)
zephyr_library_sources(source/fwlib/ram_common/ameba_ipc_api.c)
zephyr_library_sources(source/fwlib/ram_common/ameba_ipc_ram.c)
zephyr_library_sources(source/fwlib/ram_common/ameba_rtc.c)

# used by driver, need use zephyr_library_sources_ifdef
zephyr_library_sources_ifdef(CONFIG_ADC_AMEBA source/fwlib/ram_common/ameba_adc.c)
zephyr_library_sources_ifdef(CONFIG_CAN_AMEBA_A2C source/fwlib/ram_common/ameba_a2c.c)
zephyr_library_sources_ifdef(CONFIG_AMEBA_LCDC source/fwlib/ram_common/ameba_lcdc.c)
zephyr_library_sources_ifdef(CONFIG_MIPI_DBI_AMEBA_LCDC source/fwlib/ram_common/ameba_lcdc.c)
zephyr_library_sources_ifdef(CONFIG_INPUT_CTC_AMEBA source/fwlib/ram_common/ameba_captouch.c)
zephyr_library_sources_ifdef(CONFIG_I2S_AMEBA source/fwlib/ram_common/ameba_audio_clock.c)
zephyr_library_sources_ifdef(CONFIG_I2S_AMEBA source/fwlib/ram_common/ameba_pll.c)
zephyr_library_sources_ifdef(CONFIG_AUDIO_AMEBA_DMIC source/fwlib/ram_common/ameba_codec.c)
zephyr_library_sources_ifdef(CONFIG_SOC_FLASH_AMEBA source/fwlib/ram_common/ameba_flash_ram.c)
zephyr_library_sources_ifdef(CONFIG_DMA_AMEBA source/fwlib/ram_common/ameba_gdma_ram.c)
zephyr_library_sources_ifdef(CONFIG_I2C_AMEBA source/fwlib/ram_common/ameba_i2c.c)
zephyr_library_sources_ifdef(CONFIG_LEDC_AMEBA source/fwlib/ram_common/ameba_ledc.c)
zephyr_library_sources_ifdef(CONFIG_SPI_AMEBA source/fwlib/ram_common/ameba_spi.c)
zephyr_library_sources_ifdef(CONFIG_I2S_AMEBA source/fwlib/ram_common/ameba_sport.c)
zephyr_library_sources_ifdef(CONFIG_COUNTER_TMR_AMEBA source/fwlib/ram_common/ameba_tim.c)
zephyr_library_sources_ifdef(CONFIG_COUNTER_TMR_AMEBA source/fwlib/ram_common/ameba_ups.c)
zephyr_library_sources_ifdef(CONFIG_UART_AMEBA source/fwlib/ram_common/ameba_uart.c)
zephyr_library_sources_ifdef(CONFIG_TEMP_AMEBA source/fwlib/ram_common/ameba_thermal.c)
zephyr_library_sources_ifdef(CONFIG_UDC_AMEBA source/fwlib/ram_common/ameba_usb.c)
zephyr_library_sources_ifdef(CONFIG_WIFI_AMEBA source/fwlib/ram_common/ameba_pmu.c)
zephyr_library_sources_ifdef(CONFIG_WIFI_AMEBA source/fwlib/ram_common/ameba_pmctimer.c)

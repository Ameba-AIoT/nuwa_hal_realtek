# Copyright (c) 2026 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

# Pre-compiled libraries
set(LIB_PATH ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/blobs/ameba/${CONFIG_SOC_SERIES}/lib)

if(EXISTS ${LIB_PATH})
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_whc_ap.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_rtw_event.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_api.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wpa_lite.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_com_sec.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_wifi_rtk_app.a)
  zephyr_link_libraries_ifdef(CONFIG_WIFI_AMEBA ${LIB_PATH}/lib_coex_api.a)

  zephyr_link_libraries_ifdef(CONFIG_PM
    ${LIB_PATH}/lib_pmc.a
  )

  zephyr_link_libraries(
    ${LIB_PATH}/lib_chipinfo.a
    ${LIB_PATH}/lib_rom.a
    )

endif()

# Include directories
zephyr_include_directories(
  include
  source/fwlib/include
  source/misc
  source/swlib
)

zephyr_include_directories(
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/include
)

# Base SOC sources
# TODO: Adapt remaining ram_common sources from ameba-rtos to Zephyr APIs
# These files depend on FreeRTOS types (rtos_sema_t, IPC_MSG_STRUCT, etc.)
# and need porting before enabling compilation.
# zephyr_library_sources(
#   source/fwlib/ram_common/ameba_arch.c
# )

zephyr_library_sources(
  source/fwlib/ram_common/ameba_clk.c
  source/fwlib/ram_common/ameba_ipc_api.c
  source/fwlib/ram_common/ameba_ipc_ram.c
  source/fwlib/ram_common/ameba_reset.c
  source/fwlib/ram_hp/ameba_system.c
  source/swlib/log.c
  )

zephyr_library_sources_ifdef(CONFIG_BT source/fwlib/ram_common/ameba_otp.c)

# Peripheral drivers
zephyr_library_sources_ifdef(CONFIG_PINCTRL source/fwlib/ram_common/ameba_pinmux.c)
zephyr_library_sources_ifdef(CONFIG_WIFI_AMEBA source/misc/ameba_pmu.c)
zephyr_library_sources_ifdef(CONFIG_RTC_AMEBA source/fwlib/ram_common/ameba_rtc.c)
zephyr_library_sources_ifdef(CONFIG_GPIO_AMEBA source/fwlib/ram_common/ameba_gpio.c)
zephyr_library_sources_ifdef(CONFIG_SOC_FLASH_AMEBA source/fwlib/ram_common/ameba_flash_ram.c)
zephyr_library_sources_ifdef(CONFIG_DMA_AMEBA source/fwlib/ram_common/ameba_gdma_ram.c)
zephyr_library_sources_ifdef(CONFIG_I2C_AMEBA source/fwlib/ram_common/ameba_i2c.c)
zephyr_library_sources_ifdef(CONFIG_LEDC_AMEBA source/fwlib/ram_common/ameba_ledc.c)
zephyr_library_sources_ifdef(CONFIG_SPI_AMEBA source/fwlib/ram_hp/ameba_spi.c)
zephyr_library_sources_ifdef(CONFIG_COUNTER_TMR_AMEBA source/fwlib/ram_common/ameba_tim.c)
zephyr_library_sources_ifdef(CONFIG_UART_AMEBA source/fwlib/ram_hp/ameba_uart.c)
zephyr_library_sources_ifdef(CONFIG_UART_AMEBA_LOGUART source/fwlib/ram_common/ameba_loguart.c)
zephyr_library_sources_ifdef(CONFIG_TEMP_AMEBA source/fwlib/ram_common/ameba_thermal.c)
zephyr_library_sources_ifdef(CONFIG_ADC_AMEBA source/fwlib/ram_common/ameba_adc.c)
zephyr_library_sources_ifdef(CONFIG_SDHC_AMEBA source/fwlib/ram_common/ameba_sd.c)
zephyr_library_sources_ifdef(CONFIG_AMEBA_LCDC source/fwlib/ram_hp/ameba_lcdc.c)
zephyr_library_sources_ifdef(CONFIG_MIPI_DSI_AMEBA source/fwlib/ram_hp/ameba_mipi.c)

zephyr_library_sources_ifdef(CONFIG_PM
  source/fwlib/ram_common/ameba_psram.c
  source/fwlib/ram_common/ameba_pll.c
  source/fwlib/ram_common/ameba_aontimer.c
  source/fwlib/ram_hp/ameba_pmc.c
)

zephyr_library_sources_ifdef(CONFIG_MCUBOOT
  loader/bootloader/bootloader_hp.c
  loader/bootloader/boot_ota_hp.c
  source/fwlib/ram_common/ameba_psram.c
  source/fwlib/ram_common/ameba_pll.c
  source/fwlib/ram_hp/ameba_flashclk.c
)

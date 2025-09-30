zephyr_compile_options(
	-mcmse
	-DCONFIG_PLATFORM_8735B
	-DCONFIG_BUILD_RAM
)

zephyr_link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/lib/libsoc_ntz.a)
zephyr_link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/lib/libvideo_ntz.a)

zephyr_include_directories(
	include
	source/cmsis/rtl8735b/include
	source/cmsis/rtl8735b/lib/include
	source/fwlib/rtl8735b/include
	source/fwlib/rtl8735b/lib/include
	source/fwlib/rtl8735b/lib/source/ram/video

	source/misc/utilities/include
	source/app/stdio_port
	source/app/rtl_printf/include
)

zephyr_library_sources(
	source/fwlib/rtl8735b/source/ram/hal_timer.c
	source/fwlib/rtl8735b/source/ram_s/hal_wdt.c
	source/fwlib/rtl8735b/source/ram/hal_gdma.c
	source/fwlib/rtl8735b/source/ram/hal_gpio.c
)

if(CONFIG_VIDEO_AMEBAPRO2 OR CONFIG_I2C_AMEBAPRO2)
	zephyr_library_sources(source/fwlib/rtl8735b/source/ram/hal_i2c.c)
endif()

zephyr_library_sources_ifdef(CONFIG_UART_AMEBAPRO2_LOGUART source/fwlib/rtl8735b/source/ram/hal_uart.c)
zephyr_library_sources_ifdef(CONFIG_SPI_AMEBAPRO2 source/fwlib/rtl8735b/source/ram/hal_ssi.c)
zephyr_library_sources_ifdef(CONFIG_SPI_AMEBAPRO2 source/fwlib/rtl8735b/source/ram/rtl8735b_ssi.c)
zephyr_library_sources_ifdef(CONFIG_ENTROPY_AMEBAPRO2_TRNG source/fwlib/rtl8735b/source/ram_s/hal_trng_sec.c)
zephyr_library_sources_ifdef(CONFIG_SOC_FLASH_AMEBAPRO2 source/fwlib/rtl8735b/source/ram/hal_spic.c)
zephyr_library_sources_ifdef(CONFIG_SOC_FLASH_AMEBAPRO2 source/fwlib/rtl8735b/source/ram/hal_flash.c)

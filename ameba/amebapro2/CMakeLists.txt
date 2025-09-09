zephyr_compile_options(
	-mcmse
	-DCONFIG_PLATFORM_8735B
	-DCONFIG_BUILD_RAM
)

zephyr_link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/lib/libsoc_ntz.a)

zephyr_include_directories(
	include
	source/cmsis/rtl8735b/include
	source/cmsis/rtl8735b/lib/include
	source/fwlib/rtl8735b/include
	source/fwlib/rtl8735b/lib/include

	source/misc/utilities/include
	source/app/stdio_port
	source/app/rtl_printf/include
)

zephyr_library_sources(
	source/fwlib/rtl8735b/source/ram/hal_timer.c
	source/fwlib/rtl8735b/source/ram_s/hal_wdt.c
)

zephyr_library_sources_ifdef(CONFIG_UART_AMEBAPRO2_LOGUART source/fwlib/rtl8735b/source/ram/hal_uart.c)
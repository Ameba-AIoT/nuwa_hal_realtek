zephyr_include_directories(inc)

zephyr_library_sources_ifdef(CONFIG_BT hci/bt_normal_patch.c)
zephyr_library_sources_ifdef(CONFIG_BT hci/hci_platform.c)
zephyr_library_sources_ifdef(CONFIG_BT hci/hci_uart.c)


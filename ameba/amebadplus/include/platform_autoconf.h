
/*
 * < CONFIG SOC
 */
#undef  CONFIG_SOC_EXT_FLASH

/*
 * < CONFIG TrustZone
 */
#undef  CONFIG_TRUSTZONE

/*
 * < CONFIG Link Option
 */
#define CONFIG_IMG1_FLASH 1
#undef  CONFIG_IMG1_SRAM
#define CONFIG_IMG2_FLASH 1
#undef  CONFIG_IMG2_PSRAM

/*
 * < CONFIG Mass Production
 */
#undef  CONFIG_MP_INCLUDED

/*
 * < CONFIG Shell CMD
 */
#undef  CONFIG_NEW_ATCMD
#undef  CONFIG_LONGER_CMD

/*
 * < CONFIG VFS
 */
#define CONFIG_VFS_LITTLEFS_INCLUDED 1
#undef  CONFIG_VFS_FATFS_INCLUDED

/*
 * < CONFIG OTA OPTION
 */
#undef  CONFIG_UPGRADE_BOOTLOADER

/*
 * < CONFIG WIFI
 */
#define CONFIG_WLAN_MENU 1
#define CONFIG_KM0_NP_KM4_AP 1
#undef  CONFIG_WIFI_KM4_SINGLE_CORE
#define CONFIG_INIC_INTF_IPC 1
#undef  CONFIG_INIC_INTF_SDIO
#undef  CONFIG_HIGH_TP_TEST
#define CONFIG_WLAN 1
#define CONFIG_AS_INIC_AP 1
#undef  CONFIG_ENABLE_WPS
#undef  CONFIG_WIFI_CSI_ENABLE

/*
 * < CONFIG LWIP
 */
#undef  CONFIG_FAST_DHCP

/*
 * < CONFIG BT
 */
#undef  CONFIG_BT_MENU


/*
 * < MENUCONFIG FOR KM4 CONFIG
 */

/*
 * < CONFIG CHIP
 */
#define CONFIG_AMEBADPLUS 1
#define ARM_CORE_CM4 1
#undef  CONFIG_FPGA
#undef  CONFIG_AMEBADPLUS_A_CUT
#define CONFIG_AMEBADPLUS_B_CUT 1

/*
 * < CONFIG OS
 */
#define CONFIG_KERNEL 1
#define PLATFORM_FREERTOS 1
#define TASK_SCHEDULER_DISABLED (0)

/*
 * < CONFIG USB
 */
#undef  CONFIG_USB_DEVICE_EN
#define  CONFIG_USB_FS

/*
 * < CONFIG FUNCTION TEST
 */
#undef  CONFIG_PER_TEST

/*
 * < CONFIG SECURE TEST
 */
#undef  CONFIG_SEC_VERIFY

/*
 * < LWIP Config
 */
#undef  CONFIG_IP_NAT

/*
 * < SSL Config
 */
#define CONFIG_USE_MBEDTLS_ROM 1
#define CONFIG_MBED_TLS_ENABLED 1
#undef  CONFIG_SSL_ROM_TEST

/*
 * < 802154 Config
 */
#undef  CONFIG_802154_EN

/*
 * < GUI Config
 */
#undef  CONFIG_GUI_EN

/*
 * < Audio Config
 */
#undef  CONFIG_AUDIO_FWK
#undef  CONFIG_MEDIA_PLAYER

/*
 * < IMQ Config
 */
#undef  CONFIG_IMQ_EN

/*
 * < Wifi Audio Config
 */
#undef  CONFIG_WIFI_AUDIO

/*
 * < Build Option
 */
#define CONFIG_TOOLCHAIN_ASDK 1
#undef  CONFIG_TOOLCHAIN_ARM_GCC
#undef  CONFIG_LINK_ROM_LIB
#define CONFIG_LINK_ROM_SYMB 1


# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(source/common)
zephyr_include_directories(source/device/core)
zephyr_include_directories_ifdef(CONFIG_USBD_CDC_ACM source/device/cdc_acm)
zephyr_include_directories_ifdef(CONFIG_USBD_HID source/device/hid)
zephyr_include_directories_ifdef(CONFIG_USBD_MSC source/device/msc)
zephyr_include_directories_ifdef(CONFIG_USBD_COMPOSITE source/device/composite)

zephyr_library_sources_ifdef(CONFIG_USBD_CDC_ACM source/device/cdc_acm/usbd_cdc_acm.c)
zephyr_library_sources_ifdef(CONFIG_USBD_HID source/device/hid/usbd_hid.c)
zephyr_library_sources_ifdef(CONFIG_USBD_COMPOSITE_CDC_ACM_HID
  source/device/composite/usbd_composite_cdc_acm_hid.c
  source/device/composite/usbd_composite_cdc_acm.c
  source/device/composite/usbd_composite_hid.c
)

zephyr_library_sources_ifdef(CONFIG_USBD_COMPOSITE_CDC_ACM_MSC
  source/device/composite/usbd_composite_cdc_acm_msc.c
  source/device/composite/usbd_composite_cdc_acm.c
  source/device/composite/usbd_composite_msc.c
  source/device/composite/usbd_composite_scsi.c
)

zephyr_linker_sources_ifdef(CONFIG_USBD_COMPOSITE_CDC_ACM_MSC
  SECTIONS source/device/composite/usbd_composite_msc.ld
)

zephyr_library_sources_ifdef(CONFIG_USBD_MSC
  source/device/msc/usbd_msc.c
  source/device/msc/usbd_scsi.c
)

zephyr_linker_sources_ifdef(CONFIG_USBD_MSC
  SECTIONS source/device/msc/usbd_msc.ld
)

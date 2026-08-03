# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(source/common)
zephyr_include_directories_ifdef(CONFIG_USB_DEVICE_AMEBA source/device/core)
zephyr_include_directories_ifdef(CONFIG_USBD_CDC_ACM source/device/cdc_acm)
zephyr_include_directories_ifdef(CONFIG_USBD_HID source/device/hid)
zephyr_include_directories_ifdef(CONFIG_USBD_MSC source/device/msc)
zephyr_include_directories_ifdef(CONFIG_USBD_COMPOSITE source/device/composite)
zephyr_include_directories_ifdef(CONFIG_USBD_VENDOR source/device/vendor)

zephyr_library_sources_ifdef(CONFIG_USBD_CDC_ACM source/device/cdc_acm/usbd_cdc_acm.c)
zephyr_library_sources_ifdef(CONFIG_USBD_HID source/device/hid/usbd_hid.c)

zephyr_library_sources_ifdef(CONFIG_USBD_MSC
  source/device/msc/usbd_msc.c
  source/device/msc/usbd_scsi.c
)

zephyr_linker_sources_ifdef(CONFIG_USBD_MSC
  SECTIONS source/device/msc/usbd_msc.ld
)

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

zephyr_library_sources_ifdef(CONFIG_USBD_COMPOSITE_CDC_ACM_UAC
  source/device/composite/usbd_composite_cdc_acm_uac.c
  source/device/composite/usbd_composite_cdc_acm.c
  source/device/composite/usbd_composite_uac1.c
)

zephyr_library_sources_ifdef(CONFIG_USBD_COMPOSITE_HID_UAC
  source/device/composite/usbd_composite_hid_uac.c
  source/device/composite/usbd_composite_hid_bi_dir.c
  source/device/composite/usbd_composite_uac1.c
)

zephyr_library_sources_ifdef(CONFIG_USBD_VENDOR
  source/device/vendor/usbd_vendor.c
)

zephyr_include_directories_ifdef(CONFIG_USBD_DFU source/device/dfu)
zephyr_library_sources_ifdef(CONFIG_USBD_DFU
  source/device/dfu/usbd_dfu.c
)

zephyr_include_directories_ifdef(CONFIG_USBH_UVC source/host/uvc)
zephyr_library_sources_ifdef(CONFIG_USBH_UVC
  source/host/uvc/usbh_uvc.c
  source/host/uvc/usbh_uvc_class.c
  source/host/uvc/usbh_uvc_parse.c
  source/host/uvc/usbh_uvc_stream.c
)

zephyr_include_directories_ifdef(CONFIG_USB_HOST_AMEBA source/host/core)
zephyr_include_directories_ifdef(CONFIG_USBH_CDC_ACM source/host/cdc_acm)
zephyr_include_directories_ifdef(CONFIG_USBH_DFU source/host/dfu)
zephyr_include_directories_ifdef(CONFIG_USBH_VENDOR source/host/vendor)
zephyr_include_directories_ifdef(CONFIG_USBH_UAC source/host/uac)
zephyr_include_directories_ifdef(CONFIG_USBH_COMPOSITE source/host/composite)

zephyr_library_sources_ifdef(CONFIG_USBH_CDC_ACM source/host/cdc_acm/usbh_cdc_acm.c)
zephyr_library_sources_ifdef(CONFIG_USBH_DFU source/host/dfu/usbh_dfu.c)
zephyr_library_sources_ifdef(CONFIG_USBH_VENDOR source/host/vendor/usbh_vendor.c)
zephyr_library_sources_ifdef(CONFIG_USBH_UAC source/host/uac/usbh_uac1.c)

zephyr_library_sources_ifdef(CONFIG_USBH_COMPOSITE_HID_UAC
  source/host/composite/usbh_composite_hid_uac.c
  source/host/composite/usbh_composite_hid.c
  source/host/composite/usbh_composite_uac1.c
)

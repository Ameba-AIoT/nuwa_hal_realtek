# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_compile_options(-mcmse)

set(LIB_PATH ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/blobs/${CONFIG_SOC_SERIES}/lib)
zephyr_link_libraries(${LIB_PATH}/lib_chipinfo.a)

zephyr_include_directories(
  common
  dwl
  jpeg
  inc
  pp
)

# used by jpeg
zephyr_library_sources(
  jpeg/jpegdecapi.c
  jpeg/jpegdechdrs.c
  jpeg/jpegdecinternal.c
  jpeg/jpegdecscan.c
  jpeg/jpegdecutils.c
  pp/ppapi.c
  pp/ppinternal.c
  common/commonconfig.c
  common/regdrv.c
  dwl/dwl.c
)

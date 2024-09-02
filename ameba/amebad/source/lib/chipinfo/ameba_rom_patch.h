/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AMEBA_ROM_PATCH_H_
#define _AMEBA_ROM_PATCH_H_

/* Exported constants --------------------------------------------------------*/
/** @defgroup ROM_PATCH_Exported_Constants ROM_PATCH Exported Constants
  * @{
  */

// add your constants here

/**
  * @}
  */

/* Exported functions -------------------------------------------------------- */
/** @defgroup ROM_PATCH_Exported_Functions ROM_PATCH Exported Functions
  * @{
  */

char *__real_strncpy(char *dst0, const char *src0, int count);
char *__real_strcpy(char *s1, const char *s2);
long __real_strtol(const char *nptr, char **endptr, int base);
/**
  * @}
  */

#endif /* _AMEBA_ROM_PATCH_H_ */

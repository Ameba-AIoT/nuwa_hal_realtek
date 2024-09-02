/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"
#include <string.h>
#include <stdio.h>
#include <build_info.h>

#define CHIP_INFO_PRINT_BUF(buf, buf_used_bytes, buflen, temp, ...)	\
do { \
    if(buf_used_bytes < buflen) { \
		temp = DiagSnPrintf(buf + buf_used_bytes, buflen - buf_used_bytes, ##__VA_ARGS__); \
		buf_used_bytes += temp; \
	} \
} while(0)

static const char *const TAG = "CHIPINFO";
/**
* @brif CHIPINFO_TypeDef maintains the BD number and corresponding OTP information.
*/
const CHIPINFO_TypeDef Chip_Info[] = {
	//subnum	pkgnum		bdnumer		memory_type

	{0xFF,		0xFF,		0xFFFF,		MEMORY_NOT_CLEAR},
};

/**
  * @brief Get BD numer form OTP.
  * @retval BD number
  */
u16 ChipInfo_BDNum(void)
{
	u16 BDnum = 0;
	assert_param(0);

	return BDnum;
}

u32 ChipInfo_MCM_Info(void)
{
	u32 memoryinfo = 0;
	assert_param(0);

	return memoryinfo;
}

bool ChipInfo_PsramExists(void)
{
	return TRUE;
}

void ChipInfo_GetSocName_ToBuf(char *buf, size_t buflen)
{
	const char *str = __FILE__;
	const char *start = "soc/";
	const char *end = "/lib";
	char socname[20];

	u32 buf_used_bytes = 0, temp;

	const char *startPtr = strstr(str, start);
	if (startPtr == NULL) {
		RTK_LOGE(TAG, "Invalid SOC PATH!\n");
		return;
	}
	startPtr += strlen(start);

	const char *endPtr = strstr(startPtr, end);
	if (endPtr == NULL) {
		RTK_LOGE(TAG, "Invalid SOC PATH!\n");
		return;
	}

	size_t namelen = MIN((size_t)(endPtr - startPtr), sizeof(socname));
	strncpy(socname, startPtr, namelen);
	socname[namelen] = '\0';
	CHIP_INFO_PRINT_BUF(buf, buf_used_bytes, buflen, temp, "%s: ", socname);

#ifdef AMD_BACK2MSATER_TODO
	u32 index;
	int bResult;
	u8 data;

	/* dump otp 0x7F0~0x800 but no meaning */
	for (index = OTP_CHIPVER; index <= OTP_CHIPINFO; index++) {
		bResult = OTP_Read8(index, &data);
		if (bResult != RTK_SUCCESS) {
			RTK_LOGE(TAG, "ERR: Get Soc Name Fail\n");
			break;
		}
		CHIP_INFO_PRINT_BUF(buf, buf_used_bytes, buflen, temp, "%02x", data);
	}
#endif
	CHIP_INFO_PRINT_BUF(buf, buf_used_bytes, buflen, temp, "\r\n");
}

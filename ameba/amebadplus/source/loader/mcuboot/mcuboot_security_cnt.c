/*
 * Copyright (c) 2025 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ameba_soc.h>
#include "bootutil/bootutil_public.h"
#include "ameba_security_counter_utils.h"

fih_ret boot_nv_security_counter_init(void)
{
	FIH_RET(FIH_SUCCESS);
}

fih_ret boot_nv_security_counter_get(uint32_t image_id, fih_int *security_counter)
{
	uint32_t current_counter;
	int32_t ret;

	if (!security_counter) {
		RTK_LOGE(NOTAG, "security_counter is NULL");
		FIH_RET(FIH_FAILURE);
	}

	ret = ameba_sec_counter_get(image_id, &current_counter);
	if (ret != 0) {
		RTK_LOGE(NOTAG, "Failed to get counter for image_id=%u", image_id);
		FIH_RET(FIH_FAILURE);
	}

	RTK_LOGI(NOTAG, "boot_nv_security_counter_get: %u\n", current_counter);
	*security_counter = fih_int_encode(current_counter);
	FIH_RET(FIH_SUCCESS);
}

int32_t boot_nv_security_counter_update(uint32_t image_id, uint32_t img_security_cnt)
{
	int32_t ret;

	ret = ameba_sec_counter_update(image_id, img_security_cnt);
	if (ret != RTK_SUCCESS) {
		return -BOOT_EBADSTATUS;
	}

	RTK_LOGW(NOTAG, "boot_nv_security_counter_update: img: %u, cnt: %u\n", image_id, img_security_cnt);
	return RTK_SUCCESS;
}

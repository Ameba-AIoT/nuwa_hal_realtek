/*
 * Copyright (c) 2025 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ameba_soc.h>
#include "bootutil/bootutil_public.h"
#include "ameba_security_counter_utils.h"

/**
 * @brief Initialize the security counter subsystem
 *
 * This function is called during mcuboot initialization to set up
 * the non-volatile counter system. For Ameba platform, no special
 * initialization is required as OTP access is always available.
 *
 * @return FIH_SUCCESS on success, FIH_FAILURE on error
 */
fih_ret boot_nv_security_counter_init(void)
{
	FIH_RET(FIH_SUCCESS);
}

/**
 * @brief Get the security counter value for an image
 *
 * Reads the current security counter value from OTP for the specified
 * image. The counter is encoded in OTP as the number of trailing zero bits.
 *
 * @param[in]  image_id         Image identifier (0 for secure, 1 for non-secure)
 * @param[out] security_counter Pointer to store the counter value
 *
 * @return FIH_SUCCESS on success, FIH_FAILURE on error
 */
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

/**
 * @brief Update the security counter for an image
 *
 * Updates the security counter in OTP memory. The counter can only be
 * increased, never decreased (rollback protection). The counter is
 * encoded in OTP by clearing bits from LSB upwards.
 *
 * @param[in] image_id         Image identifier (0 for secure, 1 for non-secure)
 * @param[in] img_security_cnt New security counter value
 *
 * @return 0 on success
 * @return -BOOT_EBADARGS if arguments are invalid
 * @return -BOOT_EBADSTATUS if OTP operation fails
 */
int32_t boot_nv_security_counter_update(uint32_t image_id, uint32_t img_security_cnt)
{
	int32_t ret;

	ret = ameba_sec_counter_update(image_id, img_security_cnt);
	if (ret != RTK_SUCCESS) {
		/* Map internal error codes to mcuboot error codes */
		return -BOOT_EBADSTATUS;
	}

	RTK_LOGW(NOTAG, "boot_nv_security_counter_update: img: %u, cnt: %u\n", image_id, img_security_cnt);
	return RTK_SUCCESS;
}

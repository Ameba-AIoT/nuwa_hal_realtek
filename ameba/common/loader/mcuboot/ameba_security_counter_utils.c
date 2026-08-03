/*
 * Copyright (c) 2025 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ameba_soc.h>
#include "ameba_security_counter_utils.h"

/*
 * OTP_USER_START (see ameba_otpc.h) is 0x380 on every Ameba SoC that has
 * shipped this file so far (g2, dplus, smart) — the first word of the OTP
 * user-programmable region. IMG0 and IMG1 currently alias the same word:
 * there is one shared counter regardless of image_id, not one per image. A
 * SoC that needs independent per-image counters can #define these three
 * ahead of including this header to move IMG0/IMG1 to distinct OTP words.
 */
#ifndef SEC_COUNTER_IMG0
#define SEC_COUNTER_IMG0 0x380 /* sysreg_sec 0x380 [31:0] */
#pragma message "Using default SEC_COUNTER_IMG0 = 0x380"
#endif

#ifndef SEC_COUNTER_IMG1
#define SEC_COUNTER_IMG1 0x380 /* sysreg_sec 0x380 [31:0] */
#pragma message "Using default SEC_COUNTER_IMG1 = 0x380"
#endif

#ifndef SEC_COUNTER_NONE
#define SEC_COUNTER_NONE 0x380
#pragma message "Using default SEC_COUNTER_NONE = 0x380"
#endif

bool ameba_sec_counter_get_addr(uint32_t image_id, uint32_t *addr)
{
	if (addr == NULL) {
		return false;
	}

	switch (image_id) {
	case 0u:
		*addr = SEC_COUNTER_IMG0;
		return true;
	case 1u:
		*addr = SEC_COUNTER_IMG1;
		return true;
	case 255u:
		*addr = SEC_COUNTER_NONE;
		return true;
	default:
		return false;
	}
}

void ameba_sec_counter_read_words(uint32_t addr, uint32_t *buf)
{
	for (uint8_t w = 0; w < WORD_CNT; w++) {
		OTP_Read32(addr + w * sizeof(uint32_t), &buf[w]);
	}
}

uint32_t ameba_sec_counter_count_trailing_zeros(const uint32_t *words)
{
	uint32_t counter = 0;

	for (uint8_t w = 0; w < WORD_CNT; w++) {
		uint32_t value = words[w];

		for (uint8_t bit = 0; bit < WORD_BITS; bit++) {
			if ((value >> bit) & 1u) {
				return counter;
			}
			counter++;
		}
	}
	return counter;
}

void ameba_sec_counter_build_target_words(uint32_t img_security_cnt, uint32_t *target_words)
{
	uint32_t bit_pos = img_security_cnt;

	/* Initially set to all 1 */
	for (uint8_t w = 0; w < WORD_CNT; w++) {
		target_words[w] = 0xFFFFFFFFu;
	}

	for (uint8_t w = 0; w < WORD_CNT; w++) {
		if (bit_pos >= WORD_BITS) {
			/* This word is all 0 */
			target_words[w] = 0u;
			bit_pos -= WORD_BITS;
		} else {
			target_words[w] = ((~0u) << bit_pos);
			break;
		}
	}
}

int32_t ameba_sec_counter_write_bytes(uint32_t addr, const uint32_t *target_value)
{
	for (uint8_t w = 0; w < WORD_CNT; w++) {
		for (uint8_t i = 0; i < sizeof(uint32_t); i++) {
			uint8_t target_byte = (target_value[w] >> (i * 8)) & 0xFFu;

			if (OTP_Write8(addr + w * sizeof(uint32_t) + i, target_byte) !=
				RTK_SUCCESS) {
				RTK_LOGE(NOTAG, "OTP_Write8 failed at addr=0x%08x, byte=0x%02x",
						 addr + w * sizeof(uint32_t) + i, target_byte);
				return RTK_FAIL;
			}
		}
	}
	return RTK_SUCCESS;
}

int32_t ameba_sec_counter_get(uint32_t image_id, uint32_t *security_counter)
{
	uint32_t addr;
	uint32_t current_value[WORD_CNT];

	if (!security_counter) {
		RTK_LOGE(NOTAG, "security_counter is NULL");
		return RTK_FAIL;
	}
	if (!ameba_sec_counter_get_addr(image_id, &addr)) {
		RTK_LOGE(NOTAG, "Invalid image_id=%u", image_id);
		return RTK_FAIL;
	}

	ameba_sec_counter_read_words(addr, current_value);
	*security_counter = ameba_sec_counter_count_trailing_zeros(current_value);
	RTK_LOGI(NOTAG, "Get counter=%u, image_id=%u", *security_counter, image_id);

	return RTK_SUCCESS;
}

int32_t ameba_sec_counter_update(uint32_t image_id, uint32_t img_security_cnt)
{
	uint32_t addr;
	uint32_t current_counter;
	uint32_t current_value[WORD_CNT];
	uint32_t target_value[WORD_CNT];
	int32_t ret;

	if (img_security_cnt > MAX_SEC_COUNTER_BITS) {
		RTK_LOGE(NOTAG, "Invalid security count: %u (max: %u)", img_security_cnt,
				 MAX_SEC_COUNTER_BITS);
		return RTK_FAIL;
	}

	if (!ameba_sec_counter_get_addr(image_id, &addr)) {
		RTK_LOGE(NOTAG, "Invalid image_id=%u", image_id);
		return RTK_FAIL;
	}
	ameba_sec_counter_read_words(addr, current_value);
	current_counter = ameba_sec_counter_count_trailing_zeros(current_value);

	/* Rollback not allowed */
	if (img_security_cnt < current_counter) {
		RTK_LOGE(NOTAG, "Rollback attempt: current=%u, requested=%u", current_counter,
				 img_security_cnt);
		return RTK_FAIL;
	}

	/* If the values are the same, no update is required */
	if (img_security_cnt == current_counter) {
		return RTK_SUCCESS;
	}

	/* Set the low img_security_cnt bits to 0, and the rest to 1 */
	ameba_sec_counter_build_target_words(img_security_cnt, target_value);
	for (uint8_t i = 0; i < WORD_CNT; i++) {
		if ((current_value[i] & target_value[i]) != target_value[i]) {
			RTK_LOGE(NOTAG, "OTP violation: 0->1 attempt (current=0x%08x, target=0x%08x)",
					 current_value[i], target_value[i]);
			return RTK_FAIL;
		}
	}

	ret = ameba_sec_counter_write_bytes(addr, target_value);
	if (ret != 0) {
		return ret;
	}

	RTK_LOGI(NOTAG, "Update success - image_id=%u, current=%u -> %u", image_id, current_counter,
			 img_security_cnt);
	return RTK_SUCCESS;
}

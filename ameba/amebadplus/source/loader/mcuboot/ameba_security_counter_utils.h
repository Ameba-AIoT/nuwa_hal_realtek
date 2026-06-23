/*
 * Copyright (c) 2025 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AMEBA_SECURITY_COUNTER_UTILS_H
#define AMEBA_SECURITY_COUNTER_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#ifndef SEC_COUNTER_IMG0
#define SEC_COUNTER_IMG0 0x380 /* sysreg_sec 0x380 [31:0] for km4 */
#warning "Using default SEC_COUNTER_IMG0 = 0x380"
#endif

#ifndef SEC_COUNTER_IMG1
#define SEC_COUNTER_IMG1 0x380 /* sysreg_sec 0x380 [31:0] for km0 */
#warning "Using default SEC_COUNTER_IMG1 = 0x380"
#endif

#ifndef SEC_COUNTER_NONE
#define SEC_COUNTER_NONE 0x380
#endif

#define MAX_SEC_COUNTER_BITS 32u
#define WORD_BITS            32u
#define WORD_CNT             (MAX_SEC_COUNTER_BITS / WORD_BITS)

bool ameba_sec_counter_get_addr(uint32_t image_id, uint32_t *addr);
void ameba_sec_counter_read_words(uint32_t addr, uint32_t *buf);
uint32_t ameba_sec_counter_count_trailing_zeros(const uint32_t *words);
void ameba_sec_counter_build_target_words(uint32_t img_security_cnt, uint32_t *target_words);
int32_t ameba_sec_counter_write_bytes(uint32_t addr, const uint32_t *target_value);
int32_t ameba_sec_counter_get(uint32_t image_id, uint32_t *security_counter);
int32_t ameba_sec_counter_update(uint32_t image_id, uint32_t img_security_cnt);

#endif /* AMEBA_SECURITY_COUNTER_UTILS_H */

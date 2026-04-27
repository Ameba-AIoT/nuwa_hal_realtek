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
#define SEC_COUNTER_IMG0 0x380 /* sysreg_sec 0x380 [31:0] for km4tz */
#warning "Using default SEC_COUNTER_IMG0 = 0x380"
#endif

#ifndef SEC_COUNTER_IMG1
#define SEC_COUNTER_IMG1 0x380 /* sysreg_sec 0x380 [31:0] for km4ns */
#warning "Using default SEC_COUNTER_IMG1 = 0x380"
#endif

#ifndef SEC_COUNTER_NONE
#define SEC_COUNTER_NONE 0x380
#endif

#define MAX_SEC_COUNTER_BITS 32u
#define WORD_BITS            32u
#define WORD_CNT             (MAX_SEC_COUNTER_BITS / WORD_BITS)

/**
 * @brief Get the OTP address for a given image ID
 *
 * @param[in]  image_id  Image identifier (0 or 1)
 * @param[out] addr      Pointer to store the OTP address
 *
 * @return true if image_id is valid, false otherwise
 */
bool ameba_sec_counter_get_addr(uint32_t image_id, uint32_t *addr);

/**
 * @brief Read WORD_CNT 32-bit words from OTP
 *
 * @param[in]  addr  Starting OTP address
 * @param[out] buf   Buffer to store the read values (must hold WORD_CNT elements)
 */
void ameba_sec_counter_read_words(uint32_t addr, uint32_t *buf);

/**
 * @brief Count the number of consecutive 0 bits from LSB
 *
 * This function counts trailing zeros in the word array, stopping at
 * the first 1 bit encountered. This implements the "Hamming weight" counter
 * where the counter value is encoded as the number of zero bits.
 *
 * @param[in] words  Array of WORD_CNT 32-bit words
 *
 * @return The count of trailing zero bits (0 to MAX_SEC_COUNTER_BITS)
 */
uint32_t ameba_sec_counter_count_trailing_zeros(const uint32_t *words);

/**
 * @brief Build the target word pattern for a given security counter value
 *
 * The counter is encoded such that a value of N means the low N bits are 0
 * and the remaining bits are 1.
 * Examples:
 *   value 0 -> 0xFFFFFFFF (all 1s, no bits flipped to 0)
 *   value 1 -> 0xFFFFFFFE (bit 0 is 0)
 *   value 2 -> 0xFFFFFFFC (bits 0-1 are 0)
 *
 * @param[in]  img_security_cnt  The target security counter value
 * @param[out] target_words    Array to store the resulting word pattern
 */
void ameba_sec_counter_build_target_words(uint32_t img_security_cnt, uint32_t *target_words);

/**
 * @brief Write the target word pattern to OTP
 *
 * Writes the target values byte-by-byte to OTP. This function verifies that
 * no 0->1 transitions are attempted (OTP bits can only be programmed from 1 to 0).
 *
 * @param[in] addr          Starting OTP address
 * @param[in] target_value  Array of target word values
 *
 * @return 0 on success, negative error code on failure
 */
int32_t ameba_sec_counter_write_bytes(uint32_t addr, const uint32_t *target_value);

/**
 * @brief Get the current security counter value for an image
 *
 * This is a convenience function that reads the OTP and converts
 * the bit pattern to a counter value.
 *
 * @param[in]  image_id         Image identifier (0 or 1)
 * @param[out] security_counter Pointer to store the counter value
 *
 * @return 0 on success, negative error code on failure
 */
int32_t ameba_sec_counter_get(uint32_t image_id, uint32_t *security_counter);

/**
 * @brief Update the security counter for an image
 *
 * This function updates the security counter in OTP. The counter can only
 * be incremented, not decremented (rollback protection).
 *
 * @param[in] image_id         Image identifier (0 or 1)
 * @param[in] img_security_cnt New security counter value
 *
 * @return 0 on success, negative error code on failure
 */
int32_t ameba_sec_counter_update(uint32_t image_id, uint32_t img_security_cnt);

#endif /* AMEBA_SECURITY_COUNTER_UTILS_H */

/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _HAL_AMEBA_H_
#define _HAL_AMEBA_H_

#include <basic_types.h>

#define CACHE_LINE_SIZE                     32U
#define CACHE_LINE_ADDR_MSK                 (~(CACHE_LINE_SIZE - 1U))
#define CACHE_LINE_ALIGMENT(x)              (((uint32_t)(x) + (CACHE_LINE_SIZE - 1U)) & CACHE_LINE_ADDR_MSK)

void hal_delay_us(uint32_t time_us);
#define hal_delay_ms(time_ms)               do {hal_delay_us((time_ms)*1000);} while (0)
#define DelayUs                             hal_delay_us
#define DelayMs                             hal_delay_ms

#endif //_HAL_AMEBA_H_

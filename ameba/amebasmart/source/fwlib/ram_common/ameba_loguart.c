/*
 * Copyright (c) 2026 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"

/* TX path 4 is the CA32 (CPU_AP, ARMv8-A AArch32) path.  KM0 uses path 1
 * and KM4 uses path 3.  Catch accidental cross-core compilation with a
 * portable architecture check that works in both Zephyr and ameba-rtos SDK.
 * __ARM_ARCH_8A__ is defined by GCC/LLVM for Cortex-A32 (-mcpu=cortex-a32)
 * but not for Cortex-M33 (__ARM_ARCH_8M_MAIN__) or Cortex-M23 (__ARM_ARCH_8M_BASE__).
 */
#ifndef __ARM_ARCH_8A__
#error "ameba_loguart.c: TX path 4 is CA32-only. Port via LOG_UART_IDX_FLAG for KM0/KM4."
#endif

/**
 * @brief  Check if there is space in the LOGUART TX path FIFO.
 * @retval 1 if TX path FIFO is not full (writable), 0 if full.
 * @note   On AmebaSmart the CA32 (CPU_AP) always uses TX path 4.
 *         Unlike amebadplus which dispatches via LOG_UART_IDX_FLAG[CPUID],
 *         amebasmart has no such table in its HAL; the path assignment is
 *         fixed: KM0=path1, KM4=path3, CA32=path4.
 */
u8 LOGUART_Writable(void)
{
	LOGUART_TypeDef *UARTLOG = LOGUART_DEV;

	return (UARTLOG->LOGUART_UART_LSR & LOGUART_BIT_TP4F_NOT_FULL) ? 1U : 0U;
}

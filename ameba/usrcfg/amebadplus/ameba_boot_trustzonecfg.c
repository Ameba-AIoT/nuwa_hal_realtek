/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"

#ifdef CONFIG_TRUSTZONE
/*
 * MPC configuration for TF-M BL2 on amebadplus (RTL872XDA).
 *
 * Reference: sdk2/component/soc/usrcfg/amebadplus/ameba_boot_trustzonecfg.c
 *
 * mpc1_config controls SRAM (S1: 0x20000000~0x2007FFFF, 512KB).
 * Three active entries mirror the sdk2 layout, with entry2 adjusted for TF-M:
 *
 *  entry0  0x20000000 ~ 0x20006FFF  NS
 *    ROM BSS (KM0/KM4), NS MSP stacks, NS RTOS static areas used by ROM code.
 *    Fixed hardware layout, identical to sdk2.
 *
 *  entry1  0x20008000 ~ 0x20008FFF  NS
 *    KM0_RTOS_STATIC_0_NS: KM0 RTOS static segment.
 *    Fixed hardware layout, identical to sdk2.
 *    (0x20007000~0x20007FFF is intentionally not NS — Secure-reserved gap.)
 *
 *  entry2  NS_RAM_ALIAS_BASE ~ 0x20100000-1  NS
 *    TF-M adjustment of sdk2's "__km4_bd_ram_start__ ~ 0x20100000-1":
 *    sdk2 starts from __km4_bd_ram_start__ (= 0x2000B020 when TZ is off),
 *    but in TF-M that range [0x2000B020, 0x20036020) is Secure SRAM (tfm_s).
 *    We start from NS_RAM_ALIAS_BASE (0x20037020) instead, so S SRAM is never
 *    covered by a NS MPC entry.  The upper bound 0x20100000 exceeds the
 *    physical SRAM top (0x20080000); MPC ignores non-existent addresses.
 *
 * mpc2_config controls PSRAM (0x60000000+), identical to sdk2.
 */
#include "region_defs.h"   /* NS_RAM_ALIAS_BASE */

const TZ_CFG_TypeDef mpc2_config[MPC_ENTRY_NUM] = {
/*  Start                           End                                CTRL */
    {(u32)__km4_bd_psram_start__,   (u32)__non_secure_psram_end__ - 1, MPC_RW | MPC_NS}, /* PSRAM NS */
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS}, /* End */
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
    {0xFFFFFFFF,                    0xFFFFFFFF,                        MPC_RW | MPC_NS},
};

const TZ_CFG_TypeDef mpc1_config[MPC_ENTRY_NUM] = {
/*  Start           End              CTRL */
    {0x20000000,    0x20006FFF,      MPC_RW | MPC_NS}, /* entry0: ROM BSS, NS MSP/RTOS stacks (same as sdk2) */
    {0x20008000,    0x20008FFF,      MPC_RW | MPC_NS}, /* entry1: KM0_RTOS_STATIC_0_NS (same as sdk2) */
    {NS_RAM_ALIAS_BASE, 0x20100000 - 1, MPC_RW | MPC_NS}, /* entry2: NS SRAM (TF-M: start after S SRAM) */
    {0xFFFFFFFF,    0xFFFFFFFF,      MPC_RW | MPC_NS}, /* End */
    {0xFFFFFFFF,    0xFFFFFFFF,      MPC_RW | MPC_NS},
    {0xFFFFFFFF,    0xFFFFFFFF,      MPC_RW | MPC_NS},
    {0xFFFFFFFF,    0xFFFFFFFF,      MPC_RW | MPC_NS},
    {0xFFFFFFFF,    0xFFFFFFFF,      MPC_RW | MPC_NS},
};

#endif /* CONFIG_TRUSTZONE */

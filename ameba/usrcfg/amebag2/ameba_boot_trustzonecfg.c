/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"

#ifdef CONFIG_TRUSTZONE
/*
 * MPC configuration for TF-M BL2 on amebagreen2 (RTL8721F).
 *
 * Reference: sdk0/component/soc/usrcfg/amebagreen2/ameba_boot_trustzonecfg.c
 *
 * These strong-symbol definitions override the _WEAK stubs in lib_bootloader.a
 * so that BOOT_SecureChip_MPCCfg() configures the MPC with the correct
 * Secure/NS boundaries when BOOT_TZCfg_IsSecChip() returns TRUE.
 *
 * mpc1_config: Flash (MPC1, 0x02000000+)
 *   All flash is NS read-only (XIP code is NS or Secure-via-SAU).
 *
 * mpc2_config: SRAM S2 (MPC2, 0x20000000~0x2009FFFF, 640KB)
 *   entry0: 0x20001000~0x20005000 NS (ROM BSS NS, MSP_NS, IPC shared mem)
 *   entry1: NS_RAM_ALIAS_BASE ~ 0x200A0000 NS (TF-M NS SRAM; starts after S SRAM)
 *   NS_RAM_ALIAS_BASE = S SRAM end (depends on TFM_ISOLATION_LEVEL).
 *
 * mpc3_config: PSRAM (MPC3)
 *   All PSRAM is NS. lib's BOOT_SecureChip_MPCCfg substitutes
 *   ChipInfo_PsramBoundary()-1 at runtime when end == __non_secure_psram_end__-1.
 */
#include "region_defs.h"   /* NS_RAM_ALIAS_BASE */

const TZ_CFG_TypeDef mpc1_config[MPC_ENTRY_NUM] = {    /* Flash */
    {0x02000000,        0xFFFFFFFF,         MPC_RO | MPC_NS},   /* entry0: Flash NS read-only */
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},   /* End */
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
};

const TZ_CFG_TypeDef mpc2_config[MPC_ENTRY_NUM] = {    /* SRAM S2 */
    {0x20001000,        0x20005000 - 1,     MPC_RW | MPC_NS},   /* entry0: ROM BSS NS, MSP_NS, IPC */
    {NS_RAM_ALIAS_BASE, 0x200A0000 - 1,     MPC_RW | MPC_NS},   /* entry1: TF-M NS SRAM */
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},   /* End */
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
};

const TZ_CFG_TypeDef mpc3_config[MPC_ENTRY_NUM] = {    /* PSRAM */
    {(u32)__non_secure_psram_start__, (u32)__non_secure_psram_end__ - 1, MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},   /* End */
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
    {0xFFFFFFFF,        0xFFFFFFFF,         MPC_RW | MPC_NS},
};

#endif /* CONFIG_TRUSTZONE */

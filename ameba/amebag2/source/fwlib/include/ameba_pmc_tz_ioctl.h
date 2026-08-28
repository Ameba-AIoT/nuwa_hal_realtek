/*
 * Copyright (c) 2025, Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AMEBA_PMC_TZ_IOCTL_H_
#define _AMEBA_PMC_TZ_IOCTL_H_

#include <stdint.h>

/*
 * AmebaG2 only: contract between the non-secure PM code and the TF-M platform
 * service for handing peripherals over to the non-secure zone across a
 * power-gated sleep.
 *
 * Why it is needed at all: with CONFIG_TRUSTZONE the bootloader takes the
 * SecureChip path (Boot_Tzcfg_En -> BOOT_TZCfg_IsSecChip() ->
 * BOOT_SecureChip_TZCfg()), and BOOT_SecureChip_PeriCfg() narrows
 * REG_LSYS_SEC_PPC_CTRL to a whitelist that leaves BPC_PSRAM / BPC_SPIC /
 * BPC_CPU0 secure-only. But a power-gated AP is powered back up by the
 * non-secure NP/LP side, which needs exactly those IPs. Without the handover the
 * AP powers down fine and then nobody can wake it: both cores stay dark and the
 * IWDG resets the chip after 1~2 s.
 *
 * Why the non-secure side cannot do it itself: every field of
 * REG_LSYS_SEC_PPC_CTRL is marked DD_SEC: S, i.e. secure-write-only. The ameba
 * secure world exposes SOCPS_PeriPermissionEntry() as a PMC_ENTRY (NSC veneer)
 * for this; under TF-M there is no such veneer, so the same operation goes
 * through the platform service (tfm_platform_hal_ioctl).
 *
 * The masks and their timing mirror FreeRTOS' AP-side vPortSystemPowerOff()
 * (component/soc/amebagreen2/lib/pmc/ameba_pmc_km4tz_lib.c): release before the
 * sleep, reclaim only PSRAM and SPIC afterwards, leaving CPU0 and PMC with the
 * non-secure zone. That reproduces the FreeRTOS steady state exactly
 * (PPC_CTRL = 0xff00f0ff released, 0xcf00f0ff reclaimed).
 *
 * This header is deliberately in the HAL: both the TF-M platform_s target and
 * the Zephyr non-secure build have fwlib/include on their include path, so the
 * request ID and the masks cannot drift apart across the two repositories.
 *
 * Include <ameba_soc.h> before this header -- the masks are the LSYS_BIT_BPC_*
 * register bits from sysreg_lsys.h.
 */

/* tfm_platform_ioctl() request ID, 'RTKP' */
#define AMEBA_PMC_TZ_IOCTL_PPC_PERMISSION 0x52544b50

/* Handed to the non-secure zone before a power-gated sleep. */
#define AMEBA_PMC_TZ_PPC_SLEEP_RELEASE                                                             \
	(LSYS_BIT_BPC_PSRAM | LSYS_BIT_BPC_SPIC | LSYS_BIT_BPC_CPU0 | LSYS_BIT_BPC_PMC)

/*
 * Taken back once the AP is running again. CPU0 and PMC stay with the non-secure
 * zone, as they do under FreeRTOS after the first power-gated sleep.
 */
#define AMEBA_PMC_TZ_PPC_WAKE_RECLAIM (LSYS_BIT_BPC_PSRAM | LSYS_BIT_BPC_SPIC)

/*
 * Everything the secure side is willing to act on. Enforced there, so a
 * compromised non-secure world cannot use this call to grant itself unrelated
 * IPs (CRYPTO, PKE, TRNG, ...); the SDK's NSC veneer does not check this.
 */
#define AMEBA_PMC_TZ_PPC_ALLOWED AMEBA_PMC_TZ_PPC_SLEEP_RELEASE

/*
 * psa_invec payload. release != 0 sets the mask bits (IP controlled by the
 * non-secure zone), release == 0 clears them (secure-only).
 */
struct ameba_pmc_tz_ppc_request {
	uint32_t ip_mask;
	uint32_t release;
};

#endif /* _AMEBA_PMC_TZ_IOCTL_H_ */

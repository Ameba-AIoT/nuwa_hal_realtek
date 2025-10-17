/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "ameba_soc.h"
#include "os_wrapper.h"
#include "ameba_v8m_crashdump.h"
#include "ameba_fault_handle.h"

#include "ameba_system.h"
//#include "psram_reserve.h"
static const char *const TAG = "APP";
extern int main(void);
struct _driver_call_os_func_map driver_call_os_func_map;
void NS_ENTRY BOOT_IMG3(void);
void app_init_psram(void);
extern void app_init_debug(void);

extern void newlib_locks_init(void);

#if defined (__GNUC__)
/* Add This for C++ support to avoid compile error */
void _init(void) {}
#endif

u32 app_mpu_nocache_check(u32 mem_addr)
{
	mpu_region_config mpu_cfg;

	mpu_cfg.region_base = (uint32_t)__ram_nocache_start__;
	mpu_cfg.region_size = __ram_nocache_end__ - __ram_nocache_start__;

	if ((mem_addr >= mpu_cfg.region_base) && (mem_addr < (mpu_cfg.region_base + mpu_cfg.region_size))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

u32 app_mpu_nocache_init(void)
{
	mpu_region_config mpu_cfg;
	u32 mpu_entry = 0;

	/* ROM Code inside CPU does not enter Cache, Set to RO for NULL ptr access error */
	mpu_entry = mpu_entry_alloc();
	mpu_cfg.region_base = 0x1010A000;
	mpu_cfg.region_size = 0x101D4000 - 0x1010A000;
	mpu_cfg.xn = MPU_EXEC_ALLOW;
	mpu_cfg.ap = MPU_UN_PRIV_RO;
	mpu_cfg.sh = MPU_NON_SHAREABLE;
	mpu_cfg.attr_idx = MPU_MEM_ATTR_IDX_NC;
	mpu_region_cfg(mpu_entry, &mpu_cfg);

	mpu_entry = mpu_entry_alloc();
	mpu_cfg.region_base = (uint32_t)__ram_nocache_start__;
	mpu_cfg.region_size = __ram_nocache_end__ - __ram_nocache_start__;
	mpu_cfg.xn = MPU_EXEC_ALLOW;
	mpu_cfg.ap = MPU_UN_PRIV_RW;
	mpu_cfg.sh = MPU_NON_SHAREABLE;
	mpu_cfg.attr_idx = MPU_MEM_ATTR_IDX_NC;
	if (mpu_cfg.region_size >= 32) {
		mpu_region_cfg(mpu_entry, &mpu_cfg);
	}

	/* set 1KB retention ram no-cache */
	mpu_entry = mpu_entry_alloc();
	mpu_cfg.region_base = 0x000C0000;
	mpu_cfg.region_size = 0x400;
	mpu_cfg.xn = MPU_EXEC_ALLOW;
	mpu_cfg.ap = MPU_UN_PRIV_RW;
	mpu_cfg.sh = MPU_NON_SHAREABLE;
	mpu_cfg.attr_idx = MPU_MEM_ATTR_IDX_NC;
	mpu_region_cfg(mpu_entry, &mpu_cfg);

	/* set No-Security PSRAM Memory Write-Back */
	mpu_entry = mpu_entry_alloc();
	mpu_cfg.region_base = 0x02000000;
	mpu_cfg.region_size = 0x400000;
	mpu_cfg.xn = MPU_EXEC_ALLOW;
	mpu_cfg.ap = MPU_UN_PRIV_RW;
	mpu_cfg.sh = MPU_NON_SHAREABLE;
	mpu_cfg.attr_idx = MPU_MEM_ATTR_IDX_WB_T_RWA;
	mpu_region_cfg(mpu_entry, &mpu_cfg);

	return 0;
}

u32 app_mpu_s_nocache_init(void)
{
#if defined (configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1U)
	mpu_s_no_cache_init();
#endif
	return 0;
}

void app_vdd1833_detect(void)
{
	u32 temp;

	if (FALSE == is_power_supply18()) {
		temp = HAL_READ32(SYSTEM_CTRL_BASE_HP, REG_HS_RFAFE_IND_VIO1833);
		temp |= BIT_RFAFE_IND_VIO1833;
		HAL_WRITE32(SYSTEM_CTRL_BASE_HP, REG_HS_RFAFE_IND_VIO1833, temp);
	}

	RTK_LOGI(TAG, "REG_HS_RFAFE_IND_VIO1833 (0 is 1.8V): %x\n", HAL_READ32(SYSTEM_CTRL_BASE_HP, REG_HS_RFAFE_IND_VIO1833));
}

static void rtos_critical_enter_mcu(void)
{
	rtos_critical_enter(RTOS_CRITICAL_SOC);
}

static void rtos_critical_exit_mcu(void)
{
	rtos_critical_exit(RTOS_CRITICAL_SOC);
}

/*initialize driver call os_function map*/
static void app_driver_call_os_func_init(void)
{
	driver_call_os_func_map.driver_enter_critical = rtos_critical_enter_mcu;
	driver_call_os_func_map.driver_exit_critical = rtos_critical_exit_mcu;
}

// The Main App entry point
void app_start(void)
{
	/* enable non-secure cache */
	Cache_Enable(ENABLE);
	_memset((void *) __bss_start__, 0, (__bss_end__ - __bss_start__));

	irq_table_init(MSP_RAM_HP_NS); /* NS Vector table init */
	/* Redirect hardfault for debug, and register function pointer to print task information when a crash occurs */
	Fault_Hanlder_Redirect(vTaskCrashCallback);

	RTK_LOGI(TAG, "KM4 BOOT REASON: %x \n", BOOT_Reason());
	RTK_LOGI(TAG, "TrustZone_IsSecure: %x \n", TrustZone_IsSecure());

	SystemSetCpuClk(CONFIG_CPU_CLOCK_SEL_VALUE);

	SOCPS_InitSYSIRQ_HP();

	/* configure FreeRTOS interrupt and heap region. After heap is configured, printf can be used*/
	rtos_mem_init();

#if defined (configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1U)
	BOOT_IMG3();
#endif

#if defined (__GNUC__)
	extern void __libc_init_array(void);
	/* Add This for C++ support */
	__libc_init_array();
#endif
	newlib_locks_init();

	mpu_init();
	app_mpu_nocache_init();
	app_mpu_s_nocache_init();

	app_vdd1833_detect();
	memcpy_gdma_init();

	app_driver_call_os_func_init();

	/* Force SP align to 8bytes */
	__asm(
		"ldr r1, =#0xFFFFFFF8\n"
		"mov r0, sp \n"
		"and r0, r0, r1\n"
		"mov sp, r0\n"
	);

	main(); /* project/xxxx/src/main.c */
}

IMAGE2_ENTRY_SECTION
RAM_START_FUNCTION Img2EntryFun0 = {
	app_start,
	NULL,//BOOT_RAM_WakeFromPG,
	(u32)NewVectorTable
};


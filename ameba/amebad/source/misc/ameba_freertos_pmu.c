/*
* Copyright (c) 2024 Realtek Semiconductor Corp.
*
* SPDX-License-Identifier: Apache-2.0
*/

#include "ameba_soc.h"
#include "basic_types.h"
#include "os_wrapper.h"
#include "os_wrapper_specific.h"

// static const char *const TAG = "PMU";
uint32_t missing_tick = 0;

static uint32_t wakelock     = DEFAULT_WAKELOCK;
static uint32_t sleepwakelock_timeout     = 0;
u32 system_can_yield = 1; /* default is can */
static uint32_t sleep_type = SLEEP_PG; /* 0 is power gate, 1 is clock gate */
static uint32_t max_sleep_time = 0; /* if user want wakeup peridically, can set this timer*/
KM4SLEEP_ParamDef sleep_param;

static uint32_t deepwakelock     = DEFAULT_DEEP_WAKELOCK;
static uint32_t deepwakelock_timeout     = 0;
static uint32_t sysactive_timeout_temp = 0;
static uint32_t sysactive_timeout_flag = 0;

u32 tickless_debug = 0;

unsigned char roaming_type_flag;			// 1-tickless roaming; 2-normal roaming
static u8 roaming_awake_flag = 0;			// tickless roaming enable or not flag
static u8 roaming_awake_bcnrssi_thrhd = 0;	// tickless awake threshhold
static u8 roaming_awake_delay = 0;			// tickless awake delay
u8 roaming_awake_rssi_range = 0;			// tickless awake |rssi| change range
u8 roaming_normal_rssi_range = 0;
rtos_sema_t roaming_sema = NULL;	// start roaming semaphore

/* psm dd hook info */
PSM_DD_HOOK_INFO gPsmDdHookInfo[PMU_MAX];

u32 pmu_exec_sleep_hook_funs(void)
{
	PSM_DD_HOOK_INFO *pPsmDdHookInfo = NULL;
	u32 nDeviceIdOffset = 0;
	u32 ret = TRUE;

	for (nDeviceIdOffset = 0; nDeviceIdOffset < PMU_MAX; nDeviceIdOffset++) {
		pPsmDdHookInfo = &gPsmDdHookInfo[nDeviceIdOffset];

		/*if this device register and sleep_hook_fun not NULL*/
		if (pPsmDdHookInfo && pPsmDdHookInfo->sleep_hook_fun) {
			ret = pPsmDdHookInfo->sleep_hook_fun(0, pPsmDdHookInfo->sleep_param_ptr);

			if (ret == FALSE) {
				break;
			}
		}
	}

	return nDeviceIdOffset;
}

void pmu_exec_wakeup_hook_funs(u32 nDeviceIdMax)
{
	PSM_DD_HOOK_INFO *pPsmDdHookInfo = NULL;
	u32 nDeviceIdOffset = 0;

	for (nDeviceIdOffset = 0; nDeviceIdOffset < nDeviceIdMax; nDeviceIdOffset++) {
		pPsmDdHookInfo = &gPsmDdHookInfo[nDeviceIdOffset];

		/*if this device register and sleep_hook_fun not NULL*/
		if (pPsmDdHookInfo && pPsmDdHookInfo->wakeup_hook_fun) {
			pPsmDdHookInfo->wakeup_hook_fun(0, pPsmDdHookInfo->wakeup_param_ptr);
		}
	}
}

#define SYSTICK_THRES 0x7fffffff
/*
return: TRUE: time1 > time2
*/
int freertos_systick_check(u32 time1, u32 time2)
{
	u32 delta = time1 > time2 ? time1 - time2 : time2 - time1;
	if (delta < SYSTICK_THRES) {
		return time1 >= time2 ? TRUE : FALSE;
	} else {	//overflow
		return time1 <= time2 ? TRUE : FALSE;
	}
}

uint32_t pmu_set_sysactive_time(uint32_t timeout)
{
	u32 TimeOut = 0;

	if (sysactive_timeout_flag) {
		if (timeout > sysactive_timeout_temp) {
			sysactive_timeout_temp = timeout;
		}
		return 1;
	}

	if (sysactive_timeout_temp > timeout) {
		timeout = sysactive_timeout_temp;
	}
	sysactive_timeout_temp = 0;

	TimeOut = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms()) + timeout;

	if (freertos_systick_check(TimeOut, sleepwakelock_timeout)) {
		sleepwakelock_timeout = TimeOut;
	}
	return 0;
}

void pmu_register_sleep_callback(u32 nDeviceId, PSM_HOOK_FUN sleep_hook_fun, void *sleep_param_ptr, PSM_HOOK_FUN wakeup_hook_fun, void *wakeup_param_ptr)
{
	PSM_DD_HOOK_INFO *pPsmDdHookInfo = NULL;

	assert_param(nDeviceId < PMU_MAX);
	assert_param((sleep_hook_fun != NULL) || (wakeup_hook_fun != NULL));

	pPsmDdHookInfo = &gPsmDdHookInfo[nDeviceId];
	pPsmDdHookInfo->nDeviceId			= nDeviceId;
	pPsmDdHookInfo->sleep_hook_fun		= sleep_hook_fun;
	pPsmDdHookInfo->sleep_param_ptr		= sleep_param_ptr;
	pPsmDdHookInfo->wakeup_hook_fun		= wakeup_hook_fun;
	pPsmDdHookInfo->wakeup_param_ptr	= wakeup_param_ptr;
}

void pmu_unregister_sleep_callback(u32 nDeviceId)
{
	PSM_DD_HOOK_INFO *pPsmDdHookInfo = NULL;

	assert_param(nDeviceId < PMU_MAX);

	pPsmDdHookInfo = &gPsmDdHookInfo[nDeviceId];

	_memset(pPsmDdHookInfo, 0x00, sizeof(PSM_DD_HOOK_INFO));
}


/* can not yield CPU under suspend/resume process */
uint32_t pmu_yield_os_check(void)
{
	return system_can_yield;
}

/*
 *  It is called in idle task.
 *
 *  @return  true  : System is ready to check conditions that if it can enter sleep.
 *           false : System keep awake.
 **/
CONFIG_FW_CRITICAL_CODE_SECTION
int freertos_ready_to_sleep(void)
{
	u32 current_tick = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms());

	/* timeout */
	if (freertos_systick_check(current_tick, sleepwakelock_timeout) == FALSE) {
		return FALSE;
	}

	if (wakelock == 0) {
#if defined (CONFIG_ARM_CORE_CM0)
		/* timeout */
		if (current_tick >= km4_sleep_timeout) {
			return FALSE;
		} else {
			if (km4_sleep_timeout != 0xffffffff) {
				SOCPS_AONTimer(km4_sleep_timeout - current_tick);
				SOCPS_SetWakeEventAON(BIT_AON_WAKE_TIM0_MSK, ENABLE);
				SOCPS_AONTimerCmd(ENABLE);
			}
		}
#endif
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 *  It is called in freertos pre_sleep_processing.
 *
 *  @return  true  : System is ready to check conditions that if it can enter dsleep.
 *           false : System can't enter deep sleep.
 **/
CONFIG_FW_CRITICAL_CODE_SECTION
int freertos_ready_to_dsleep(void)
{
	u32 current_tick =  RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms());

	/* timeout */
	if (current_tick < deepwakelock_timeout) {
		return FALSE;
	}

	if (deepwakelock == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*
 *  It is called when freertos is going to sleep.
 *  At this moment, all sleep conditons are satisfied. All freertos' sleep pre-processing are done.
 *
 *  @param  expected_idle_time : The time that FreeRTOS expect to sleep.
 *                               If we set this value to 0 then FreeRTOS will do nothing in its sleep function.
 **/
#if defined (CONFIG_ARM_CORE_CM4)
void freertos_pre_sleep_processing(unsigned int *expected_idle_time)
{
	UNUSED(expected_idle_time);
}

#else
CONFIG_FW_CRITICAL_CODE_SECTION
void freertos_pre_sleep_processing(unsigned int *expected_idle_time)
{
#ifdef CONFIG_SOC_PS_MODULE
	uint32_t tick_before_sleep;
	uint32_t tick_passed;
	uint32_t tick_after_sleep;
	volatile uint32_t ms_passed = 0;
	uint32_t ms_before_sleep = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_AON_TSF_CNT_LOW);
	uint32_t wakeup_time_ms = 0;

	*expected_idle_time = 0;

	/*  Store gtimer timestamp before sleep */
	tick_before_sleep = SYSTIMER_TickGet();
	sysactive_timeout_flag = 1;
	/*Enter dlps if dsleep is ready, or enter sleep mode*/
	//if (freertos_ready_to_dsleep()) {
	//	SOCPS_AONTimer(ps_config.km0_dslp_wake_interval);
	//	SOCPS_AONTimerCmd(ENABLE);

	//	SOCPS_DeepSleep_RAM(); /* The function will not return. */
	//}

	/* some function call like xTaskGetTickCount may cause IRQ ON, */
	/* so we close IRQ again here to avoid sys irq when enter or exit sleep */
	//__asm volatile( "cpsid i" );

	SOCPS_SWRLDO_Suspend(ENABLE);

	/* make sure the following function dont call system API that may open IRQ */

	SOCPS_SleepInit();

	/* for test */
	if ((BKUP_Read(0) & BIT_WIFI_ENABLE) == 0) {
		SOCPS_AONTimer(102);
		SOCPS_SetWakeEventAON(BIT_AON_WAKE_TIM0_MSK, ENABLE);
		SOCPS_AONTimerCmd(ENABLE);
	}

	if (sleep_type == SLEEP_PG) {
		SOCPS_SleepPG();
	} else {
		SOCPS_SleepCG();
	}

	/*  update kernel tick by calculating passed tick from gtimer */
	/*  get current gtimer timestamp */
	tick_after_sleep = SYSTIMER_TickGet();

	/*  calculated passed time */
	if (tick_after_sleep > tick_before_sleep) {
		tick_passed = tick_after_sleep - tick_before_sleep;
	} else {
		/*  overflow */
		tick_passed = (0xffffffff - tick_before_sleep) + tick_after_sleep;
	}

	tick_passed += missing_tick;
	missing_tick = tick_passed & 0x1F;

	/* timer clock is 32768, 0x20 is 1ms */
	ms_passed = ((tick_passed & 0xFFFF8000) / 32768) * 1000 + ((tick_passed & 0x7FFF) * 1000) / 32768;
	//ms_passed = (((tick_passed & 0xFFFFFFE0) * 1000)/32768);/* overflow when time over 0x418937, about 130s */
	vTaskStepTick(ms_passed); /*  update kernel tick */

	wakeup_time_ms = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_AON_TSF_CNT_LOW);

	if (ps_config.km0_tickles_debug) {
		RTK_LOGI(TAG, "m0 os sleeped:[%d->%d:%d] us\n", ms_before_sleep & 0x7FFFFFFF,
				 wakeup_time_ms & 0x7FFFFFFF, wakeup_time_ms - ms_before_sleep);
	}

	sysactive_timeout_flag = 0;

	sleepwakelock_timeout = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms()) + (sysactive_timeout_temp > 2 ? sysactive_timeout_temp : 2);
	sysactive_timeout_temp = 0;

	SOCPS_SWRLDO_Suspend(DISABLE);
	//__asm volatile( "cpsie i" );
#else
	/*  If PS is not enabled, then use freertos sleep function */
#endif
}
#endif

/* -------- FreeRTOS macro implementation -------- */
void pmu_acquire_wakelock(uint32_t nDeviceId)
{
	wakelock |= BIT(nDeviceId);
}

void pmu_release_wakelock(uint32_t nDeviceId)
{
	wakelock &= ~BIT(nDeviceId);
}

uint32_t pmu_get_wakelock_status(void)
{
	return wakelock;
}

uint32_t pmu_set_sleep_type(uint32_t type)
{
	sleep_type = type;
#if defined (CONFIG_ARM_CORE_CM0)
	if (ps_config.km0_pg_enable) {
		sleep_type = SLEEP_PG;
	}
#endif
	return 0;
}

uint32_t pmu_get_sleep_type(void)
{
	return sleep_type;
}

void pmu_set_max_sleep_time(uint32_t timer_ms)
{
	max_sleep_time = timer_ms;
}

uint32_t pmu_get_max_sleep_time(void)
{
	return max_sleep_time;
}

void pmu_set_dsleep_active_time(uint32_t TimeOutMs)
{
	u32 timeout = 0;


	timeout = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms()) + TimeOutMs;
	//DBG_8195A("pmu_set_dsleep_active_time: %d %d\n", timeout, deepwakelock_timeout);

	if (timeout > deepwakelock_timeout) {
		deepwakelock_timeout = timeout;
	}
}

void pmu_acquire_deepwakelock(uint32_t nDeviceId)
{
	deepwakelock |= BIT(nDeviceId);
}

void pmu_release_deepwakelock(uint32_t nDeviceId)
{
	deepwakelock &= ~BIT(nDeviceId);
}

uint32_t pmu_get_deepwakelock_status(void)
{
	return deepwakelock;
}


void pmu_tickless_debug(u32 NewStatus)
{
	if (NewStatus == ENABLE) {
		tickless_debug = 1;
	} else {
		tickless_debug = 0;
	}
}

void pmu_reset_awake(u8 type)
{
	if (type == 1) {
		roaming_awake_delay = 0;
		roaming_awake_rssi_range = 0;
	} else if (type == 2) {
		roaming_normal_rssi_range = 0;
	}
}

void pmu_degrade_awake(u8 type)
{
	if (type == 1) {
		if (!roaming_awake_delay) {
			roaming_awake_delay = 4;
		}
		roaming_awake_rssi_range = 3; // temp fix to 6db range

		if (++roaming_awake_delay > 7) {
			roaming_awake_delay = 7;    // max 7 second
		}
	} else if (type == 2) {
		if (!roaming_normal_rssi_range) {
			roaming_normal_rssi_range = 5;    // temp fix to 5db range
		} else if (roaming_normal_rssi_range < 8) {
			roaming_normal_rssi_range++;
		}
	}
}

void pmu_set_roaming_awake(u8 enable, u8 threshhold, u8 winsize)
{
	assert_param(enable == 1 || enable == 0);
	assert_param(threshhold > 0 && threshhold < 100);
	assert_param(winsize > 0 && winsize < 100);

	roaming_awake_flag = enable;
	if (1 == enable) {
		extern u8 bcnrssi_count_slide_winsize;
		roaming_awake_bcnrssi_thrhd = threshhold;
		bcnrssi_count_slide_winsize = winsize;
		if (roaming_sema == NULL) {
			rtos_sema_create_binary(&roaming_sema);
			rtos_sema_take(roaming_sema, 1);
		}
	} else if (0 == enable) {
		roaming_awake_bcnrssi_thrhd = 0;
		if (roaming_sema != NULL) {
			rtos_sema_delete(roaming_sema);
			roaming_sema = NULL;
		}
	}
	pmu_reset_awake(1);
	pmu_reset_awake(2);
}

u8 pmu_get_roaming_awake(u8 *p)
{
	if (roaming_awake_flag && p != NULL) {
		p[0] = roaming_awake_flag | (1 << 7);
		p[0] &= 0xf1;
		p[0] |= (roaming_awake_delay & 0x7) << 1;
		p[0] &= 0xcf;
		p[0] |= (roaming_awake_rssi_range & 0x3) << 4;
		p[1] = roaming_awake_bcnrssi_thrhd;
	}
	return roaming_awake_flag;
}

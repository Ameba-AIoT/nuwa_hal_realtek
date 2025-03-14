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
#include "os_wrapper_specific.h"

static const char *const TAG = "KM4SLEEP";
u32 KM4SleepTick = 0;

CPU_PWR_SEQ SYSPLL_ON_SEQ[] = {
	/* Enable SYS PLL */
	{0x48000208, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 28)},
	{0x48000280, CPU_PWRSEQ_CMD_WRITE,		0x00000000,			0x00000003},
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			4}, //delay 2us
	{0x48000280, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 2)},
	{0x480002B0, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01U << 31)},
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			4}, //delay 400us
	{0x48000264, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 29),			0x00000000},
	{0x480002B0, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x03 << 29)},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 21),			0x00000000},
	{0x480003F4, CPU_PWRSEQ_CMD_POLLING,	(0x01 << 12),	(0x01 << 12)},   /* temp use delay because FPGA dont have this function */
	//{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			100}, //delay 100us
	{0x480002B0, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 8)},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ SYSPLL_OFF_SEQ[] = {
	/* Disable SYS PLL */
	{0x480002B0, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 8),			0x00000000},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x001 << 21)},
	{0x48000264, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x001 << 29)},
	{0x480002B0, CPU_PWRSEQ_CMD_WRITE,	(0x007U << 29),			0x00000000},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ HSPWR_ON_SEQ[] = {
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x011 << 0),			0x00000000},  /* reset 21c[0]&[4]for WDG reset */
	{0x00000000, CPU_PWRSEQ_CMD_LOGE,		0x00000000,			0x00000000/* or 1 */}, /* Enable LOG or not */

	/* 1. Enable SWR */ //PMC
	//{0x48000240, CPU_PWRSEQ_CMD_WRITE,		0x00000000,			(0x01 << 0)},
	//{0x480003F4, CPU_PWRSEQ_CMD_POLLING,	(0x01 << 11),			(0x01 << 11)}, /* temp use delay because FPGA dont have this function */
	//{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			100}, //delay 100us
	//{0x48000240, CPU_PWRSEQ_CMD_WRITE,		0x00000000,			(0x01 << 2)},

	/* 2. Enable xtal/banggap */ //PMC
	//{0x48000260, CPU_PWRSEQ_CMD_WRITE,		0x00000000,			0x03},
	//{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			750}, //delay 750us

	/* 4. Enable HS power */
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 1)},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 2)},
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			200}, //delay 100us
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 3)},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,	(0x03 << 17),			0x00000000},

	/* 5. Enable clock and reset */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 0)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 4)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x1FF << 16),	(0x101 << 16)},    /* BIT24 is just for FPGA, ASIC dont need */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x07 << 1)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x07 << 25),			0x00000000},  //set KM4 200MHz

	/* 1. Recover KM4 Memory power, the memory low power config in sleep_hsram_config[] */
	{0x48000B08, CPU_PWRSEQ_CMD_WRITE,	(0xFFFFFFFF),			0x00000000},

	/* 6. Enable WIFI related */
	{0x48000314, CPU_PWRSEQ_CMD_WRITE,		0x00000000,			0x00000001}, // enable HW auto response

	/* 7. HS IPC Enable*/
	{0x40000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	    		BIT_HS_IPC_FEN}, //function enable
	{0x40000210, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	    		BIT_HS_IPC_CKE}, //clock enable

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ HSPWR_OFF_SEQ[] = {
	/* 1. HSSYSON clock switch to 4M */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 17)},

	/* 2. Delay 20us for clock switch down*/
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			20},

	/* 3. Gate CPU & Platform Clock */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 16),			0x00000000},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 24),			0x00000000},  /* BIT24 is just for FPGA, ASIC dont need */

	/* 4. Reset CPU & Platform */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x01F << 0),			0x00000000},

	/* 6. Disable HS Platform/HSSYSON power */
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x003 << 17)},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,	(0x007 << 1),			0x00000000},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ HSCLK_GATE_SEQ[] = {
	/* 1. HSSYSON clock switch to 4M */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 17)},

	/* 2. Delay 20us for clock switch down*/
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			20},

	/* 3. Gate CPU & Platform Clock */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 16),			0x00000000},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 24),			0x00000000},  /* BIT24 is just for FPGA, ASIC dont need */

	/* 4. Disable clock generator*/
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 4),			0x0000000F},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ HSCLK_ON_SEQ[] = {
	/* 1. Recover KM4 Memory power, the memory low power config in sleep_hsram_config[] */
	{0x48000B08, CPU_PWRSEQ_CMD_WRITE,	(0xFFFFFFFF),			0x00000000},

	/* 3. Enable clock and reset */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 4)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 24)},  /* BIT24 is just for FPGA, ASIC dont need */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 16)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 17),			0x00000000},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

/*It is different from HSPWR_OFF_SEQ[] for some peripheral to work in KM4 power gate mode*/
CPU_PWR_SEQ HSPWR_GATE_SEQ[] = {
	/* 1. Disable SDIO dev func en off */
	{0x40000208, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 1),			0x00000000},

	/* 2. HSSYSON clock switch to 4M */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 17)},

	/* 3. Delay 20us for clock switch down*/
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			20},

	/* 4. Gate CPU & Platform Clock */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 16),			0x00000000},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x001 << 24),			0x00000000},  /* BIT24 is just for FPGA, ASIC dont need */

	/* 5. Reset CPU & Platform*/
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x01F << 0),			0x00000001},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

CPU_PWR_SEQ HSPWR_WAKE_SEQ[] = {
	/* 1. Recover KM4 Memory power, the memory low power config in sleep_hsram_config[] */
	{0x48000B08, CPU_PWRSEQ_CMD_WRITE,	(0xFFFFFFFF),			0x00000000},

	/* 2. Enable HS power */
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 2)},
	{0x00000000, CPU_PWRSEQ_CMD_DELAY,		0x00000000,			200}, //delay 100us
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 3)},
	{0x48000200, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 18),			0x00000000},

	/* 3. Enable clock and reset */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 4)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 24)},  /* BIT24 is just for FPGA, ASIC dont need */
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 16)},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,	(0x01 << 17),			0x00000000},
	{0x4800021C, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x07 << 1)},

	/* 4. Enable SDIO dev func off */
	{0x40000208, CPU_PWRSEQ_CMD_WRITE,		0x00000000,	(0x01 << 1)},

	/* End */
	{0xFFFFFFFF, CPU_PWRSEQ_CMD_END,			0x00000000,			0x00000000},
};

u8 km4_sleep_type;
u32 km4_sleep_timeout = 0xffffffff;
u32 km4_wake_event;

u32 km4_status_on(void)
{
	u32 Temp = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LP_KM4_CTRL);

	if (Temp & BIT_LSYS_HPLAT_CKE) {
		return 1;
	} else {
		return 0;
	}
}

void km4_pm_init(void)
{

}

void km4_boot_on(void)
{
	u32 Temp;
	pmu_acquire_wakelock(PMU_KM4_RUN);

	/* Let KM4 RUN */
	Temp = HAL_READ32(SYSTEM_CTRL_BASE, REG_LP_BOOT_CFG) | BIT_SOC_BOOT_PATCH_KM4_RUN;
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_LP_BOOT_CFG, Temp);

	BOOT_ROM_CM4PON((u32)HSPWR_ON_SEQ);

	/*IPC table initialization*/
	ipc_table_init(IPCM4_DEV);
}

void km4_power_gate(void)
{
	u32 temp = 0;

	if (!km4_status_on()) {
		return;
	}

	/* poll KM4 clock gate */
	while (1) {
		temp = HAL_READ32(SYSTEM_CTRL_BASE_HP, REG_HS_PLATFORM_PARA);	/*get KM4 sleep status*/
		if (temp & BIT_KM4_SLEEP_STATUS) {
			break;
		}
	}

	RTK_LOGD(TAG, "HPPG\n");

	BOOT_ROM_CM4PON((u32)HSPWR_GATE_SEQ);

	pmu_release_wakelock(PMU_KM4_RUN);
	pmu_release_wakelock(PMU_OS);
}

void km4_power_wake(void)
{
	//u32 Temp = 0;

	if (km4_status_on()) {
		return;
	}

	RTK_LOGD(TAG, "HPPW\n");

	pmu_acquire_wakelock(PMU_KM4_RUN);

	BOOT_ROM_CM4PON((u32)HSPWR_WAKE_SEQ);
}

void km4_clock_gate(void)
{
	u32 temp = 0;

	if (!km4_status_on()) {
		return;
	}

	/* poll KM4 clock gate */
	while (1) {
		temp = HAL_READ32(SYSTEM_CTRL_BASE_HP, REG_HS_PLATFORM_PARA);	/*get KM4 sleep status*/
		if (temp & BIT_KM4_SLEEP_STATUS) {
			break;
		}
	}

	RTK_LOGD(TAG, "HPCG\n");

	BOOT_ROM_CM4PON((u32)HSCLK_GATE_SEQ);

	pmu_release_wakelock(PMU_KM4_RUN);
	pmu_release_wakelock(PMU_OS);
}

void km4_clock_on(void)
{
	if (km4_status_on()) {
		return;
	}

	pmu_acquire_wakelock(PMU_KM4_RUN);

	BOOT_ROM_CM4PON((u32)HSCLK_ON_SEQ);

	RTK_LOGD(TAG, "HPCW\n");

	/* tell KM4 wake */
	asm volatile("sev");
}

int km4_suspend(u32 type)
{
	int ret = SUCCESS;
	KM4SLEEP_ParamDef *sleep_param;
	u32 duration = 0;

	PIPC_MSG_STRUCT	ipc_msg_temp = (PIPC_MSG_STRUCT)ipc_get_message(NULL, IPC_INT_KM4_TICKLESS_INDICATION);
	sleep_param = (KM4SLEEP_ParamDef *)ipc_msg_temp->msg;
	if (sleep_param != NULL) {
		duration = sleep_param->sleep_time;
	}

	if (duration > 0) {
		/* used for resume delay */
		km4_sleep_timeout = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms()) + duration;
	}

	if (type == SLEEP_CG) {
		km4_clock_gate();
	} else {
		km4_power_gate();
	}

	return ret;
}

void km4_wake_event_update(void)
{
	/*the timer is used to control KM4 max sleep time*/
	u32 current_tick = RTOS_CONVERT_MS_TO_TICKS(rtos_time_get_current_system_time_ms());
	if (current_tick >= km4_sleep_timeout) {
		km4_wake_event |= BIT_HP_WEVT_TIMER_STS;
		km4_sleep_timeout = 0xffffffff;
	}

	/*when keyScan is used as a wake up source,*/
	if ((ps_config.km0_enable_key_touch  | BIT_KEY_ENABLE)
		&& KeyScan_GetINT(KEYSCAN_DEV)) {
		km4_wake_event |= BIT_HP_WEVT_KEYSCAN_STS;
	}

	/*when CAPTOUCH is used as a wake up source,*/
	if ((ps_config.km0_enable_key_touch  | BIT_CAPTOUCH_ENABLE)
		&& CapTouch_GetISR(CAPTOUCH_DEV)) {
		km4_wake_event |= BIT_HP_WEVT_CAPTOUCH_STS;
	}

	/*when GPIO is used as a wake up source,*/
	//if ((ps_config.km0_enable_key_touch  | BIT_GPIO_ENABLE)
	//	&& GPIO trigger wake KM4)
	//	km4_wake_event |=BIT_HP_WEVT_GPIO_STS;

}

void km4_set_wake_event(u32 wevt)
{
	km4_wake_event |= wevt;
}

u32 km4_get_wake_event(void)
{
	return km4_wake_event ;
}

void km4_resume(void)
{
	if (km4_status_on()) {
		goto exit;
	}

	pmu_acquire_wakelock(PMU_KM4_RUN);

	if (km4_sleep_type == SLEEP_CG) {
		km4_clock_on();
	} else {
		km4_power_wake();
	}

	/*set wake up status*/
	HAL_WRITE32(SYSTEM_CTRL_BASE_HP, REG_HS_WAKE_EVENT_STATUS1, km4_wake_event);

	SOCPS_AudioLDO(ENABLE);

exit:
	km4_wake_event = 0;
}

void km4_flash_highspeed_suspend(u32 Protection)
{
	FLASH_ClockSwitch(BIT_SHIFT_FLASH_CLK_XTAL, Protection);
	BOOT_ROM_CM4PON((u32)SYSPLL_OFF_SEQ);
}

void km4_flash_highspeed_init(void)
{
	BOOT_ROM_CM4PON((u32)SYSPLL_ON_SEQ);

	/* calibration high speed before KM4 run */
	flash_operation_config();
}

void km4_flash_highspeed_resume(u32 Protection)
{
	BOOT_ROM_CM4PON((u32)SYSPLL_ON_SEQ);
	FLASH_ClockSwitch(BIT_SHIFT_FLASH_CLK_PLL, Protection);
}

void km4_tickless_ipc_int(void *Data, u32 IrqStatus, u32 ChanNum)
{
	/* To avoid gcc warnings */
	(void) Data;
	(void) IrqStatus;
	(void) ChanNum;

	u32 Rtemp;
	KM4SLEEP_ParamDef *psleep_param;

	NVIC_ClearPendingIRQ(UART_LOG_IRQ_LP);
	/* Clear LOGUART Rx FIFO */
	while (LOGUART_Readable()) {
		LOGUART_GetChar(FALSE);
	}
	InterruptEn(UART_LOG_IRQ_LP, 10);
	IPCM0_DEV->IPCx_USR[IPC_INT_CHAN_SHELL_SWITCH] = 0x00000000;

	PIPC_MSG_STRUCT	ipc_msg_temp = (PIPC_MSG_STRUCT)ipc_get_message(NULL, IPC_INT_KM4_TICKLESS_INDICATION);
	psleep_param = (KM4SLEEP_ParamDef *)ipc_msg_temp->msg;
	DCache_Invalidate((u32)psleep_param, sizeof(KM4SLEEP_ParamDef));

	//set dlps
	if (psleep_param->dlps_enable) {
		SOCPS_AONTimerCmd(DISABLE);
		if (psleep_param->sleep_time) {
			SOCPS_AONTimer(psleep_param->sleep_time);
			SOCPS_AONTimerCmd(ENABLE);
		}
		FLASH_Write_Lock();
		SOCPS_DeepSleep_RAM();
		FLASH_Write_Unlock();
	}

	km4_sleep_type = psleep_param->sleep_type;

	switch (psleep_param->sleep_type) {
	case SLEEP_PG:
		if (SUCCESS == km4_suspend(SLEEP_PG)) {

			Rtemp = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LP_BOOT_CFG);
			Rtemp |= BIT_SOC_BOOT_WAKE_FROM_PS_HS;
			HAL_WRITE32(SYSTEM_CTRL_BASE_LP, REG_LP_BOOT_CFG, Rtemp);
		}
		break;
	case SLEEP_CG:
		if (SUCCESS == km4_suspend(SLEEP_CG)) {
			pmu_set_sysactive_time(2);
		}
		break;

	default:
		RTK_LOGE(TAG, "unknow sleep type\n");
	}

	KM4SleepTick = SYSTIMER_TickGet();
	//DBG_8195A("T:%d, tms:%d\r\n",KM4SleepTick,(((KM4SleepTick & 0xFFFF8000)/32768) * 1000 + ((KM4SleepTick & 0x7FFF) * 1000) /32768));
}

/**
  * @brief  Get km4 sleep time.
  * @param  null.
  * @retval km4 sleep time in ms.
  */
uint32_t pmu_get_km4sleeptime(void)
{
	u32 current_tick = 0, km4sleeptime = 0;

	current_tick = SYSTIMER_TickGet();

	if (current_tick >= KM4SleepTick) {
		km4sleeptime = current_tick - KM4SleepTick;
	} else {
		km4sleeptime = (0xFFFFFFFF - KM4SleepTick) + current_tick;
	}

	km4sleeptime = ((km4sleeptime & 0xFFFF8000) / 32768) * 1000 + ((km4sleeptime & 0x7FFF) * 1000) / 32768;

	return km4sleeptime;
}

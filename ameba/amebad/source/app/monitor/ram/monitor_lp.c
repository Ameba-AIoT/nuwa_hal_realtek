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

static const char *const TAG = "MONITOR";
u32
CmdRamHelp(
	IN  u16 argc,
	IN  u8  *argv[]
);

u32
cmd_reboot(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;

	WDG_InitTypeDef WDG_InitStruct;
	u32 CountProcess;
	u32 DivFacProcess;

	FLASH_ClockSwitch(BIT_SHIFT_FLASH_CLK_XTAL, TRUE);

	UART_RxCmd(UART2_DEV, DISABLE);
	RCC_PeriphClockSource_UART(UART2_DEV, UART_RX_CLK_XTAL_40M);
	UART_RxMonitorCmd(UART2_DEV, DISABLE);
	UART_SetBaud(UART2_DEV, 115200);
	UART_RxCmd(UART2_DEV, ENABLE);

	if (_strcmp((const char *)argv[0], "uartburn") == 0) {
		/* make KM4 sleep*/
		FLASH_Write_Lock();
		BKUP_Set(0, BKUP_BIT_UARTBURN_BOOT);
	}

	RTK_LOGS(TAG, RTK_LOG_ALWAYS, "Rebooting ...\n\r");

	WDG_Scalar(5, &CountProcess, &DivFacProcess);
	WDG_InitStruct.CountProcess = CountProcess;
	WDG_InitStruct.DivFacProcess = DivFacProcess;
	WDG_Init(&WDG_InitStruct);
	WDG_Cmd(ENABLE);

	return TRUE;
}

u32
CmdTickPS(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;

	if (_strcmp((const char *)argv[0], "r") == 0) { // release
		if (_strcmp((const char *)argv[1], "debug") == 0) {
			pmu_tickless_debug(ENABLE);
		} else {
			pmu_tickless_debug(DISABLE);
		}
		pmu_release_wakelock(PMU_OS);
	}

	if (_strcmp((const char *)argv[0], "a") == 0) { // acquire
		pmu_acquire_wakelock(PMU_OS);
	}

	if (_strcmp((const char *)argv[0], "type") == 0) { // PG or CG
		if (_strcmp((const char *)argv[1], "pg") == 0) {
			pmu_set_sleep_type(SLEEP_PG);
		} else if (_strcmp((const char *)argv[1], "cg") == 0) {
			pmu_set_sleep_type(SLEEP_CG);
		} else {
			pmu_set_sleep_type(SLEEP_PG);
		}
	}

	if (_strcmp((const char *)argv[0], "dslp") == 0) {
		u32 duration_ms = _strtoul((const char *)(argv[1]), (char **)NULL, 10);

		SOCPS_AONTimer(duration_ms);
		SOCPS_AONTimerCmd(ENABLE);

		SOCPS_DeepSleep_RAM();

	}

	if (_strcmp((const char *)argv[0], "get") == 0) { // get sleep & wake time
		RTK_LOGS(TAG, RTK_LOG_ALWAYS, "lockbit:%x \n", pmu_get_wakelock_status());
		RTK_LOGS(TAG, RTK_LOG_ALWAYS, "dslp_lockbit:%x\n", pmu_get_deepwakelock_status());
	}

	return TRUE;
}

u32
CmdRTC(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;

	RTC_TimeTypeDef RTC_TimeStruct;

	if (_strcmp((const char *)argv[0], "get") == 0) { // dump RTC
		RTC_AlarmTypeDef RTC_AlarmStruct_temp;


		RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
		RTC_GetAlarm(RTC_Format_BIN, &RTC_AlarmStruct_temp);

		RTK_LOGI(TAG, "time: %d:%d:%d:%d (%d) \n", RTC_TimeStruct.RTC_Days,
				 RTC_TimeStruct.RTC_Hours,
				 RTC_TimeStruct.RTC_Minutes,
				 RTC_TimeStruct.RTC_Seconds,
				 RTC_TimeStruct.RTC_H12_PMAM);

		RTK_LOGI(TAG, "alarm: %d:%d:%d:%d (%d) \n", RTC_AlarmStruct_temp.RTC_AlarmTime.RTC_Days,
				 RTC_AlarmStruct_temp.RTC_AlarmTime.RTC_Hours,
				 RTC_AlarmStruct_temp.RTC_AlarmTime.RTC_Minutes,
				 RTC_AlarmStruct_temp.RTC_AlarmTime.RTC_Seconds,
				 RTC_AlarmStruct_temp.RTC_AlarmTime.RTC_H12_PMAM);
	}

	if (_strcmp((const char *)argv[0], "set") == 0) {
		RTC_TimeStructInit(&RTC_TimeStruct);
		RTC_TimeStruct.RTC_Hours = _strtoul((const char *)(argv[1]), (char **)NULL, 10);
		RTC_TimeStruct.RTC_Minutes = _strtoul((const char *)(argv[2]), (char **)NULL, 10);
		RTC_TimeStruct.RTC_Seconds = _strtoul((const char *)(argv[3]), (char **)NULL, 10);

		if (_strcmp((const char *)argv[5], "pm") == 0) {
			RTC_TimeStruct.RTC_H12_PMAM = RTC_H12_PM;
		} else {
			RTC_TimeStruct.RTC_H12_PMAM = RTC_H12_AM;
		}

		RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
	}

	return TRUE;
}

/**
  * @brief  write p-efuse to patch a register when power on.
  * @param  page: 3bit, when you set 4800_03C4, page=0x03
  * @param  addr: 8bit, when you set 4800_03C4, page=0xC4
  * @param  val: 8bit, when you set 4800_03C4, 4800_03C4[7:0]=val
  * @retval  None
***/
void cmd_efuse_extpath_write(u8 page, u8 addr, u8 val)
{
	u8 efuse_hear_b0;
	u8 efuse_hear_b1;
	u32 start_addr;

	efuse_hear_b0 = 0xF; //fixed
	efuse_hear_b0 |= page << 5; //0x03 << 5=>4800_0300

	efuse_hear_b1 = 0x0E;	//1x16bit=>efuse_b0 & efuse_b1
	efuse_hear_b1 |= 0x0E << 4; //4800_0000

	/* write at the end of logical mapping address */
	/* so that hardware can aotoload it */
	start_addr = LOGICAL_MAP_SECTION_LEN - otp_logical_remain();

	RTK_LOGS(TAG, RTK_LOG_INFO, "app_efuse_extpath_write: 0x%x 0x%x\n", start_addr, otp_logical_remain());
	RTK_LOGS(TAG, RTK_LOG_INFO, "app_efuse_extpath_write: %x \n", efuse_hear_b0);
	RTK_LOGS(TAG, RTK_LOG_INFO, "app_efuse_extpath_write: %x \n", efuse_hear_b1);
	RTK_LOGS(TAG, RTK_LOG_INFO, "app_efuse_extpath_write: %x \n", addr);
	RTK_LOGS(TAG, RTK_LOG_INFO, "app_efuse_extpath_write: %x \n", val);

	EFUSE_PMAP_WRITE8(0, start_addr, efuse_hear_b0, L25EOUTVOLTAGE);
	EFUSE_PMAP_WRITE8(0, start_addr + 1, efuse_hear_b1, L25EOUTVOLTAGE);
	EFUSE_PMAP_WRITE8(0, start_addr + 2, addr, L25EOUTVOLTAGE); //0xC=>4800_03C4
	EFUSE_PMAP_WRITE8(0, start_addr + 3, val, L25EOUTVOLTAGE); //4800_03C4[8:0]=val
}

u32
CmdCTC(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;

	if (_strcmp((const char *)argv[0], "dump") == 0) { // dump RTC
		CapTouch_DbgDumpReg(CAPTOUCH_DEV);
	}

	if (_strcmp((const char *)argv[0], "etc") == 0) {
		u32 ch = 0;

		for (ch = 0; ch < 2; ch++) {
			CapTouch_DbgDumpETC(CAPTOUCH_DEV, ch);
		}
	}

	return TRUE;
}

u32
CmdHsSdm32K(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;
	(void) *argv;

	u32 temp;

	RTK_LOGI(TAG, "ENABLE HS SDM 32K\n");

	/*osc clk enable*/
	temp = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LP_XTAL_CTRL2);
	temp &= (~BIT1);
	HAL_WRITE32(SYSTEM_CTRL_BASE_LP, REG_LP_XTAL_CTRL2, temp);

	/*select xtal 40M*/
	temp = HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_SYS_EFUSE_SYSCFG2);
	temp &= (~(0xf << 16));
	HAL_WRITE32(SYSTEM_CTRL_BASE_LP, REG_SYS_EFUSE_SYSCFG2, temp);

	/*enable hs sdm*/
	temp = HAL_READ32(WIFI_REG_BASE, 0xe8);
	temp &= (~0x1f);
	HAL_WRITE32(WIFI_REG_BASE, 0xe8, temp);

	temp = HAL_READ32(WIFI_REG_BASE, 0xec);
	temp |= (0x3U << 30);
	HAL_WRITE32(WIFI_REG_BASE, 0xec, temp);

	/*select hs_sdm & 32k*/
	temp = HAL_READ32(WIFI_REG_BASE, 0x0);
	temp &= (~(0x3 << 20));
	temp |= (0x1 << 20);
	HAL_WRITE32(WIFI_REG_BASE, 0x0, temp);

	return TRUE;
}


CMD_TABLE_DATA_SECTION
const COMMAND_TABLE   shell_cmd_table[] = {
	{
		(const u8 *)"?",		0, CmdRamHelp,	(const u8 *)"\tHELP (?) : \n"
		"\t\t Print help messag\n"
	},
	{
		(const u8 *)"DW",		2, cmd_dump_word,	(const u8 *)"\tDW <Address, Hex>\n"
		"\t\t Dump memory dword or Read Hw dword register; \n"
		"\t\t Can Dump only one dword at the same time \n"
		"\t\t Unit: 4Bytes \n"
	},
	{
		(const u8 *)"EW",		2, cmd_write_word,	(const u8 *)"\tEW <Address, Hex>\n"
		"\t\t Write memory dword or Write Hw dword register \n"
		"\t\t Can write only one dword at the same time \n"
		"\t\t Ex: EW Address Value \n"
	},
	{
		(const u8 *)"FLASH",	8, cmd_flash,	(const u8 *)"\tFLASH \n"
		"\t\t erase chip \n"
		"\t\t erase sector addr \n"
		"\t\t erase block addr \n"
		"\t\t read addr len \n"
		"\t\t write addr data \n"
	},
	{
		(const u8 *)"REBOOT",	4, cmd_reboot,	(const u8 *)"\tREBOOT \n"
		"\t\t reboot \n"
		"\t\t reboot uartburn \n"
	},
	{
		(const u8 *)"TICKPS",	4, CmdTickPS,	(const u8 *)"\tTICKPS \n"
		"\t\t r: release os wakelock \n"
		"\t\t a: acquire os wakelock \n"
	},
	{
		(const u8 *)"RTC",		4, CmdRTC,	(const u8 *)"\tRTC \n"
		"\t\t get\n"
	},
	{
		(const u8 *)"CTC",		4, CmdCTC,	(const u8 *)"\tCTC \n"
		"\t\t dump/etc\n"
	},
	{
		(const u8 *)"HSSDM32K",	0, CmdHsSdm32K,	(const u8 *)"\tHSSDM32K \n"
		"\t\t enable hs sdm 32k\n"
	},
};

u32
CmdRamHelp(
	IN  u16 argc,
	IN  u8  *argv[]
)
{
	/* To avoid gcc warnings */
	(void) argc;
	(void) *argv;

	COMMAND_TABLE *cmd_table = (COMMAND_TABLE *)__cmd_table_start__;
	u32 cmd_mum = ((__cmd_table_end__ - __cmd_table_start__) / sizeof(COMMAND_TABLE));
	u32	index ;

	RTK_LOGS(NOTAG, RTK_LOG_ALWAYS, "----------------- TEST COMMAND MODE HELP %d------------------\n", cmd_mum);
	for (index = 0  ; index < cmd_mum; index++) {
		if (cmd_table[index].msg) {
			RTK_LOGS(NOTAG, RTK_LOG_ALWAYS, "%s\n", cmd_table[index].msg);
		}
	}
	RTK_LOGS(NOTAG, RTK_LOG_ALWAYS, "----------------- TEST COMMAND MODE END  %x------------------\n", cmd_mum);

	return TRUE ;
}


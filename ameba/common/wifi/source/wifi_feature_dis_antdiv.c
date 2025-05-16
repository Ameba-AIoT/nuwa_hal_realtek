/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define __WIFI_FEATURE_DIS_ANTDIV_C__
#include <stdbool.h>
#include <stdint.h>

#define UNUSED(a)		(void)a

#if defined(CONFIG_SINGLE_CORE_WIFI)

void wifi_hal_antdiv_training_timer_init(void *a)
{
	UNUSED(a);
}

void wifi_hal_antdiv_init(void *a)
{
	UNUSED(a);
}

void wifi_hal_antdiv_watchdog(void *a)
{
	UNUSED(a);
}

void wifi_hal_antdiv_get_rssi_info(bool a, uint8_t b, uint8_t c)
{
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
}

void wifi_hal_antdiv_get_rate_info(bool a, uint8_t b, uint8_t c)
{
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
}

void wifi_hal_antdiv_get_evm_info(bool a, uint8_t b)
{
	UNUSED(a);
	UNUSED(b);
}

void rtw_wnm_dbg_cmd(uint8_t *buf, int32_t buf_len, int32_t flags, void *userdata)
{
	UNUSED(buf);
	UNUSED(buf_len);
	UNUSED(flags);
	UNUSED(userdata);
}

#else
// uint32_t pmu_set_sysactive_time(uint32_t timeout)
// {
// 	(void)timeout;
// 	return 0;
// }

void rtw_wnm_dbg_cmd(uint8_t *buf, int32_t buf_len, int32_t flags, void *userdata)
{
	UNUSED(buf);
	UNUSED(buf_len);
	UNUSED(flags);
	UNUSED(userdata);
}

#endif

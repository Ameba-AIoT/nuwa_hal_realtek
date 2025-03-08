/**
  ******************************************************************************
  * @file    ameba_intfcfg.c
  * @author
  * @version V1.0.0
  * @date    2016-05-17
  * @brief   This file provides firmware functions to manage the following
  *          functionalities:
  *           - uart mbed function config
  ******************************************************************************
  * @attention
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
  ******************************************************************************
  */

#include "ameba_soc.h"

SDIOHCFG_TypeDef sdioh_config = {
	.sdioh_bus_speed = SD_SPEED_HS,				//SD_SPEED_DS or SD_SPEED_HS
	.sdioh_bus_width = SDIOH_BUS_WIDTH_4BIT, 	//SDIOH_BUS_WIDTH_1BIT or SDIOH_BUS_WIDTH_4BIT
	.sdioh_cd_pin = _PB_25,						//_PB_25/_PA_6/_PNC
	.sdioh_wp_pin = _PNC,						//_PB_24/_PA_5/_PNC
};

#if defined(CONFIG_FTL_ENABLED)
#define FTL_MEM_CUSTEM		1
#if FTL_MEM_CUSTEM == 0
#error "You should allocate flash sectors to for FTL physical map as following, then set FTL_MEM_CUSTEM to 1. For more information, Please refer to Application Note, FTL chapter. "
#else
const u8 ftl_phy_page_num = 3;									/* The number of physical map pages, default is 3*/
const u32 ftl_phy_page_start_addr = FTL_PHY_PAGE_START_ADDR;					/* The start offset of flash pages which is allocated to FTL physical map.
																	Users should modify it according to their own memory layout!! */
#endif
#endif

UARTCFG_TypeDef uart_config[4] = {
	/* HS UART--> UART0 */
	{
		.LOW_POWER_RX_ENABLE = DISABLE, /*Enable low power RX*/
	},
	/* BT UART--> UART1 */
	{
		.LOW_POWER_RX_ENABLE = DISABLE,
	},
	/* Log UART--> UART2 */
	{
		.LOW_POWER_RX_ENABLE = DISABLE,
	},
	/* LPUART-->UART3 */
	{
		.LOW_POWER_RX_ENABLE = DISABLE,
	},
};

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor *****END OF FILE****/

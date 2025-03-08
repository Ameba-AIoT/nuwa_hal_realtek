/**
  ******************************************************************************
  * @file    ameba_bootcfg.c
  * @author
  * @version V1.0.0
  * @date    2018-09-12
  * @brief   This file provides firmware functions to manage the following
  *          functionalities:
  *           - memory layout config
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

/*
* @brif	MMU Configuration.
*	There are 8 MMU entries totally. Entry 0 & Entry 1 are already used by OTA, Entry 2~7 can be used by Users.
*/
MMU_ConfDef Flash_MMU_Config[] = {
	/*VAddrStart,	VAddrEnd,			PAddrStart,		PAddrEnd*/
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 2
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 3
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 4
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 5
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 6
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},	//Entry 7
	{0xFFFFFFFF,	0xFFFFFFFF,			0xFFFFFFFF,		0xFFFFFFFF},
};

/*
* @brif	RSIP Mask Configuration.
*	There are 4 RSIP mask entries totally. Entry 0 is already used by System Data, Entry 3 is reserved by Realtek.
*	Only Entry 1 & Entry 2 can be used by Users.
*	MaskAddr: start address for RSIP Mask, should be 4KB aligned
*	MaskSize: size of the mask area, unit is 4KB, MaxSize 255*4KB
*/
RSIP_MaskDef RSIP_Mask_Config[] = {
	/*MaskAddr,		MaskSize*/
	{0x08002000,	2},		//Entry 0: 4K system data & 4K backup

	/* customer can set here */
	{0xFFFFFFFF,	0xFF},	//Entry 1: can be used by users
	{0xFFFFFFFF,	0xFF},	//Entry 2: can be used by users
	{0xFFFFFFFF, 	0xFF},	//Entry 3: Reserved by Realtek. If RDP is not used, this entry can be used by users.

	/* End */
	{0xFFFFFFFF, 	0xFF},
};

/**
* @brif  boot log enable or disable.
* 	FALSE: disable
*	TRUE: enable
*/
u8 Boot_Log_En = FALSE;

/**
* @brif  Firmware verify callback handler.
*	If users need to verify checksum/hash for image2, implement the function and assign the address
*	to this function pointer.
* @param  None
* @retval 1: Firmware verify successfully, 0: verify failed
*/
FuncPtr FwCheckCallback = NULL;

/**
* @brif  Firmware select hook.
*	If users need to implement own OTA select method, implement the function and assign the address
*	to this function pointer.
* @param  None
* @retval 0: both firmwares are invalid, select none, 1: boot from OTA1, 2: boot from OTA2
*/
FuncPtr OTASelectHook = NULL;



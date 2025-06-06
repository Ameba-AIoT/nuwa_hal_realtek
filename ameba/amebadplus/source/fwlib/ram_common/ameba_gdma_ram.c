/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ameba_soc.h"

static const char *const TAG = "GDMA";
static u8 GDMA_IrqNum[8] = {
	GDMA0_CHANNEL0_IRQ,
	GDMA0_CHANNEL1_IRQ,
	GDMA0_CHANNEL2_IRQ,
	GDMA0_CHANNEL3_IRQ,
	GDMA0_CHANNEL4_IRQ,
	GDMA0_CHANNEL5_IRQ,
	GDMA0_CHANNEL6_IRQ,
	GDMA0_CHANNEL7_IRQ,
};

/** @addtogroup Ameba_Periph_Driver
  * @{
  */

/** @defgroup GDMA
  * @brief	GDMA driver modules
  * @{
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup GDMA_Exported_Functions GDMA Exported Functions
  * @{
  */

/**
  * @brief  Fills each GDMA_InitStruct member with its default value.
  * @param  GDMA_InitStruct: pointer to a GDMA_InitTypeDef structure which will be
  *         initialized.
  * @retval None
  */
__weak
void GDMA_StructInit(PGDMA_InitTypeDef GDMA_InitStruct)
{
	_memset((void *)GDMA_InitStruct, 0, sizeof(GDMA_InitTypeDef));

	GDMA_InitStruct->GDMA_SrcDataWidth = TrWidthFourBytes;
	GDMA_InitStruct->GDMA_DstDataWidth = TrWidthFourBytes;
	GDMA_InitStruct->GDMA_SrcMsize = MsizeEight;
	GDMA_InitStruct->GDMA_DstMsize = MsizeEight;

	GDMA_InitStruct->SecureTransfer = 0;
}

/**
  * @brief  Initializes the GDMA registers according to the specified parameters
  *         in GDMA_InitStruct.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  GDMA_InitStruct: pointer to a GDMA_InitTypeDef structure that contains
  *         the configuration information for the GDMA peripheral.
  * @retval   None
  */
__weak
void GDMA_Init(u8 GDMA_Index, u8 GDMA_ChNum, PGDMA_InitTypeDef GDMA_InitStruct)
{
	u32 CtlxLow = 0;
	u32 CtlxUp = 0;
	u32 CfgxLow = 0;
	u32 CfgxUp = 0;
	u32 BlockSize = GDMA_InitStruct->GDMA_BlockSize;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	/* first open secure transfer, then set other registers */
	if (GDMA_InitStruct->SecureTransfer) {
		GDMA->CH[GDMA_ChNum].CFG_HIGH &= ~BIT_CFGX_UP_SEC_DISABLE;
	} else {
		GDMA->CH[GDMA_ChNum].CFG_HIGH |= BIT_CFGX_UP_SEC_DISABLE;
	}

	/* Enable GDMA in DmaCfgReg*/
	//GDMA->DmaCfgReg = 1;

	/* Check chanel is avaliable */
	if (GDMA->ChEnReg & BIT(GDMA_ChNum)) {
		/* Disable Channel */
		RTK_LOGW(TAG, "Channel had used; Disable Channel!!!!\n");

		GDMA_Cmd(GDMA_Index, GDMA_ChNum, DISABLE);

	}

	/* Check if there are the pending isr; TFR, Block, Src Tran, Dst Tran, Error */
	GDMA_ClearINT(GDMA_Index, GDMA_ChNum);

	/* Fill in SARx register */
	GDMA->CH[GDMA_ChNum].SAR = GDMA_InitStruct->GDMA_SrcAddr;
	/* Fill in DARx register */
	GDMA->CH[GDMA_ChNum].DAR = GDMA_InitStruct->GDMA_DstAddr;

	/* Process CTLx */
	CtlxLow = BIT_CTLX_LO_INT_EN |
			  (GDMA_InitStruct->GDMA_DstDataWidth << 1) |
			  (GDMA_InitStruct->GDMA_SrcDataWidth << 4) |
			  (GDMA_InitStruct->GDMA_DstInc << 7) |
			  (GDMA_InitStruct->GDMA_SrcInc << 9) |
			  (GDMA_InitStruct->GDMA_DstMsize << 11) |
			  (GDMA_InitStruct->GDMA_SrcMsize << 14) |
			  (GDMA_InitStruct->GDMA_DIR << 20) |
			  (GDMA_InitStruct->GDMA_LlpDstEn << 27) |
			  (GDMA_InitStruct->GDMA_LlpSrcEn << 28);


	CtlxUp = (BlockSize & BIT_CTLX_UP_BLOCK_BS) /*|
		BIT_CTLX_UP_DONE*/;


	/* Fill in CTLx register */
	GDMA->CH[GDMA_ChNum].CTL_LOW = CtlxLow;
	GDMA->CH[GDMA_ChNum].CTL_HIGH = CtlxUp;

	/* Program CFGx */
	CfgxLow = (GDMA_InitStruct->GDMA_ReloadSrc << 30) |
			  (GDMA_InitStruct->GDMA_ReloadDst << 31);

	CfgxUp = GDMA->CH[GDMA_ChNum].CFG_HIGH;
	CfgxUp &= ~(BIT_CFGX_UP_SRC_PER | BIT_CFGX_UP_DEST_PER);
	if (GDMA_InitStruct->GDMA_SrcHandshakeInterface >= 16) {
		GDMA_InitStruct->GDMA_SrcHandshakeInterface = (GDMA_InitStruct->GDMA_SrcHandshakeInterface & 0xf);
		CfgxUp |= (GDMA_InitStruct->GDMA_SrcHandshakeInterface << 7) | GDMA_BIT_ExtendedSRC_PER1;
	} else {
		CfgxUp |= (GDMA_InitStruct->GDMA_SrcHandshakeInterface << 7);
	}

	if (GDMA_InitStruct->GDMA_DstHandshakeInterface >= 16) {
		GDMA_InitStruct->GDMA_DstHandshakeInterface = (GDMA_InitStruct->GDMA_DstHandshakeInterface & 0xf);
		CfgxUp |= (GDMA_InitStruct->GDMA_DstHandshakeInterface << 11) | GDMA_BIT_ExtendedDEST_PER1;
	} else {
		CfgxUp |= (GDMA_InitStruct->GDMA_DstHandshakeInterface << 11);
	}

	GDMA->CH[GDMA_ChNum].CFG_LOW = CfgxLow;
	GDMA->CH[GDMA_ChNum].CFG_HIGH = CfgxUp;

	GDMA_INTConfig(GDMA_Index, GDMA_ChNum, GDMA_InitStruct->GDMA_IsrType, ENABLE);
}

/**
  * @brief  Set LLP mode when use GDMA LLP (AmebaZ not support this mode).
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  MultiBlockCount: block counts.
  * @param  pGdmaChLli: LLP node list, point to a Linked List Item.
  * @param  round: 0: Linear list, last node id last one 1:last node is not the end.
  * @retval   None
  */
__weak
void GDMA_SetLLP(u8 GDMA_Index, u8 GDMA_ChNum, u32 MultiBlockCount, struct GDMA_CH_LLI *pGdmaChLli, u32 round)
{
	u32 CtlxLow, CtlxUp;
	PGDMA_CH_LLI_ELE pLliEle = &pGdmaChLli->LliEle;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	//DBG_GDMA_INFO("Block Count %lu\n", MultiBlockCount);

	CtlxLow = GDMA->CH[GDMA_ChNum].CTL_LOW;
	CtlxUp = GDMA->CH[GDMA_ChNum].CTL_HIGH;

	CtlxUp = (pGdmaChLli->BlockSize & BIT_CTLX_UP_BLOCK_BS) | /*BIT_CTLX_UP_DONE | */CtlxUp;
	GDMA->CH[GDMA_ChNum].CTL_HIGH = CtlxUp;

	/* LLP register */
	GDMA->CH[GDMA_ChNum].LLP = (u32) & (pGdmaChLli->LliEle);

	while (MultiBlockCount > 0) {
		pLliEle = &pGdmaChLli->LliEle;

		/* Clear the last element llp enable bit */
		if ((1 == MultiBlockCount) & (~round)) {
			CtlxLow &= ~(BIT_CTLX_LO_LLP_DST_EN |
						 BIT_CTLX_LO_LLP_SRC_EN);
		}
		/* Update block size for transfer */
		CtlxUp &= ~(BIT_CTLX_UP_BLOCK_BS);
		CtlxUp |= (pGdmaChLli->BlockSize & BIT_CTLX_UP_BLOCK_BS);

		/* Updatethe Llpx context */
		pLliEle->CtlxLow = CtlxLow;
		pLliEle->CtlxUp = CtlxUp;
		pLliEle->Llpx = (u32)&pGdmaChLli->pNextLli->LliEle;

		DCache_CleanInvalidate((u32)pGdmaChLli, sizeof(struct GDMA_CH_LLI));

		/* Update the Lli and Block size list point to next llp */
		pGdmaChLli = pGdmaChLli->pNextLli;
		MultiBlockCount--;
	}
}

/**
  * @brief  Clear the specidied Pending Interrupt status.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  GDMA_IT: specifies the UARTx interrupt sources whose interrupt status will be cleared.
  *          This parameter can be one or combinations of the following values:
  *            @arg TransferType
  *            @arg BlockType
  *            @arg ErrType
  *            @arg or the combination of the interrupt types above
  * @note  this function completes clearing the specified type interrupt status. Which type
  *            interrupt status will be cleared, refer to the parameter "GDMA_IT".
  * @retval   active interrupt types whose interrupt status is cleared.
  *               this return value refers to @ref DMA_interrupts_definition
  */
__weak  u32
GDMA_ClearINTPendingBit(u8 GDMA_Index, u8 GDMA_ChNum, u32 GDMA_IT)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	uint32_t temp_bit = 0;
	uint32_t temp_isr = 0;

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));
	assert_param(IS_GDMA_CONFIG_IT(GDMA_IT));

	/* get isr */
	if ((GDMA->RAW_TFR & BIT(GDMA_ChNum)) && (GDMA->STATUS_TFR & BIT(GDMA_ChNum))) {
		temp_isr |= (TransferType & GDMA_IT);
	}
	if ((GDMA->RAW_BLOCK & BIT(GDMA_ChNum)) && (GDMA->STATUS_BLOCK & BIT(GDMA_ChNum))) {
		temp_isr |= (BlockType & GDMA_IT);
	}
	if ((GDMA->RAW_ERR & BIT(GDMA_ChNum)) && (GDMA->STATUS_ERR & BIT(GDMA_ChNum))) {
		temp_isr |= (ErrType & GDMA_IT);
	}

	/* clear the selected DMA interrupts */
	temp_bit = BIT(GDMA_ChNum) | BIT(GDMA_ChNum + 8);

	if (GDMA_IT & TransferType) {
		GDMA->CLEAR_TFR = temp_bit;
	}

	if (GDMA_IT & BlockType) {
		GDMA->CLEAR_BLOCK = temp_bit;
	}

	if (GDMA_IT & ErrType) {
		GDMA->CLEAR_ERR = temp_bit;
	}

	return temp_bit;
}

/**
  * @brief  Clear all the Pending Interrupt status.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @note    clear all the active interrupts status for the current GDMA channel.
  * @retval   active interrupt types whose interrupt status is cleared.
  *               this return value refers to @ref DMA_interrupts_definition
  */
__weak  u32
GDMA_ClearINT(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	uint32_t temp_bit = 0;
	uint32_t temp_isr = 0;

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	/* get isr */
	if ((GDMA->RAW_TFR & BIT(GDMA_ChNum)) && (GDMA->STATUS_TFR & BIT(GDMA_ChNum))) {
		temp_isr |= TransferType;
	}
	if ((GDMA->RAW_BLOCK & BIT(GDMA_ChNum)) && (GDMA->STATUS_BLOCK & BIT(GDMA_ChNum))) {
		temp_isr |= BlockType;
	}
	if ((GDMA->RAW_ERR & BIT(GDMA_ChNum)) && (GDMA->STATUS_ERR & BIT(GDMA_ChNum))) {
		temp_isr |= ErrType;
	}

	/* clear the selected DMA interrupts */
	temp_bit = BIT(GDMA_ChNum) | BIT(GDMA_ChNum + 8);

	/* clear all types of interrupt */
	GDMA->CLEAR_TFR = temp_bit;
	GDMA->CLEAR_BLOCK = temp_bit;
	GDMA->CLEAR_ERR = temp_bit;

	return temp_isr;
}

/**
  * @brief  Enables or disables the specified GDMA interrupts.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  GDMA_IT: specifies the GDMA interrupt sources to be enabled or disabled.
  *          This parameter can be one or combinations of the following values:
  *            @arg TransferType
  *            @arg BlockType
  *            @arg ErrType
  *            @arg or the combination of the interrupt types above
  * @param  NewState: DISABLE/ENABLE.
  * @retval   None
  */
__weak  void
GDMA_INTConfig(u8 GDMA_Index, u8 GDMA_ChNum, u32 GDMA_IT, u32 NewState)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	u32 temp_bit = 0;

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));
	assert_param(IS_GDMA_CONFIG_IT(GDMA_IT));

	if (NewState != DISABLE) {
		/* Enable the selected DMA Chan interrupts */
		temp_bit = BIT(GDMA_ChNum) | BIT(GDMA_ChNum + 8);

		if (GDMA_IT & TransferType) {
			GDMA->MASK_TFR = temp_bit;
		}

		if (GDMA_IT & BlockType) {
			GDMA->MASK_BLOCK = temp_bit;
		}

		if (GDMA_IT & ErrType) {
			GDMA->MASK_ERR = temp_bit;
		}
	} else {
		/* Disable the selected DMA Chan interrupts */
		temp_bit = BIT(GDMA_ChNum + 8);

		if (GDMA_IT & TransferType) {
			GDMA->MASK_TFR = temp_bit;
		}

		if (GDMA_IT & BlockType) {
			GDMA->MASK_BLOCK = temp_bit;
		}

		if (GDMA_IT & ErrType) {
			GDMA->MASK_ERR = temp_bit;
		}
	}
}

/**
  * @brief  GDMA channel Disable/Enable.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  NewState: Disable/Enable.
  * @retval   None
  */
__weak  void
GDMA_Cmd(u8 GDMA_Index, u8 GDMA_ChNum, u32 NewState)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	u32 ValTemp = 0;

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	/* ChEnWE is used to tell HW just set the needed ChEn bit */
	/* This allows software to set a mask bit without */
	/* performing a read-modified write operation. */
	ValTemp |= BIT(GDMA_ChNum + 8);

	if (NewState == ENABLE) {
		ValTemp |= BIT(GDMA_ChNum);
	} else {
		ValTemp &= ~ BIT(GDMA_ChNum);
	}

	GDMA->ChEnReg = ValTemp;
}

/**
  * @brief  GDMA AutoReload set.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  CleanType: This parameter can be any combination of the following values:
  *		 @arg CLEAN_RELOAD_SRC
  *		 @arg CLEAN_RELOAD_DST
  * @retval   None
  */
__weak  void
GDMA_ChCleanAutoReload(u8 GDMA_Index, u8 GDMA_ChNum, u32 CleanType)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	u32 CfgxLow;

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	CfgxLow = GDMA->CH[GDMA_ChNum].CFG_LOW;

	if (CleanType == CLEAN_RELOAD_SRC) {
		CfgxLow &= ~BIT_CFGX_L_RELOAD_SRC;
	} else if (CleanType == CLEAN_RELOAD_DST) {
		CfgxLow &= ~BIT_CFGx_L_RELOAD_DST;
	} else {
		CfgxLow &= ~BIT_CFGX_L_RELOAD_SRC;
		CfgxLow &= ~BIT_CFGx_L_RELOAD_DST;
	}

	GDMA->CH[GDMA_ChNum].CFG_LOW = CfgxLow;
}

/**
  * @brief  Set source address.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  SrcAddr: source address
  * @retval   None
  */
__weak
void GDMA_SetSrcAddr(u8 GDMA_Index, u8 GDMA_ChNum, u32 SrcAddr)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	GDMA->CH[GDMA_ChNum].SAR = SrcAddr;
}

/**
  * @brief  Get the source reading address during DMA transmission.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval value: source address
  */
__weak
u32 GDMA_GetSrcAddr(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	return GDMA->CH[GDMA_ChNum].RSAR;
}

/**
  * @brief  Get the destination writing address during DMA transmission.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval value: destination address
  */
__weak
u32 GDMA_GetDstAddr(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	return GDMA->CH[GDMA_ChNum].RDAR;
}

/**
  * @brief  Set destination address.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  DstAddr: destination address
  * @retval   None
  */
__weak
void GDMA_SetDstAddr(u8 GDMA_Index, u8 GDMA_ChNum, u32 DstAddr)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	GDMA->CH[GDMA_ChNum].DAR = DstAddr;
}

/**
  * @brief  Set GDMA block size
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @param  BlkSize:
  * @retval   None
  */
__weak
void GDMA_SetBlkSize(u8 GDMA_Index, u8 GDMA_ChNum, u32 BlkSize)
{
	u32 BlkSizeTemp = 0;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	BlkSizeTemp = GDMA->CH[GDMA_ChNum].CTL_HIGH;

	BlkSizeTemp &= ~BIT_CTLX_UP_BLOCK_BS;
	BlkSizeTemp |= (BlkSize & BIT_CTLX_UP_BLOCK_BS);

	GDMA->CH[GDMA_ChNum].CTL_HIGH = BlkSizeTemp;
}

/**
  * @brief  Gets the total number of data bytes written to the target memory or peripheral device during DMA transmission.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval   block size configured for the current GDMA channel
  */
__weak
u32 GDMA_GetBlkSize(u8 GDMA_Index, u8 GDMA_ChNum)
{
	u32 BlkSize = 0;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	BlkSize = GDMA->CH[GDMA_ChNum].CTL_HIGH;

	BlkSize &= BIT_CTLX_UP_BLOCK_BS;

	return BlkSize;
}

/**
  * @brief  Register channel if this channel is used.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval None
  */
static void GDMA_ChnlRegister(u8 GDMA_Index, u8 GDMA_ChNum, IRQ_FUN IrqFun, u32 IrqData, u32 IrqPriority)
{
	u8 IrqNum = 0;
	u8 ValTemp = HAL_READ8(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_REASON_SW + 3);

	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));
	/* register idle channel. */

	ValTemp |= BIT(GDMA_ChNum);
	HAL_WRITE8(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_REASON_SW + 3, ValTemp);

	if (IrqFun != NULL) {
		IrqNum = GDMA_IrqNum[GDMA_ChNum];
		InterruptRegister(IrqFun, IrqNum, IrqData, IrqPriority);
		InterruptEn(IrqNum, IrqPriority);
	}
}

/**
  * @brief  Unregister channel if this channel is not used.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval   None
  */
static void GDMA_ChnlUnRegister(u8 GDMA_Index, u8 GDMA_ChNum)
{
	u8 IrqNum = 0;
	u8 ValTemp = HAL_READ8(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_REASON_SW + 3);

	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	ValTemp &= ~BIT(GDMA_ChNum);
	HAL_WRITE8(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_REASON_SW + 3, ValTemp);

	IrqNum = GDMA_IrqNum[GDMA_ChNum];
	InterruptDis(IrqNum);
	InterruptUnRegister(IrqNum);
}

/**
  * @brief  Alloc a free channel.
  * @param  GDMA_Index: 0 .
  * @param  IrqFun: GDMA IRQ callback function.
  * @param  IrqData: GDMA IRQ callback data.
  * @param  IrqPriority: GDMA IrqPriority.
  * @retval value: GDMA_ChNum
  */
__weak  u8
GDMA_ChnlAlloc(u32 GDMA_Index, IRQ_FUN IrqFun, u32 IrqData, u32 IrqPriority)
{
	u32 found = 0;
	u32 GDMA_ChNum = 0xFF;
	u8 ValTemp = 0;

	assert_param(IS_GDMA_Index(GDMA_Index));

	if (IPC_SEMTake(GDMA_SEM_IDX, 1000) == FALSE) {
		return GDMA_ChNum;
	}

	/* Match idle channel. */
	ValTemp = HAL_READ8(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_REASON_SW + 3);

	for (GDMA_ChNum = 0; GDMA_ChNum <= MAX_GDMA_CHNL; GDMA_ChNum++) {
		if ((ValTemp & BIT(GDMA_ChNum)) == 0) {
			found = 1;
			break;
		}
	}

	/*If idle channel is found,  register it. */
	if (found) {
		GDMA_ChnlRegister(GDMA_Index, GDMA_ChNum, IrqFun, IrqData, IrqPriority);
	} else {
		GDMA_ChNum = 0xFF;
	}

	IPC_SEMFree(GDMA_SEM_IDX);
	return GDMA_ChNum;
}

/**
  * @brief  Free a channel, this channel will not be used.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval   TRUE/_FLASE
  */
__weak  u8
GDMA_ChnlFree(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = NULL;
	u8 ret  = FALSE;
	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	if (IPC_SEMTake(GDMA_SEM_IDX, 1000) == FALSE) {
		return ret;
	}

	if (TrustZone_IsSecure()) {
		/* disable secure, or non-secure can not use this channel */
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
		GDMA->CH[GDMA_ChNum].CFG_HIGH |= BIT_CFGX_UP_SEC_DISABLE;
	}

	GDMA_ChnlUnRegister(GDMA_Index, GDMA_ChNum);

	IPC_SEMFree(GDMA_SEM_IDX);

	ret = TRUE;
	return ret;
}

/**
  * @brief  Get irq number for a channel.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval   IrqNum
  */
__weak  u8
GDMA_GetIrqNum(u8 GDMA_Index, u8 GDMA_ChNum)
{
	u8 IrqNum;

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));

	IrqNum = GDMA_IrqNum[GDMA_ChNum];

	return IrqNum;
}

/**
  * @brief  Set channel priority.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval   IrqNum
  */
__weak  void
GDMA_SetChnlPriority(u8 GDMA_Index, u8 GDMA_ChNum, u32 ChnlPriority)
{
	u32 CfgxLow;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);

	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}

	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	CfgxLow = GDMA->CH[GDMA_ChNum].CFG_LOW;

	CfgxLow &= ~BIT_CFGX_CH_PRIOR;
	CfgxLow |= SET_CFGX_CH_PRIOR(ChnlPriority);
	GDMA->CH[GDMA_ChNum].CFG_LOW = CfgxLow;

}

/**
  * @brief  Suspend a channel.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  */
__weak  void
GDMA_Suspend(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}
	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	GDMA->CH[GDMA_ChNum].CFG_LOW |= BIT_CFGx_L_CH_SUSP;
}

/**
  * @brief  Resume a channel.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  */
__weak  void
GDMA_Resume(u8 GDMA_Index, u8 GDMA_ChNum)
{
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}
	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	GDMA->CH[GDMA_ChNum].CFG_LOW &= ~BIT_CFGx_L_CH_SUSP;
}

/**
  * @brief  Abort a channel.
  * @param  GDMA_Index: 0.
  * @param  GDMA_ChNum: 0 ~ 7.
  * @retval TRUE/FALSE
  */
__weak  u8
GDMA_Abort(u8 GDMA_Index, u8 GDMA_ChNum)
{
	u32 timeout = 500;
	GDMA_TypeDef *GDMA = ((GDMA_TypeDef *) GDMA_BASE);
	if (TrustZone_IsSecure()) {
		GDMA = ((GDMA_TypeDef *) GDMA0_REG_BASE_S);
	}
	/* Check the parameters */
	assert_param(IS_GDMA_Index(GDMA_Index));
	assert_param(IS_GDMA_ChannelNum(GDMA_ChNum));

	GDMA_Suspend(GDMA_Index, GDMA_ChNum);
	/*If ChEnReg[GDMA_ChNum] is not equal to 0, it means that
	  the channel is working and the Suspend status must be checked.*/
	while (timeout--) {
		if ((GDMA->ChEnReg & BIT(GDMA_ChNum)) == 0 || \
			(GDMA->CH[GDMA_ChNum].CFG_LOW & BIT_CFGx_L_INACTIVE)) {
			break;
		}
	}
	/*If the channel is still active after the timeout period, resume is required*/
	if (timeout == 0) {
		GDMA_Resume(GDMA_Index, GDMA_ChNum);
		return FALSE;
	}

	GDMA_Cmd(GDMA_Index, GDMA_ChNum, DISABLE);
	return TRUE;
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
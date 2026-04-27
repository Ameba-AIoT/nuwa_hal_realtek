/*
 * Copyright (c) 2025, Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ameba_soc.h>
#include <boot_security_km4tz.h>
#include <bootloader_km4tz.h>
#include <boot_ota_km4tz.h>

#include <sysflash/sysflash.h>

#include <string.h>

#include "bootutil/bootutil.h"

_LONG_CALL_ void RCC_PeriphClockCmd(u32 APBPeriph, u32 APBPeriph_Clock, u8 NewState);
extern void BOOT_ROM_Copy(void *__restrict dst0, const void *__restrict src0, size_t len0);
extern void RSIP_IV_Set(uint8_t index, uint8_t *IV);

extern MCM_MemTypeDef meminfo;

static const u32 ImagePattern[2] = {
	0x35393138,
	0x31313738,
};

int BOOT_RSIP_Load_Image(uint8_t id, uint8_t *iv, struct image_header *hdr)
{
	uint32_t off = 0;
	const struct flash_area *fap = NULL;
	int rc;

	RTK_LOGI(NOTAG, "Attempting to parse IV from TLV...");

	rc = flash_area_open(id, &fap);
	if (rc != 0) {
		return rc;
	}

	rc = flash_area_read(fap, off, hdr, sizeof(*hdr));
	if (rc != 0) {
		goto end;
	}

#ifdef CONFIG_MCUBOOT
	/*TODO: TF-M bl2 support RSIP */

	/* Traverse through the TLV area to find the image hash TLV. */
	struct image_tlv_iter it = {0};
	uint16_t type;
	uint16_t len;
#if defined(MCUBOOT_SWAP_USING_OFFSET)
	it.start_off = 0;
#endif
	rc = bootutil_tlv_iter_begin(&it, hdr, fap, IMAGE_TLV_ANY, false);

	if (rc != 0) {
		goto end;
	}

	while (true) {
		rc = bootutil_tlv_iter_next(&it, &off, &len, &type);
		if (rc != 0) {
			goto end;
		}


		if (type == CONFIG_AMEBA_RSIP_IV_TYPE_IN_TLV) {
			/* Get the image's hash value from the manifest section. */
			if (len != 8) {
				rc = -1;
				goto end;
			}

			rc = flash_area_read(fap, off, iv, len);
			break;
		}
	}
#endif
end:
	flash_area_close(fap);
	return rc;
}

u32  Boot_Copy_NP_Image(uint32_t np_logic_addr)
{
	static const char *const NpLabel[] = {"NP XIP IMG", "NP SRAM", "NP PSRAM"};
	u32 StartAddr = np_logic_addr;
	IMAGE_HEADER ImgHdr;
	u32 DstAddr, Len;
	u32 i;

	for (i = 0; i < 3; i++) {
		BOOT_ROM_Copy((void *)&ImgHdr, (void *)StartAddr, IMAGE_HEADER_LEN);
		if (_memcmp(ImgHdr.signature, ImagePattern, sizeof(ImagePattern)) != 0) {
			return 0;
		}

		DstAddr = ImgHdr.image_addr - IMAGE_HEADER_LEN;
		Len = ImgHdr.image_size + IMAGE_HEADER_LEN;

		/* np rom code jump address is from NP_BOOT_INDEX */
		if (ImgHdr.boot_index == NP_BOOT_INDEX) {
			RTK_LOGD(NOTAG, "NP_BOOT_INDEX: %x", ImgHdr.image_addr);
			HAL_WRITE32(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_ADDR_NS, ImgHdr.image_addr);
		}

		/* If not XIP sub-image, copy it to specific address(include the IMAGE_HEADER)*/
		RTK_LOGD(NOTAG, "try copy %s: %x <- %x, %x", NpLabel[i], DstAddr, StartAddr, Len);
		if ((!IS_FLASH_ADDR(DstAddr)) && (Len > IMAGE_HEADER_LEN)) {
			RTK_LOGD(NOTAG, "  copy");
			BOOT_ROM_Copy((void *)DstAddr, (void *)StartAddr, Len);
			DCache_CleanInvalidate(DstAddr, Len);
		}

		/* empty Image, Just put in flash, for image hash later */
		if (Len == IMAGE_HEADER_LEN) {
			DstAddr = StartAddr;
		}
		StartAddr += Len;
	}
	return StartAddr - np_logic_addr;
}

void BOOT_RSIP_MMU_Config(uint32_t flash_phy_addr,
						  uint8_t app_flash_id,
						  uint32_t app_flash_offset,
						  uint32_t app_flash_size,
						  uint32_t ap_logic_addr,
						  uint32_t np_logic_addr)
{
	FIH_DECLARE(fih_rc, FIH_FAILURE);

	struct image_header hdr_img;
	uint32_t np_size;
	uint32_t ap_size;
	uint8_t iv[8];

	uint32_t start, end;

	BOOT_RSIP_Load_Image(app_flash_id, iv, &hdr_img);

	if (SYSCFG_OTP_RSIPEn() == TRUE) {
		RSIP_IV_Set(RSIP_IV1, iv);
	}

	/* NOTE: Mapping start address should take mcuboot header(0x200) into consideration to make
	 * actual code start at np_logic_addr
	 */
	RSIP_MMU_Config(MMU_ID1, np_logic_addr - hdr_img.ih_hdr_size, ap_logic_addr,
					flash_phy_addr + app_flash_offset);
	RSIP_MMU_Cmd(MMU_ID1, ENABLE);
	RSIP_MMU_Cache_Clean();

	FIH_CALL(BOOT_OTFCheck, fih_rc, np_logic_addr, np_logic_addr + app_flash_size, RSIP_IV1, RSIP_REGION1);
	np_size = Boot_Copy_NP_Image(np_logic_addr);
	ap_size = hdr_img.ih_img_size - np_size;

	RTK_LOGD(NOTAG, "np: %u, %u, %x\n", np_size, ap_size, hdr_img.ih_img_size);

	/* NOTE: KM4TZ's code start address is determing by:
	 *   CONFIG_FLASH_BASE_ADDRESS(like:0x04000000) + DT_REG_ADDR(IMG_TZ_PATITION)
	 *   CONFIG_FLASH_BASE_ADDRESS is configured to primary_logic_addr_base
	 *   primary_logic_addr_base equals to IMG0_LOGIC_ADDR - IMG0_SLOT0_OFFSET
	 *   IMG0_SLOT0_OFFSET equals to DT_REG_ADDR(IMG_TZ_PATITION)
	 *
	 *   Actual code start at ap_logic_addr + hdr_img.ih_hdr_size
	 */
	RSIP_MMU_Config(MMU_ID2, ap_logic_addr + hdr_img.ih_hdr_size,
					ap_logic_addr + 0x02000000,
					flash_phy_addr + app_flash_offset + np_size +
					hdr_img.ih_hdr_size);
	RSIP_MMU_Cmd(MMU_ID2, ENABLE);
	RSIP_MMU_Cache_Clean();

	start = ap_logic_addr + hdr_img.ih_hdr_size;
	end = start + ap_size;
	FIH_CALL(BOOT_OTFCheck, fih_rc, start, end, RSIP_IV1, RSIP_REGION2);

#ifdef BL2
	/* NOTE: Mapping start address take mcuboot header into consideration to make
	 * actual code start at S_CODE_START, same as MMU_ID1
	 */
	RSIP_MMU_Config(MMU_ID3, S_CODE_START - hdr_img.ih_hdr_size, 0x11000000,
					flash_phy_addr + S_IMAGE_PRIMARY_PARTITION_OFFSET);
	RSIP_MMU_Cmd(MMU_ID3, ENABLE);
	RSIP_MMU_Cache_Clean();
#endif
}

void boot_prepare(uint32_t flash_phy_addr,
				  uint8_t app_flash_id,
				  uint32_t app_flash_offset,
				  uint32_t app_flash_size,
				  uint32_t ap_logic_addr,
				  uint32_t np_logic_addr)
{
#if 0
	/*Usage for jlink debug */
	Pinmux_Config(_PA_6, PINMUX_FUNCTION_SWD_CLK);
	PAD_PullCtrl(_PA_6, GPIO_PuPd_UP);
	Pinmux_Config(_PA_7, PINMUX_FUNCTION_SWD_DAT);
	PAD_PullCtrl(_PA_7, GPIO_PuPd_UP);
#endif

	FIH_DECLARE(fih_rc, FIH_FAILURE);

	BOOT_ReasonSet();

	/* For debug reset: when debugger reset cpu, it's required to reset other cpus and some
	 * peripherals
	 */
	if (BOOT_Reason() & (AON_BIT_RSTF_WARM_KM4NS | AON_BIT_RSTF_WARM_KM4TZ)) {
		Peripheral_Reset();
	}

	/* BOOT Reason: POR or BOR. */
	/* BOD is enabled by default. BOR may arise if voltage increases slowly during POR. */
	/* To avoid Retention RAM cannot be initialized to 0 correctly. */
	if (((BOOT_Reason() & ~AON_BIT_RSTF_BOR) == 0) || (!BOOT_RRAM_InfoValid())) {
		RRAM_DEV->MAGIC_NUMBER = 0;
		memset(RRAM_DEV, 0, sizeof(RRAM_TypeDef));
		RRAM_DEV->MAGIC_NUMBER = 0x6969A5A5;
	}

	BOOT_VerCheck();
	BOOT_SOC_ClkSet();
	BOOT_Log_Init();
	BOOT_RccConfig();
	BOOT_ResetMask_Config();

	meminfo = ChipInfo_MCMInfo();
	int ret = BOOT_PSRAM_Init();

	if (ret == -1) {
		/* psram initial fail or non-psram chip，close psram LDO(mem_ldo 1.8V)*/
		LDO_MemSetInNormal(MLDO_OFF);
#ifdef CONFIG_PSRAM_USED
		assert_param(0); /*Code Can only XIP When No Psram*/
#endif
	} else {
		LDO_MemSetInNormal(MLDO_NORMAL);
		LDO_MemSetInSleep(MLDO_SLEEP);
	}

	BOOT_RSIP_MMU_Config(flash_phy_addr, app_flash_id, app_flash_offset,
						 app_flash_size, ap_logic_addr, np_logic_addr);

	BOOT_Data_Flash_Init();
	flash_highspeed_setup();

	FIH_CALL(BOOT_RAM_TZCfg, fih_rc);
	if (FIH_NOT_EQ(fih_rc, FIH_SUCCESS)) {
		RTK_LOGE(NOTAG, "Boot tz config failed");
		FIH_PANIC;
	}
	BOOT_Enable_NP();
}

void BOOT_Image1(void)
{
#ifdef CONFIG_MCUBOOT
	extern void z_arm_reset(void);
	z_arm_reset();
#else
	extern void Reset_Handler(void);
	Reset_Handler();
#endif
}

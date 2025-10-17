/*
 * Copyright (c) 2025 Realtek Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "osif.h"
#include "hci/hci_common.h"
#include "hci/hci_transport.h"
#include "hci_uart.h"
#include "hci_platform.h"
#include "bt_debug.h"
#include "ameba.h"
#include "platform_autoconf.h"
#if defined(CONFIG_BT_COEXIST)
#include "rtw_coex_host_api.h"
#endif
#include "rtw_wifi_defs.h"

#define HCI_PHY_EFUSE_OFFSET       0x120
#define HCI_PHY_EFUSE_LEN          0x13
#define HCI_LGC_EFUSE_OFFSET       0x190
#define HCI_LGC_EFUSE_LEN          0x20
#define HCI_MAC_ADDR_LEN           6
#define HCI_CONFIG_SIGNATURE       0x8723ab55
#define HCI_CONFIG_HDR_LEN         6

#define HCI_CFG_BAUDRATE           BIT0
#define HCI_CFG_FLOWCONTROL        BIT1
#define HCI_CFG_BD_ADDR            BIT2
#define LEFUSE(x)                  ((x)-HCI_LGC_EFUSE_OFFSET)
#define PEFUSE(x)                  ((x)-HCI_PHY_EFUSE_OFFSET)

struct BT_Cali_TypeDef {
	u32 IQK_xx;
	u32 IQK_yy;
	u32 IDAC;
	u32 QDAC;
};

static uint8_t hci_phy_efuse[HCI_PHY_EFUSE_LEN]  = {0};
static uint8_t hci_lgc_efuse[HCI_LGC_EFUSE_LEN]  = {0};
unsigned char hci_init_config[] = {
	0x55, 0xab, 0x23, 0x87,

	0x19, 0x00,

	/* BT MAC address */
	0x30, 0x00, 0x06, 0x11, 0x28, 0x36, 0x12, 0x51, 0x89,

	/* Baudrate 921600 */
	0x0c, 0x00, 0x04, 0x04, 0x50, 0xF7, 0x03,

	/* flow control */
	0x18, 0x00, 0x01, 0x5c,

	/* efuse value */
	0x94, 0x01, 0x06, 0x08, 0x00, 0x00, 0x00, 0x27, 0x07,

	0x9f, 0x01, 0x05, 0x23, 0x23, 0x23, 0x23, 0x59,

	0xA4, 0x01, 0x04, 0xFE, 0xFE, 0xFE, 0xFE,

	/* LE trx on delay */
	0xD6, 0x00, 0x02, 0x8C, 0x8C
};
unsigned int hci_init_config_len = sizeof(hci_init_config);

static uint8_t  hci_cfg_bd_addr[HCI_MAC_ADDR_LEN] = {0};
static uint32_t hci_cfg_flag                      = 0;

extern int wifi_set_ips_internal(u8 enable);
extern int wifi_set_lps_enable(u8 enable);
extern int wifi_is_running(unsigned char wlan_idx);

void hci_platform_cfg_bd_addr(uint8_t *bdaddr)
{
	/* bdaddr is little endian, please print in reverse order from high address to low address to confirm habits */
	for (uint8_t i = 0; i < HCI_MAC_ADDR_LEN; i++) {
		hci_cfg_bd_addr[i] = bdaddr[i];
	}

	hci_cfg_flag |= HCI_CFG_BD_ADDR;
}

#if defined(CONFIG_BT_COEXIST)
#define RESERVE_LEN 1
static uint8_t hci_write_reg(uint8_t offset, uint16_t value)
{
	/* OpCode: 0xFD4A, Data Len: Cmd(7), Event(6) */
	uint16_t opcode = 0xFD4A;
	uint8_t buf_raw[RESERVE_LEN + 7];
	uint8_t *buf = buf_raw + RESERVE_LEN;

	buf[2] = 4;
	buf[3] = offset;
	buf[4] = (value >> 0) & 0xff;
	buf[5] = (value >> 8) & 0xff;
	buf[6] = 0;

	if (HCI_SUCCESS != hci_sa_send_cmd_sync(opcode, buf, buf[2] + 3)) {
		return HCI_FAIL;
	}

	return HCI_SUCCESS;
}
#endif

void hci_platform_bt_rf_calibration(void)
{
#if defined(CONFIG_BT_COEXIST)
	bool check_bt_iqk = false;
	bool lok_cond0 = false;
	bool lok_cond1 = false;
	bool lok_cond2 = false;
	u32 iqk_xx, iqk_yy, qdac, idac;
	struct bt_rfk_param temp_pram = {0};
	u32 result = 0;

	lok_cond0 = !(hci_lgc_efuse[LEFUSE(0x1A1)] & BIT0);
	lok_cond1 = !(hci_phy_efuse[1] & BIT0);
	lok_cond2 = !(((hci_lgc_efuse[0x16] == 0xff) && (hci_lgc_efuse[0x17] == 0xff)) ||
				  ((hci_lgc_efuse[0x18] == 0xff) && (hci_lgc_efuse[0x19] == 0xff)) ||
				  (hci_lgc_efuse[0x1a] == 0xff) ||
				  (hci_lgc_efuse[0x1b] == 0xff));

	if (lok_cond0) {
		BT_LOGD("[BT_RFK] USE FIX LOGIC EFUSE\r\n");
		if (lok_cond2) {
			BT_LOGD("[BT_RFK] logic efuse: has data\r\n");
		} else {
			BT_LOGD("[BT_RFK] bt_iqk_logic_efuse_valid: no data\r\n");
		}
	} else {
		BT_LOGD("[BT_RFK] LOGIC EFUSE HAS NO DATA\r\n");
	}

	if (lok_cond1) {
		BT_LOGD("[BT_RFK] physical efuse: has data hci_tp_phy_efuse[1]= %x \r\n", hci_phy_efuse[1]);
	} else {
		BT_LOGD("[BT_RFK] bt_iqk_efuse_valid: no data\r\n");
	}

	if (lok_cond2) {
		BT_LOGD("[BT_RFK] logic efuse: has data\r\n");
	} else {
		BT_LOGD("[BT_RFK] bt_iqk_logic_efuse_valid: no data\r\n");
	}
	// check if bt rfk
	if ((lok_cond0 && lok_cond2) ||
		(!lok_cond0 && !lok_cond1 && lok_cond2)) {
		iqk_xx = hci_lgc_efuse[0x16] | (hci_lgc_efuse[0x17] << 8);
		iqk_yy = hci_lgc_efuse[0x18] | (hci_lgc_efuse[0x19] << 8);
		qdac = hci_lgc_efuse[0x1a];
		idac = hci_lgc_efuse[0x1b];
		// LOK
		memset(&temp_pram, 0, sizeof(struct bt_rfk_param));
		temp_pram.type = BT_LOK;
		temp_pram.rfk_data1 = idac; // idac
		temp_pram.rfk_data2 = qdac; // qdac
		rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));

		hci_phy_efuse[0] = 0;
		hci_phy_efuse[1] = hci_phy_efuse[1] & (~BIT0);
		hci_phy_efuse[3] = (iqk_xx >> 0) & 0xff;
		hci_phy_efuse[4] = (iqk_xx >> 8) & 0xff;
		hci_phy_efuse[5] = (iqk_yy >> 0) & 0xff;
		hci_phy_efuse[6] = (iqk_yy >> 8) & 0xff;

		check_bt_iqk = true;
	}

	if (!lok_cond0 && lok_cond1) {
		if (hci_phy_efuse[0] != 0) {
			// DCK
			memset(&temp_pram, 0, sizeof(struct bt_rfk_param));
			temp_pram.type = BT_DCK;
			temp_pram.rfk_data1 = hci_phy_efuse[0xe];
			temp_pram.rfk_data2 = hci_phy_efuse[0xf];
			rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));
		} else {
			BT_LOGD("[BT_RFK] hci_tp_phy_efuse[0]=0\r\n");
		}

		iqk_xx = hci_phy_efuse[3] | (hci_phy_efuse[4] << 8);
		iqk_yy = hci_phy_efuse[5] | (hci_phy_efuse[6] << 8);
		qdac = hci_phy_efuse[0xc];
		idac = hci_phy_efuse[0xd];

		// LOK
		memset(&temp_pram, 0, sizeof(struct bt_rfk_param));
		temp_pram.type = BT_LOK;
		temp_pram.rfk_data1 = idac; // idac
		temp_pram.rfk_data2 = qdac; // qdac
		rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));

		check_bt_iqk = true;
	}

	// start rfk if check fail
	if (check_bt_iqk == false) {
		BT_LOGD("[BT_RFK] bt_check_iqk:  NO IQK LOK DATA need start LOK,\r\n");
		BT_LOGD("[BT_RFK] we need start iqk\r\n");
		// send vendor cmd if check_bt_iqk is false
		if (HCI_FAIL == hci_write_reg(0x00, 0x4000)) {
			BT_LOGD("[BT_RFK] Write RF 0x00=0x4000 fail\r\n");
			return;
		}
		if (HCI_FAIL == hci_write_reg(0x01, 0x0180)) {
			BT_LOGD("[BT_RFK] Write RF 0x01=0x0180 fail\r\n");
			return;
		}
		if (HCI_FAIL == hci_write_reg(0x02, 0x3800)) {
			BT_LOGD("[BT_RFK] Write RF 0x02=0x3800 fail\r\n");
			return;
		}
		if (HCI_FAIL == hci_write_reg(0x3f, 0x0400)) {
			BT_LOGD("[BT_RFK] Write RF 0x3f=0x0400 fail\r\n");
			return;
		}

		// start IQK
		memset(&temp_pram, 0, sizeof(struct bt_rfk_param));
		temp_pram.type = BT_IQK;
		if (RTK_SUCCESS == rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param))) {
			// rfk success
			result = (temp_pram.rfk_data1 << 24) + (temp_pram.rfk_data2 << 16) + (temp_pram.rfk_data3 << 8) + (temp_pram.rfk_data4);
			iqk_xx = (result >> 22) & 0x3FF; // iqk_xx
			iqk_yy = (result >> 12) & 0x3FF; // iqk_yy
			idac = (result >> 6) & 0x3F; // idac
			qdac = (result >> 0) & 0x3F; // qdac

			// LOK
			memset(&temp_pram, 0, sizeof(struct bt_rfk_param));
			temp_pram.type = BT_LOK;
			temp_pram.rfk_data1 = idac;
			temp_pram.rfk_data2 = qdac;
			rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));

			hci_phy_efuse[0] = 0;
			hci_phy_efuse[1] = hci_phy_efuse[1] & (~BIT0);
			hci_phy_efuse[3] = (iqk_xx >> 0) & 0xff;
			hci_phy_efuse[4] = (iqk_xx >> 8) & 0xff;
			hci_phy_efuse[5] = (iqk_yy >> 0) & 0xff;
			hci_phy_efuse[6] = (iqk_yy >> 8) & 0xff;

			BT_LOGD("\n\r[BT_RFK] IQK OK\r\n");
		} else {
			BT_LOGD("\n\r[BT_RFK] IQK FAIL\r\n");
		}
	}
#endif
}

int hci_platform_get_write_phy_efuse_data(uint8_t *data, uint8_t len)
{
	memcpy(data, hci_phy_efuse, len);
	return HCI_SUCCESS;
}

static uint8_t hci_platform_read_efuse(void)
{
	uint32_t Idx = 0;
	uint8_t p_buf[1024] = {0};

	/* Read Logic Efuse */
	if (EFUSE_LMAP_READ(p_buf) == RTK_FAIL) {
		BT_LOGE("EFUSE_LMAP_READ Fail!\r\n");
	}
	memcpy(hci_lgc_efuse, p_buf + HCI_LGC_EFUSE_OFFSET, HCI_LGC_EFUSE_LEN);

	/* Read Physical Efuse */
	for (Idx = 0; Idx < 16; Idx++) {
		EFUSE_PMAP_READ8(0, HCI_PHY_EFUSE_OFFSET + Idx, hci_phy_efuse + Idx, L25EOUTVOLTAGE);
		if ((Idx == 7) && (hci_phy_efuse[Idx] == 0)) {
			hci_phy_efuse[Idx] = 0x13;
		}
	}

	EFUSE_PMAP_READ8(0, 0x1FD, hci_phy_efuse + 16, L25EOUTVOLTAGE);
	EFUSE_PMAP_READ8(0, 0x1FE, hci_phy_efuse + 17, L25EOUTVOLTAGE);
	EFUSE_PMAP_READ8(0, 0x1FF, hci_phy_efuse + 18, L25EOUTVOLTAGE);

#if 0
	BT_DUMPA("Read Logic Efuse:\r\n", hci_lgc_efuse, HCI_LGC_EFUSE_LEN);
	BT_DUMPA("Read Phy Efuse:\r\n", hci_phy_efuse, HCI_PHY_EFUSE_LEN);
#endif

	return HCI_SUCCESS;
}

static uint8_t hci_platform_parse_config(void)
{
	uint8_t *p, i;
	uint16_t entry_offset, entry_len, config_length;
	uint32_t config_header;
	uint16_t tx_flatk;

	if (hci_init_config_len <= HCI_CONFIG_HDR_LEN) {
		return HCI_IGNORE;
	}

	p = hci_init_config;
	memcpy((void *)&config_header, (void *)p, sizeof(uint32_t));
	if (HCI_CONFIG_SIGNATURE != config_header) {
		return HCI_FAIL;
	}

	memcpy((void *)&config_length, (void *)(p + 4), sizeof(uint16_t));
	if (config_length != (uint16_t)(hci_init_config_len - HCI_CONFIG_HDR_LEN)) {
		/* Fix the len, just avoid the length is not corect */
		p[4] = (uint8_t)((hci_init_config_len - HCI_CONFIG_HDR_LEN) >> 0);
		p[5] = (uint8_t)((hci_init_config_len - HCI_CONFIG_HDR_LEN) >> 8);
	}

	p += HCI_CONFIG_HDR_LEN;
	while (p < hci_init_config + hci_init_config_len) {
		memcpy((void *)&entry_offset, (void *)p, sizeof(uint16_t));
		entry_len = *(uint8_t *)(p + 2);
		p += 3;

		switch (entry_offset) {
		case 0x000c:
			hci_set_work_baudrate(p);
			break;
		case 0x0018:
			/* MP Mode, Close Flow Control */
			if (hci_is_mp_mode()) {
				p[0] = p[0] & (~BIT2);
			}
			break;
		case 0x0030:
			/* Set ConfigBuf MacAddr, Use Customer Assign or Efuse */
			if (hci_cfg_flag & HCI_CFG_BD_ADDR) {
				for (i = 0; i < HCI_MAC_ADDR_LEN; i++) {
					p[i] = hci_cfg_bd_addr[i];
				}
			} else {
				if ((hci_lgc_efuse[0] != 0xff) || (hci_lgc_efuse[1] != 0xff) || (hci_lgc_efuse[2] != 0xff) || \
					(hci_lgc_efuse[3] != 0xff) || (hci_lgc_efuse[4] != 0xff) || (hci_lgc_efuse[5] != 0xff)) {
					for (i = 0; i < HCI_MAC_ADDR_LEN; i++) {
						p[i] = hci_lgc_efuse[HCI_MAC_ADDR_LEN - 1 - i];
					}
				}
			}
			BT_LOGA("Bluetooth init BT_ADDR in cfgbuf [%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
					p[5], p[4], p[3], p[2], p[1], p[0]);
			break;
		case 0x0194:
			if (hci_lgc_efuse[LEFUSE(0x196)] == 0xff) {
				if (!(hci_lgc_efuse[2]& BIT0)) {
					tx_flatk = hci_lgc_efuse[0xa] | hci_lgc_efuse[0xb] << 8;
					//bt_flatk_8721d(tx_flatk);
					//BT_LOGA("WRITE physical FLATK=tx_flatk=%x \r\n", tx_flatk);
#if defined(CONFIG_BT_COEXIST)
					struct bt_rfk_param temp_pram = {0};
					temp_pram.type = BT_FLATK;
					temp_pram.rfk_data1 = (tx_flatk >> 8) & 0xff;
					temp_pram.rfk_data2 = (tx_flatk >> 0) & 0xff;
					rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));
#else
					(void) tx_flatk;
#endif
				}
				break;
			} else {
				p[0] = hci_lgc_efuse[LEFUSE(0x196)];
				if (hci_lgc_efuse[LEFUSE(0x196)] & BIT1) {
					p[1] = hci_lgc_efuse[LEFUSE(0x197)];
				}

				if (hci_lgc_efuse[LEFUSE(0x196)] & BIT2) {
					p[2] = hci_lgc_efuse[LEFUSE(0x198)];
					p[3] = hci_lgc_efuse[LEFUSE(0x199)];

					tx_flatk = hci_lgc_efuse[LEFUSE(0x198)] | hci_lgc_efuse[LEFUSE(0x199)] << 8;
					//bt_flatk_8721d(tx_flatk);
					//BT_LOGA("\r\n WRITE logic FLATK=tx_flatk=%x \r\n", tx_flatk);
#if defined(CONFIG_BT_COEXIST)
					struct bt_rfk_param temp_pram = {0};
					temp_pram.type = BT_FLATK;
					temp_pram.rfk_data1 = (tx_flatk >> 8) & 0xff;
					temp_pram.rfk_data2 = (tx_flatk >> 0) & 0xff;
					rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));
#else
					(void) tx_flatk;
#endif
				} else {
					if (!(hci_lgc_efuse[2]& BIT0)) {
						tx_flatk = hci_lgc_efuse[0xa] | hci_lgc_efuse[0xb] << 8;
						//bt_flatk_8721d(tx_flatk);
						//BT_LOGA("\r\n WRITE  physical FLATK=tx_flatk=%x \r\n", tx_flatk);
#if defined(CONFIG_BT_COEXIST)
						struct bt_rfk_param temp_pram = {0};
						temp_pram.type = BT_FLATK;
						temp_pram.rfk_data1 = (tx_flatk >> 8) & 0xff;
						temp_pram.rfk_data2 = (tx_flatk >> 0) & 0xff;
						rtk_coex_btc_bt_rfk(&temp_pram, sizeof(struct bt_rfk_param));
#else
						(void)tx_flatk;
#endif
					}
				}

				if (hci_lgc_efuse[LEFUSE(0x196)] & BIT5) {
					p[4] = hci_lgc_efuse[LEFUSE(0x19a)];
					p[5] = hci_lgc_efuse[LEFUSE(0x19b)];
				}
			}
			break;
		case 0x019f:
			for (i = 0; i < entry_len; i++) {
				if (hci_lgc_efuse[LEFUSE(0x19c + i)] != 0xff) {
					p[i] = hci_lgc_efuse[LEFUSE(0x19c + i)];
				}
			}
			break;
		case 0x01a4:
			for (i = 0; i < entry_len; i++) {
				if (hci_lgc_efuse[LEFUSE(0x1a2 + i)] != 0xff) {
					p[i] = hci_lgc_efuse[LEFUSE(0x1a2 + i)];
				}
			}
			break;
		default:
			break;
		}

		p += entry_len;
	}

	return HCI_SUCCESS;
}

static void bt_power_on(void)
{
	set_reg_value(0x40000000, BIT0 | BIT1, 3);
	osif_delay(5);
}

static void bt_power_off(void)
{
	set_reg_value(0x40000000, BIT0 | BIT1, 0);
	osif_delay(5);
}

void hci_platform_controller_reset(void)
{
	/* BT Controller Power */
	bt_power_on();

	/* isolation */
	set_reg_value(0x40000000, BIT16, 0);
	osif_delay(5);

	/* BT function enable */
	set_reg_value(0x40000204, BIT24, 0);
	osif_delay(5);

	set_reg_value(0x40000204, BIT24, 1);
	osif_delay(50);

	/* BT clock enable */
	set_reg_value(0x40000214, BIT24, 1);
	osif_delay(5);

	BT_LOGD("BT Reset OK!\r\n");
}

bool rtk_bt_pre_enable(void)
{
#if defined(CONFIG_WLAN) && CONFIG_WLAN
	if (!(wifi_is_running(STA_WLAN_INDEX) || wifi_is_running(SOFTAP_WLAN_INDEX))) {
		BT_LOGE("WiFi is OFF! Please Restart BT after Wifi on!\r\n");
		return false;
	}

	if (hci_is_wifi_need_leave_ps()) {
		wifi_set_lps_enable(FALSE);
		wifi_set_ips_internal(FALSE);
	}
#endif

	return true;
}

void rtk_bt_post_enable(void)
{

}

uint8_t hci_platform_open(void)
{
	if (!CHECK_CFG_SW(CFG_SW_BT_FW_LOG)) {
		/* open bt log pa16 */
		/* close please change efuse:EFUSE wmap 1a1 1 fe */
		set_reg_value(0x48000440, BIT0 | BIT1 | BIT2 | BIT3 | BIT4, 17);
		BT_LOGA("FW LOG OPEN\r\n");
	}

	/* Read Efuse and Parse Configbuf */
	if (HCI_FAIL == hci_platform_read_efuse()) {
		return HCI_FAIL;
	}

	if (HCI_FAIL == hci_platform_parse_config()) {
		return HCI_FAIL;
	}

	/* BT Controller Reset */
	hci_platform_controller_reset();

	/* UART Open */
	if (HCI_FAIL == hci_uart_open()) {
		return HCI_FAIL;
	}

	return HCI_SUCCESS;
}

void hci_platform_close(void)
{
	/* BT Controller Power Off */
	bt_power_off();

	/* UART Close */
	hci_uart_close();

#if defined(CONFIG_WLAN) && CONFIG_WLAN
	if (hci_is_wifi_need_leave_ps()) {
		wifi_set_lps_enable(wifi_user_config.lps_enable);
		wifi_set_ips_internal(wifi_user_config.ips_enable);
	}
#endif
}

void hci_platform_free(void)
{
	/* UART Free */
	hci_uart_free();
}

void hci_platform_get_config(uint8_t **buf, uint16_t *len)
{
	*buf = hci_init_config;
	*len = hci_init_config_len;
}
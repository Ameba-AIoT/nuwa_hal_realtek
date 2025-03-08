/*
* Copyright (c) 2024 Realtek Semiconductor Corp.
*
* SPDX-License-Identifier: Apache-2.0
*/

#include "wifi_conf.h"

#define CH_NULL 1

/* allow customer to adjust the pwrlmt regu corresponding to domain code, for example*/
volatile struct _pwr_lmt_regu_remap pwrlmt_regu_remapping_1v8[1] = {{0}};

volatile u8 array_len_of_pwrlmt_regu_remapping_1v8 = sizeof(pwrlmt_regu_remapping_1v8) / sizeof(struct _pwr_lmt_regu_remap);

/******************************************************************************
 *                             TX_Power Limit
 ******************************************************************************/

// regu_en_1v8 = {FCC, MKK, ETSI, IC, KCC, ACMA, CHILE, MEXICO, WW, GL, UKRAINE, CN, QATAR, UK, NCC, EXT}
const bool regu_en_1v8[16] = {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0};

u8 regu_en_array_len_1v8 = sizeof(regu_en_1v8) / sizeof(bool);

const s8 tx_pwr_limit_2g_fcc_1v8[4][14] = {
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 52, 28, 127}, /*CCK*/
	{40, 44, 56, 56, 56, 56, 56, 56, 56, 44, 40, 32, 4, 127}, /*OFDM*/
	{36, 40, 52, 52, 52, 52, 52, 52, 52, 40, 36, 32, 0, 127}, /*HT_B20*/
	{127, 127, 36, 44, 52, 52, 44, 36, 32, 28, 8, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_fcc_1v8[3][28] = {
	{32, 40, 40, 40, 40, 40, 40, 32, 32, 40, 40, 40, 40, 40, 40, 40, 40, 40, 32, 40, 40, 40, 40, 40, 40, 40, 40, 40}, /*OFDM*/
	{28, 36, 36, 36, 36, 36, 36, 28, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 28, 36, 36, 36, 36, 36, 36, 36, 36, 36}, /*HT_B20*/
	{20, 32, 32, 20, 24, 34, 34, 34, 26, 34, 36, 36, 36, 36}  /*HT_B40*/
};
const u8 tx_shap_fcc_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_etsi_1v8[4][14] = {
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 127}, /*CCK*/
	{52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 127}, /*OFDM*/
	{52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 127}, /*HT_B20*/
	{127, 127, 48, 48, 48, 48, 48, 48, 48, 48, 48, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_etsi_1v8[3][28] = {
	{40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 127, 32, 32, 32, 32, 32, 32, 32, 127}, /*OFDM*/
	{40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 127, 32, 32, 32, 32, 32, 32, 32, 127}, /*HT_B20*/
	{36, 36, 36, 36, 36, 36, 36, 36, 36, 127, 32, 32, 32, 127}  /*HT_B40*/
};
const u8 tx_shap_etsi_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_mkk_1v8[4][14] = {
	{60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60}, /*CCK*/
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 127}, /*OFDM*/
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 127}, /*HT_B20*/
	{127, 127, 52, 52, 52, 52, 52, 52, 52, 52, 52, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_mkk_1v8[3][28] = {
	{40, 40, 40, 40, 40, 40, 40, 36, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 127, 127, 127, 127, 127, 127, 127, 127}, /*OFDM*/
	{36, 40, 40, 40, 40, 40, 40, 36, 36, 40, 40, 40, 40, 40, 40, 40, 40, 40, 36, 36, 127, 127, 127, 127, 127, 127, 127, 127}, /*HT_B20*/
	{36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 127, 127, 127, 127}  /*HT_B40*/
};
const u8 tx_shap_mkk_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_ic_1v8[4][14] = {
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 44, 28, 127}, /*CCK*/
	{40, 44, 56, 56, 56, 56, 56, 56, 56, 44, 40, 32, 4, 127}, /*OFDM*/
	{36, 40, 52, 52, 52, 52, 52, 52, 52, 40, 36, 32, 0, 127}, /*HT_B20*/
	{127, 127, 36, 44, 52, 52, 44, 36, 32, 28, 8, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_ic_1v8[3][28] = {
	{32, 36, 36, 36, 40, 40, 40, 32, 32, 40, 40, 40, 40, 127, 127, 127, 40, 40, 32, 34, 36, 40, 40, 40, 40, 127, 127, 127}, /*OFDM*/
	{32, 32, 32, 32, 36, 36, 36, 32, 28, 36, 36, 36, 36, 127, 127, 127, 36, 36, 28, 28, 32, 36, 36, 36, 36, 127, 127, 127}, /*HT_B20*/
	{20, 32, 32, 20, 24, 34, 127, 127, 26, 34, 36, 36, 127, 127}  /*HT_B40*/
};
const u8 tx_shap_ic_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_kcc_1v8[4][14] = {
	{56, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 56, 127}, /*CCK*/
	{52, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 52, 127}, /*OFDM*/
	{52, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 52, 127}, /*HT_B20*/
	{127, 127, 48, 52, 52, 52, 52, 52, 52, 52, 48, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_kcc_1v8[3][28] = {
	{36, 40, 40, 36, 28, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 36, 32, 127, 127, 127}, /*OFDM*/
	{32, 32, 32, 30, 28, 40, 40, 36, 36, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 36, 28, 127, 127, 127}, /*HT_B20*/
	{28, 28, 28, 32, 32, 36, 36, 36, 36, 36, 28, 28, 127, 127}  /*HT_B40*/
};
const u8 tx_shap_kcc_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_cn_1v8[4][14] = {
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 48, 127}, /*CCK*/
	{52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 36, 127}, /*OFDM*/
	{52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 48, 24, 127}, /*HT_B20*/
	{127, 127, 48, 48, 48, 48, 48, 48, 48, 48, 48, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_cn_1v8[3][28] = {
	{40, 40, 40, 40, 40, 40, 40, 40, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 32, 32, 32, 32, 32, 127, 127, 127}, /*OFDM*/
	{40, 40, 40, 40, 40, 40, 40, 40, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 32, 32, 32, 32, 32, 127, 127, 127}, /*HT_B20*/
	{36, 36, 36, 36, 127, 127, 127, 127, 127, 127, 28, 28, 127, 127}  /*HT_B40*/
};
const u8 tx_shap_cn_1v8[2][4] = {{0, 0, 0, 0}, {0, 0, 0}}; /*{2G{CCK, OFDM, HE_B20, HE_B40}, 5G{OFDM, HE_B20, HE_B40}}*/

const s8 tx_pwr_limit_2g_ww_1v8[4][14] = {
	{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 44, 28, 60}, /*CCK*/
	{40, 44, 52, 52, 52, 52, 52, 52, 52, 44, 40, 32, 4, 127}, /*OFDM*/
	{36, 40, 52, 52, 52, 52, 52, 52, 52, 40, 36, 32, 0, 127}, /*HT_B20*/
	{127, 127, 36, 44, 48, 48, 44, 36, 32, 28, 8, 127, 127, 127}  /*HT_B40*/
};
const s8 tx_pwr_limit_5g_ww_1v8[3][28] = {
	{32, 36, 36, 36, 28, 40, 40, 32, 32, 40, 40, 40, 40, 40, 40, 40, 40, 40, 32, 34, 32, 32, 32, 32, 32, 32, 32, 40}, /*OFDM*/
	{28, 32, 32, 30, 28, 36, 36, 28, 28, 36, 36, 36, 36, 36, 36, 36, 36, 36, 28, 28, 32, 32, 32, 32, 28, 32, 32, 36}, /*HT_B20*/
	{20, 28, 28, 20, 24, 34, 34, 34, 26, 34, 28, 28, 32, 36}  /*HT_B40*/
};
const s8 tx_pwr_limit_2g_acma_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_acma_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_acma_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_chile_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_chile_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_chile_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_mexico_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_mexico_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_mexico_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_ukraine_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_ukraine_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_ukraine_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_qatar_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_qatar_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_qatar_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_uk_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_uk_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_uk_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_ncc_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_ncc_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_ncc_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_2g_ext_1v8[][CH_NULL] = {{0}};
const s8 tx_pwr_limit_5g_ext_1v8[][CH_NULL] = {{0}};
const u8 tx_shap_ext_1v8[][CH_NULL] = {{0}};


/******************************************************************************
 *                           TX_Power byRate
 ******************************************************************************/
const s8 array_mp_txpwr_byrate_2g_1v8[] = {
	0x30, 0x34, 0x38, 0x38,  //11M 5.5M 2M 1M is 12dbm 13dbm 14dbm 14dBm
	0x30, 0x34, 0x34, 0x38,  //18M 12M 9M 6M is 12dbm 13dbm 13dbm 14dBm
	0x2c, 0x2c, 0x2c, 0x30,  //54M 48M 36M 24M is 11dbm 11dbm 11dbm 12dBm
	0x2c, 0x30, 0x30, 0x34,  // HT_MCS3 MCS2 MCS1 MCS0 is 11dbm 12dbm 12dbm 13dBm
	0x28, 0x28, 0x28, 0x2c  // HT_MCS7 MCS6 MCS5 MCS4 is 10dbm 10dbm 10dbm 11dBm
};
u8 array_mp_txpwr_byrate_2g_array_len_1v8 = sizeof(array_mp_txpwr_byrate_2g_1v8) / sizeof(s8);
const s8 array_mp_txpwr_byrate_5g_1v8[] = {
	0x2c, 0x2c, 0x30, 0x30,  //18M 12M 9M 6M is 11dbm 11dbm 12dbm 12dBm
	0x24, 0x24, 0x28, 0x28,  //54M 48M 36M 24M is 9dbm 9dbm 10dbm 10dBm
	0x28, 0x28, 0x2c, 0x2c,  // HT_MCS3 MCS2 MCS1 MCS0 is 10dbm 10dbm 11dbm 11dBm
	0x20, 0x20, 0x24, 0x24  // HT_MCS7 MCS6 MCS5 MCS4 is 8dbm 8dbm 9dbm 9dBm
};
u8 array_mp_txpwr_byrate_5g_array_len_1v8 = sizeof(array_mp_txpwr_byrate_5g_1v8) / sizeof(s8);

s8 wifi_hal_phy_get_power_limit_value_1v8(u8 regulation, u8 band, u8 limit_rate, u8 chnl_idx, bool is_shape)
{
	s8 power_limit = 127;
	s8 tx_shape_idx = -1;

	switch (regulation) {
	case TXPWR_LMT_FCC:
		if (is_shape) {
			tx_shape_idx = tx_shap_fcc_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_fcc_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_fcc_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_ETSI:
		if (is_shape) {
			tx_shape_idx = tx_shap_etsi_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_etsi_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_etsi_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_MKK:
		if (is_shape) {
			tx_shape_idx = tx_shap_mkk_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_mkk_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_mkk_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_IC:
		if (is_shape) {
			tx_shape_idx = tx_shap_ic_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_ic_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_ic_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_KCC:
		if (is_shape) {
			tx_shape_idx = tx_shap_kcc_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_kcc_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_kcc_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_ACMA:
		if (is_shape) {
			tx_shape_idx = tx_shap_acma_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_acma_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_acma_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_NCC:
		if (is_shape) {
			tx_shape_idx = tx_shap_ncc_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_ncc_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_ncc_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_MEXICO:
		if (is_shape) {
			tx_shape_idx = tx_shap_mexico_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_mexico_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_mexico_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_CHILE:
		if (is_shape) {
			tx_shape_idx = tx_shap_chile_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_chile_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_chile_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_UKRAINE:
		if (is_shape) {
			tx_shape_idx = tx_shap_ukraine_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_ukraine_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_ukraine_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_CN:
		if (is_shape) {
			tx_shape_idx = tx_shap_cn_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_cn_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_cn_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_QATAR:
		if (is_shape) {
			tx_shape_idx = tx_shap_qatar_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_qatar_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_qatar_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_UK:
		if (is_shape) {
			tx_shape_idx = tx_shap_uk_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_uk_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_uk_1v8[limit_rate][chnl_idx];
			}
		}
		break;

	case TXPWR_LMT_WW:
		if (is_shape) {
			tx_shape_idx = -1;
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_ww_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_ww_1v8[limit_rate][chnl_idx];
			}
		}
		break;
	case TXPWR_LMT_EXT:
		if (is_shape) {
			tx_shape_idx = tx_shap_ext_1v8[band][limit_rate];
		} else {
			if (band == BAND_ON_24G) {
				power_limit = tx_pwr_limit_2g_ext_1v8[limit_rate][chnl_idx];
			} else {
				power_limit = tx_pwr_limit_5g_ext_1v8[limit_rate][chnl_idx];
			}
		}
		break;
	default:
		break;
	}

	if (is_shape) {
		return tx_shape_idx;
	} else {
		return power_limit;
	}
}

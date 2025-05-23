/**
  ******************************************************************************
  * @file    wifi_intf_drv_to_app_basic.h
  * @author
  * @version
  * @date
  * @brief   This file provides user interface for Wi-Fi station and AP mode configuration
  *             base on the functionalities provided by Realtek Wi-Fi driver.
  ******************************************************************************
  * @attention
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2024, Realtek Semiconductor Corporation. All rights reserved.
  ******************************************************************************
  */

#ifndef __WIFI_CONF_BASIC_H
#define __WIFI_CONF_BASIC_H

/** @defgroup WIFI_API
 *  @brief      WIFI_API module
 *  @{
 */
#include "rtw_wifi_common.h"
#if !defined (CONFIG_FULLMAC) && !(defined(ZEPHYR_WIFI))
#include "rtw_wifi_constants.h"
#include "platform_stdlib.h"
#include "basic_types.h"
#include "wifi_intf_drv_to_bt.h"
#include "dlist.h"
#include <rtw_skbuff.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup WIFI_Exported_Types WIFI Exported Types
* @{
*/
/**********************************************************************************************
 *                                  common structures
 *********************************************************************************************/
/**
 * @brief The enumeration lists the results of the function, size int
 */
enum {
	RTW_SUCCESS                      = 0,	/**< Success */

	RTW_ERROR                        = -1,	/**< Generic Error */
	RTW_BADARG                       = -2,	/**< Bad Argument */
	RTW_BUSY                         = -3,	/**< Busy */
	RTW_NOMEM                        = -4,	/**< No Memory */
	RTW_TIMEOUT                      = -5,	/**< Timeout */

	RTW_CONNECT_INVALID_KEY	         = -11,	/**< Invalid key */
	RTW_CONNECT_SCAN_FAIL            = -12,
	RTW_CONNECT_AUTH_FAIL            = -13,
	RTW_CONNECT_AUTH_PASSWORD_WRONG  = -14,
	RTW_CONNECT_ASSOC_FAIL           = -15,
	RTW_CONNECT_4WAY_HANDSHAKE_FAIL  = -16,
	RTW_CONNECT_4WAY_PASSWORD_WRONG  = -17,
};

/** @defgroup SSID_LEN_Defs
  * @{
  */
#define INIC_MAX_SSID_LENGTH (33)
/** @} */

#pragma pack(1)/*_rtw_ssid_t and _rtw_mac_t are 1 byte alignment for some issues long long ago*/
/**
  * @brief  The structure is used to describe the SSID.
  */
struct _rtw_ssid_t {
	unsigned char		len;     /**< SSID length */
	unsigned char		val[INIC_MAX_SSID_LENGTH]; /**< SSID name (AP name)  */
};

/**
  * @brief  The structure is used to describe the unique 6-byte MAC address.
  */
struct _rtw_mac_t {
	unsigned char		octet[6]; /**< Unique 6-byte MAC address */
};
#pragma pack()

/**********************************************************************************************
 *                                   scan structures
 *********************************************************************************************/
#define SCAN_LONGEST_WAIT_TIME	(12000) /**< scan longest wait time */
#define PSCAN_FAST_SURVEY	0x02 /**< set to select scan time to FAST_SURVEY_TO, otherwise SURVEY_TO */

#pragma pack(1)/*scan related structs are 1 byte alignment for some issues long long ago*/
/**
  * @brief  The structure is used to describe the busyness of a channe.
  */
struct acs_mntr_rpt {
	u16 meas_time; /*Measurements time on this channel, unit:ms*/
	u16 busy_time; /*time that the primary channel was sensed busy, unit:ms*/
	u16 tx_time; /*time spent transmitting frame on this channel, unit:ms */
	s8 noise; /*unit: dbm*/
	u8 channel;
};

/**
  * @brief  The structure is used to describe the scan result of the AP.
  */
struct rtw_scan_result {
	struct _rtw_ssid_t              SSID;             /**< Service Set Identification (i.e. Name of Access Point)                    */
	struct _rtw_mac_t               BSSID;            /**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
	signed short		                  signal_strength;  /**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
	u8          			   bss_type;         /**< val: RTW_BSS_TYPE_INFRASTRUCTURE, RTW_BSS_TYPE_WTN_HELPER*/
	u32					       security;         /**< val: RTW_SECURITY_OPEN, RTW_SECURITY_WEP_PSK...*/
	u8        				   wps_type;         /**< val: RTW_WPS_TYPE_DEFAULT, RTW_WPS_TYPE_USER_SPECIFIED...*/
	unsigned int               channel;          /**< Radio channel that the AP beacon was received on                          */
	u8					       band;             /**< val: RTW_802_11_BAND_5GHZ, RTW_802_11_BAND_2_4GHZ*/
	char	country_code[2];
	u8		rom_rsvd[4];
};

struct _rtw_channel_scan_time_t {
	unsigned short		active_scan_time;      /**< active scan time per channel, units: millisecond, default is 100ms */
	unsigned short		passive_scan_time;     /**< passive scan time per channel, units: millisecond, default is 110ms */
};

/**
  * @brief  The structure is used to describe the scan parameters used for scan,
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
struct _rtw_scan_param_t {
	u8										options;/*val: RTW_SCAN_ACTIVE, RTW_SCAN_PASSIVE...*/
	char									*ssid;
	unsigned char							*channel_list;
	unsigned char							channel_list_num;
	struct _rtw_channel_scan_time_t 				chan_scan_time;
	unsigned short						max_ap_record_num;	   /**< config the max number of recorded AP, when set to 0, use default value 64 */
	void									*scan_user_data;
	int (*scan_user_callback)(unsigned int ap_num, void *user_data);/**< used for normal asynchronized mode */
	int (*scan_report_each_mode_user_callback)(struct rtw_scan_result *scanned_ap_info, void *user_data); /*used for RTW_SCAN_REPORT_EACH mode */
	int (*scan_report_acs_user_callback)(struct acs_mntr_rpt *acs_mntr_rpt);/*used for report acs info*/
};
#pragma pack()
/**********************************************************************************************
 *                                     connect structures
 *********************************************************************************************/
/**
  * @brief  The structure is used for fullmac to get wpa_supplicant's info for STA connect,
  */
struct _rtw_wpa_supp_connect_t {
	u8 rsnxe_ie[RSNXE_MAX_LEN];
};

/**
  * @brief	The structure is used to describe the connection setting about SSID,
  * 		security type and password,etc., used when connecting to an AP.
  * @note  The data length of string pointed by ssid should not exceed 32,
  * 	   and the data length of string pointed by password should not exceed 64.
  * @note  If this struct is used for wifi connect, the channel is used to config
  * 	   whether it is a full channel scan(when channel is set to 0), or it will
  * 	   only scan one channel(do active scan on the configured channel).
  * @note  pscan_option set to PSCAN_FAST_SURVEY means do fast survey on the specified channel
  * 	   set to 0 means do normal scan on the specified channel or full channel.
  */
struct _rtw_network_info_t {
	struct _rtw_ssid_t					ssid;
	struct _rtw_mac_t					bssid;
	u32							security_type;	/*val: RTW_SECURITY_OPEN, RTW_SECURITY_WEP_PSK...*/
	unsigned char				*password;
	int 						password_len;
	int 						key_id;
	unsigned char				channel;		/**< set to 0 means full channel scan, set to other value means only scan on the specified channel */
	unsigned char				pscan_option;	/**< used when the specified channel is set, set to 0 for normal partial scan, set to PSCAN_FAST_SURVEY for fast survey*/
	unsigned char 				is_wps_trigger;	/**< connection triggered by WPS process**/
	struct _rtw_wpa_supp_connect_t	wpa_supp; /**< fullmac inic host used, user can ignore**/
	struct _rtw_mac_t		prev_bssid;
	u8							by_reconn; /*connection triggered by RTK auto reconnect process, user can ignore*/
	u8							rom_rsvd[4];
};

/**
 * @brief  The structure is retry_limit for auth/assoc/key exchange.
 * @note   all re_limits are limited to within 10
 */
struct rtw_conn_step_retries {
	u8 reauth_limit : 4;             /**< indicate retry limit of auth-open/shared key */
	u8 sae_reauth_limit : 4;         /**< indicate retry limit of sae auth */
	u8 reassoc_limit : 4;            /**< indicate retry limit of assoc */
	u8 eapol_key_rsend_limit : 4;    /**< indicate retry limit of 4way handshake */
};

/**********************************************************************************************
 *                                     wifi_status structures
 *********************************************************************************************/
/**
  * @brief  The structure is used to store the WIFI setting gotten from WIFI driver.
  * @note	size can't be changed
  */
struct _rtw_wifi_setting_t {
	u8					mode;   /**< the mode of current wlan interface, val: RTW_MODE_STA, RTW_MODE_AP, RTW_MODE_NAN */
	unsigned char 		ssid[33];   /**< the ssid of connected AP or softAP */
	unsigned char		bssid[6];   /**< the bssid of connected AP or softAP */
	unsigned char		channel;
	u32					security_type; /**< the security type of connected AP or softAP, val: RTW_SECURITY_OPEN, RTW_SECURITY_WEP_PSK...*/
	unsigned char 		password[RTW_MAX_PSK_LEN];   /**< the password of connected AP or softAP */
	unsigned char		key_idx;
	unsigned char		alg;		/**< encryption algorithm */
	unsigned int		auth_type;
	unsigned char		is_wps_trigger;	/**< connection triggered by WPS process**/
	unsigned int		rom_rsvd;
};

/**
  * @brief  The structure is used to describe the sw statistics
  */
struct _rtw_sw_statistics_t { /* software statistics for tx and rx*/
	unsigned int    max_skbbuf_used_number; /*!< max skb buffer used number       */
	unsigned int    skbbuf_used_number;     /*!< current used skbbuf number       */
};

/**
  * @brief  The structure is used to describe the phy statistics
  */
struct _rtw_phy_statistics_t {
	signed char	rssi;          /*!<average mixed rssi in 1 sec (for STA mode) */
	signed char	data_rssi;          /*!<average data rssi in 1 sec (for STA mode) */
	signed char	beacon_rssi;          /*!<average beacon rssi in 1 sec (for STA mode) */
	signed char	snr;          /*!< average snr in 1 sec (not include cck rate, for STA mode)*/
	unsigned int
	cca_clm; /*<channel loading measurement ratio by cca (the ratio of CCA = 1 in number of samples). driver do clm every 2 seconds, the value is the lastest result>*/
	unsigned int	edcca_clm; /*<channel loading measurement ratio by edcca (the ratio of EDCCA = 1 in number of samples). The value is also the lastest result>*/
	unsigned int	clm_channel; /*<channel corresponding to the latest clm result.>*/
	unsigned int	tx_retry;
	unsigned short	tx_drop;
	unsigned int	rx_drop;
	unsigned int	supported_max_rate;
};
/**********************************************************************************************
 *                                     softap structures
 *********************************************************************************************/
/**
  * @brief  The structure is used to describe the setting about SSID,
  *			security type, password and default channel, used to start AP mode.
  * @note  The data length of string pointed by ssid should not exceed 32,
  *        and the data length of string pointed by password should not exceed 64.
  */
struct _rtw_softap_info_t {
	struct _rtw_ssid_t		ssid;
	unsigned char		hidden_ssid;
	u32					security_type; /**< val: RTW_SECURITY_OPEN, RTW_SECURITY_WEP_PSK...*/
	unsigned char 		*password;
	unsigned char 		password_len;
	unsigned char		channel;
};

#ifndef CONFIG_FULLMAC
/**
  * @brief  The structure is used to describe the rtw_client_list.
  */
struct _rtw_client_list_t {
	unsigned int    count;         /**< Number of associated clients in the list    */
	struct _rtw_mac_t mac_list[MACID_HW_MAX_NUM - 2]; /**< max length array of MAC addresses */
	signed char rssi_list[MACID_HW_MAX_NUM - 2]; /**< max length array of client rssi */
	unsigned char macid_list[MACID_HW_MAX_NUM - 2]; /**< max length array of client macid */
};
#endif

/**
  * @brief  The structure is used to describe the cfg parameters used for channel switch announcement,
  */
struct _rtw_csa_parm_t {
	unsigned char new_chl;
	unsigned char chl_switch_cnt;
	unsigned char action_type;	/* 0: unicast csa action, 1: broadcast csa action, other values: disable transmit csa action */
	unsigned char bc_action_cnt; /* indicate the number of broadcast csa actions to send for each beacon interval. only valid when action_type = 1*/
	/** @brief user handle when softap switch channel by csa function
	  * @param[in] channel:  new channel
	  * @param[in] ret: val: RTW_ERROR, RTW_SUCCESS
	  */
	void (*callback)(unsigned char channel, s8 ret);
};

/**********************************************************************************************
 *                                     promisc structures
 *********************************************************************************************/
/**
*@brief The structure is format of promisc packets .
*/
struct rx_pkt_info {
	s8 recv_signal_power;
	u8 data_rate;/*val: MGN_1M, MGN_2M...*/
	u8 channel;
	u8 *buf;
	u32 len;
};

/**
 * @brief The enumeration lists rcr mode under promisc
 */
enum {
	RCR_ALL_PKT,  /**< receive all packets */
	RCR_AP_ALL     /**< receive all packtets send by connected ap */
};

/**
 * @brief The enumeration lists promisc callback return value
 */
enum {
	NEED_DRIVER_HANDLE,  /**< driver will continue process this pkt */
	BYPASS_DRIVER_HANDLE     /**< driver will bypass this pkt */
};

struct _promisc_para_t {
	/*! Receive all packets in the air or set some filtering conditions
		- RCR_ALL_PKT: receive all packets in the air
		- RCR_AP_ALL: receive all packtets send by connected AP*/
	u8 filter_mode;
	/** @brief user handle a packet\n
	  * @param[in] struct rx_pkt_info:  packet's detail information
	  * @return Should the driver continue pr8ucessing this packet after user has processed.
	  * 	- NEED_DRIVER_HANDLE: driver continue processing this packet, This setting is usually required when the STA remains connected
	  *     - BYPASS_DRIVER_HANDLE: driver drop this packet, This setting is usually used when STA does not need connect*/
	u8(*callback)(struct rx_pkt_info *pkt_info);
};

/**********************************************************************************************
 *                                     wpa_lite structures
 *********************************************************************************************/
/**
 * @brief The enumeration lists the type of pmksa operations.
 */
enum {
	PMKSA_SET = 0,
	PMKSA_DEL = 1,
	PMKSA_FLUSH = 2,
};

/**
 * @brief  The structure is pmksa ops.
 */
struct rtw_pmksa_ops_t {
	u8 ops_id;
	u8 wlan_idx;
	u8 pmkid[16];
	u8 mac_addr[6];
	u8 pmk[32];/*pmksa is maintained in NP when use wpa_lite*/
};

/**
 * @brief  The structure is crypt info.
 */
struct rtw_crypt_info {
	u8 pairwise;            /* indicate pairwise(1) key or group key(0) */
	u8 mac_addr[6];
	u8 wlan_idx;
	u16 key_len;
	u8 key[32];
	u8 key_idx;
	u8 driver_cipher;
	u8 transition_disable_exist;
	u8 transition_disable_bitmap;
	u8 device_id : 5;       /* tx_raw: flag for no linked peer, and will be converted to camid when force_cam_entry=1 */
	u8 force_cam_entry : 1; /* tx_raw must set force_cam_entry=1 */
	u8 rpt_mode : 1;
};

/**
 * @brief  The structure is status of wpa_4way.
 */
struct rtw_wpa_4way_status {
	u8 *mac_addr;             /**< mac addr of 4-way interactive peer device */
	u8 wlan_idx;              /**< index of wlan interface */
	u8 is_start : 1;          /**< start(1) or stop(0) of 4way/2way exchange */
	u8 is_grpkey_update : 1;  /**< indicate first key change(0) or update grp_key change(1) */
	u8 is_success : 1;        /**< result of 4way/2way exchange: 0-fail; 1-success */
};

struct wpa_sae_param_t {
	unsigned char 		peer_mac[6];
	unsigned char 		self_mac[6];
	u8					h2e;
	u8					sae_reauth_limit;
};

struct rtw_owe_param_t {
	u16 group;
	u8 pub_key[RTW_OWE_KEY_LEN];/*32(Temporarily support group 19 with 256 bit public key)*/
	u8 pub_key_len;
	u8 peer_mac[6];
};

struct rtw_kvr_param_t {
#if defined(CONFIG_IEEE80211V) || defined(CONFIG_IEEE80211K) || defined(CONFIG_IEEE80211R)
	u8 nb_active;
	u8 btm_active;
	unsigned char 		peer_mac[6];
	unsigned char 		self_mac[6];
#ifdef CONFIG_IEEE80211R
	u8 ft_active;
	u16 mdid;
	u8 ft_cap;
	u8 privacy;
	u32 grp_privacy;
	u8 ie[5 + MAX_WPA_IE_LEN + MAX_FT_IE_LEN]; /* 1.NP->AP: rsnie; 2.AP->NP: mdie+rsnie+ftie*/
	u32 ielen;
#endif
#endif
};
/**********************************************************************************************
 *                                     speaker structures
 *********************************************************************************************/
/**
 * @brief The enumeration lists the type of pmksa operations.
 */
enum {
	SPEAKER_SET_INIT = 0,
	SPEAKER_SET_LATCH_I2S_COUNT = 1,
	SPEAKER_SET_TSF_TIMER = 2,
};

union speaker_set {
	struct { /*SPEAKER_SET_INIT*/
		u8 mode;              /* 0 for slave, 1 for master */
		u8 nav_thresh;        /* unit 128us */
		u8 relay_en;          /* relay control */
	} init;
	struct { /*SPEAKER_SET_LATCH_I2S_COUNT*/
		u8 port;           /* 0 for select port 0's TSFT to trigger audio latch count, 1 for port 1 */
		u8 latch_period;      /* 0 for trigger audio latch period is 4.096ms, 1 for 8.192ms */
	} latch_i2s_count;
	struct { /*SPEAKER_SET_TSF_TIMER*/
		u64 tsft;           /* unit us */
		u8 port;           /* 0 for select port 0's TSFT to trigger audio latch count, 1 for port 1 */
	} tsf_timer;
};
/**********************************************************************************************
 *                                     other structures
 *********************************************************************************************/
#ifndef CONFIG_FULLMAC
/**
 * @brief  The structure is join block param.
 */
struct internal_join_block_param {
	void				*join_sema;
	unsigned int		join_timeout;
	unsigned char		block;
};

/**
 * @brief  The structure is used to describe net device
 */
struct net_device {
	void			*priv;		/* pointer to private data */
	unsigned char		dev_addr[6];	/* set during bootup */
	u8 iface_type;
};

/**
 * @brief  The structure is used to describe wlan info
 */
struct _Rltk_wlan_t {
	struct net_device	dev;		/* Binding wlan driver netdev */
	void			*skb;		/* pending Rx packet */
	unsigned int		tx_busy;
	unsigned int		rx_busy;
	unsigned char		enable;
	rtos_sema_t			netif_rx_sema;	/* prevent race condition on .skb in rltk_netif_rx() */
};
#endif

/**
 * @brief  parameters of rf_calibration_disable.
 */
#define DIS_DPK BIT(0)

/**
 * @brief  The enumeration is wl band type.
 */
enum {
	WL_BAND_2_4G = 0,   ///<2.4g band
	WL_BAND_5G,            ///<5g band
	WL_BAND_2_4G_5G_BOTH, ///<2.4g&5g band
	WL_BANDMAX  ///< max band
};

/**
  * @brief  The structure is power limit regu map.
  */
struct _pwr_lmt_regu_remap {
	unsigned char	domain_code;
	unsigned char	PwrLmtRegu_2g;	/**< Not distinguish 2.4G and 5G; just set PwrLmtRegu_2g */
	unsigned char	PwrLmtRegu_5g;
};

/**
  * @brief  The structure is used to describe the raw data description
  */
struct raw_frame_desc_t {
	unsigned char wlan_idx;      /**< index of wlan interface which will transmit */
	unsigned char device_id;     /**< index of peer device which as a rx role for receiving this pkt, and will be update when linked peer */
	unsigned char *buf;          /**< poninter of buf where raw data is stored*/
	unsigned short buf_len;      /**< the length of raw data*/
	unsigned char tx_rate;	/*val: MGN_1M, MGN_2M...*/
	unsigned char retry_limit;
	unsigned char ac_queue;      /**< 0/3 for BE, 1/2 for BK, 4/5 for VI, 6/7 for VO */
	unsigned char sgi : 1;       /**< 1 for enable data short */
	unsigned char agg_en : 1;    /**< aggregation of tx_raw frames. 1:enable; 0-disable */
};

/**
  * @brief  Old version raw data description, only used for driver internal send mgnt frames
  */
struct _raw_data_desc_t {
	unsigned char		wlan_idx;      /**< index of wlan interface which will transmit */
	unsigned char		*buf;          /**< poninter of buf where raw data is stored*/
	unsigned short		buf_len;      /**< the length of raw data*/
	unsigned short		flags;        /**< send options*/
};

/**
  * @brief  The structure is used to describe the cfg parameters used for csi report,
  * @note  The mac_addr if not specified, the default value must be 0.
  */
struct _rtw_csi_action_parm_t {
	unsigned char group_num;/*val: CSI_GROUP_NUM_1, CSI_GROUP_NUM_2...*/
	unsigned char accuracy;/*val: CSI_ACCU_1BYTE, CSI_ACCU_2BYTE*/
	unsigned char alg_opt;/*val: CSI_ALG_LS, CSI_ALG_SMOTHING*/
	unsigned char ch_opt;/*val: CSI_CH_LEGACY, CSI_CH_NON_LEGACY*/
	unsigned char csi_role; /* indicate csi operation role, val: CSI_OP_ROLE_TRX, CSI_OP_ROLE_TX, CSI_OP_ROLE_RX */
	unsigned char mode;/*val: CSI_MODE_NORMAL, CSI_MODE_NDP, CSI_MODE_RX_RESP*/
	unsigned char act;/*val: CSI_ACT_EN, CSI_ACT_CFG*/
	unsigned short trig_frame_mgnt; /* indicate management frame subtype of rx csi triggering frame for fetching csi, val: CSI_TRIG_ASSOCREQ...*/
	unsigned short trig_frame_ctrl; /* indicate control frame subtype of rx csi triggering frame for fetching csi, val: CSI_TRIG_TRIGGER..*/
	unsigned short trig_frame_data; /* indicate data frame subtype of rx csi triggering frame for fetching csi, val: CSI_TRIG_DATA*/
	unsigned char enable;
	unsigned char trig_period;
	unsigned char data_rate;
	unsigned char data_bw;
	unsigned char mac_addr[6];
	unsigned char multi_type;     /* 0-uc csi triggering frame; 1-bc csi triggering frame */
	unsigned char trig_flag;      /* indicate source of device for triggering csi[sta<->sta]: 4bits >> 1 ~ 15 */
};

/**
 * @brief  The enumeration is transmission type for wifi custom ie.
 */
enum {
	PROBE_REQ = BIT(0),  ///<probe request
	PROBE_RSP = BIT(1),  ///<probe response
	BEACON	  = BIT(2),     ///<beacon
	ASSOC_REQ = BIT(3), ///<assocation request
};

/**
 * @brief  The structure is used to set WIFI custom ie list,
 *
 */
struct custom_ie {
	u8 *ie;/*format: [element ID (1byte) | length (1byte) | content (length bytes) ]*/
	u8 type;/*val: PROBE_REQ, PROBE_RSP...*/
};

/**
 * @brief  The structure is used to describe channel plan and country code
 */
struct country_code_table_t {
	char char2[2]; /* country code */
	u8 channel_plan; /* channel plan code */
	u8 pwr_lmt; /* tx power limit index */
};

struct rtw_tx_power_ctl_info_t {
	s8	tx_pwr_force; /* Currently user can specify tx power for all rate. unit 0.25dbm*/
	u8	b_tx_pwr_force_enbale : 1;
};

#ifndef CONFIG_FULLMAC
extern struct _Rltk_wlan_t rltk_wlan_info[NET_IF_NUM];
#define netdev_priv(dev)		dev->priv
#define rtw_is_netdev_enable(idx)	(rltk_wlan_info[idx].enable)
#define rtw_get_netdev(idx)		(&(rltk_wlan_info[idx].dev))
#endif
extern  struct wifi_user_conf wifi_user_config;
extern struct _rtw_wifi_setting_t wifi_setting[2];
/**
* @}
*/

/**********************************************************************************************
 *                                    user_configure struct
 *********************************************************************************************/
/** @defgroup WIFI_USER_CONFIG
*  @brief	WIFI_USER_CONFIG module
*  @{*/
#ifdef CONFIG_SINGLE_CORE_WIFI

/**
  * @brief  The structure is used to describe the wifi user configuration, can be configured in ameba_wificfg.c
  */
struct wifi_user_conf {
	/*!	This effects EDCCA threshold, wifi won't TX if detected energy exceeds threshold\n
		RTW_EDCCA_NORM: Adjust EDCCA threshold dynamically;\n
		RTW_EDCCA_ADAPT: For ESTI or SRRC;\n
		RTW_EDCCA_CS: For japan;\n
		RTW_EDCCA_DISABLE: Ingore EDCCA */
	unsigned char rtw_edcca_mode;

	/*!	For power limit, see Ameba Wi-Fi TX power and Country code Setup Guideline.pdf\n
		0: disable, 1: enable, 2: Depend on efuse(flash) */
	unsigned char rtw_tx_pwr_lmt_enable;

	/*!	For power by rate, see Ameba Wi-Fi TX power and Country code Setup Guideline.pdf\n
		0: disable, 1: enable, 2: Depend on efuse(flash) */
	unsigned char rtw_tx_pwr_by_rate;

	/*!	Enabled during TRP TIS certification */
	unsigned char rtw_trp_tis_cert_en;

	/*!	Force wpa mode\n
		WPA_AUTO_MODE: auto mode, follow AP;\n
		WPA_ONLY_MODE: wpa only;\n
		WPA2_ONLY_MODE: wpa2 only;\n
		WPA3_ONLY_MODE: wpa3 only;\n
		WPA_WPA2_MIXED_MODE: wpa and wpa2 mixed;\n
		WPA2_WPA3_MIXED_MODE: wpa2 and wpa3 mixed*/
	unsigned char wifi_wpa_mode_force;

	/*!	TDMA DIG affects the range of RX, when enabled, both near and distant devices can be received\n
		0:tdma dig off, 1:tdma dig on */
	unsigned char tdma_dig_enable;

	/*!	Antdiv mode\n
		RTW_ANTDIV_AUTO: auto antdiv;\n
		RTW_ANTDIV_FIX_MAIN: fix main ant;\n
		RTW_ANTDIV_FIX_AUX: fix aux ant;\n
		RTW_ANTDIV_DISABLE: antdiv disable */
	unsigned char antdiv_mode;

	/*!	The maximum number of STAs connected to the softap should not exceed AP_STA_NUM */
	unsigned char ap_sta_num;

	/*!	IPS(Inactive power save), If disconnected for more than 2 seconds, WIFI will be powered off*/
	unsigned char ips_enable;

	/*!	Power save status\n
		IPS_WIFI_OFF: The WIFI is powered off during the IPS;\n
		IPS_WIFI_PG: The WIFI enters the PG state during the IPS, and therefore it exits the IPS faster. Only dplus support this status */
	unsigned char ips_level;

	/*!	The driver does not enter the IPS due to 2s disconnection. Instead, API wifi_set_ips_internal controls the IPS\n
		0: wifi_set_ips_internal control ips enable/disable, 1: control ips enter/leave */
	unsigned char ips_ctrl_by_usr;

	/*!	LPS(leisure power save), After connection, with low traffic, part of WIFI can be powered off and woken up upon packet interaction\n
		0: disable power save when wifi connected, 1: enable */
	unsigned char lps_enable;

	/*!	Power management mode\n
		PS_MODE_LEGACY: During TBTT, wake up to receive beacon; otherwise, WIFI remains in power-save mode;\n
		PS_MODE_UAPSD_WMM: not support right now */
	unsigned char lps_mode;

	/*!	In LPS, the sta wakes up every legacy_ps_listen_interval* 102.4ms to receive beacon*/
	unsigned char legacy_ps_listen_interval;

	/*!	0: NO_LIMIT, 1: TWO_MSDU, 2: FOUR_MSDU, 3: SIX_MSDU */
	unsigned char uapsd_max_sp_len;

	/*!	BIT0: AC_VO UAPSD, BIT1: AC_VI UAPSD, BIT2: AC_BK UAPSD, BIT3: AC_BE UAPSD */
	unsigned char uapsd_ac_enable;

	/*!	0: Disable ampdu rx, 1: Enable ampdu rx */
	unsigned char ampdu_rx_enable;

	/*!	0: Disable ampdu tx, 1: Enable ampdu tx */
	unsigned char ampdu_tx_enable;

	/*!	0: If the pkt's destination address doesn't match, it won't be dropped, 1: If the pkt's destination address doesn't match, it will be dropped. */
	unsigned char bCheckDestAddress;

	/*!	The ap_compatibilty_enabled is used to configure the wlan settings, each bit controls one aspect\n
		bit 0: (0: follow 802.11 spec, do not issue deauth, 1(default): issue deauth in 1st REAUTH_TO to be compatible with ap);\n
		bit 1: (0: do not check beacon info to connect with AP with multiple SSID, 1(default): check beacon info);\n
		bit 2: (0(default): do not issue deauth at start of auth, 1: issue deauth at start of auth);\n
		bit 3: (0: do not switch WEP auth algo unless WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG, 1(default): switch WEP auth algo from shared key to open system in 1st REAUTH_TO);\n
		other bits: reserved */
	unsigned char ap_compatibilty_enabled;

	/*!	0: API wifi_set_channel does not trigger RFK;\n
		1: API wifi_set_channel triggers RFK */
	unsigned char set_channel_api_do_rfk;

	/*!	RF calibration is triggered during WIFI initialization and channel switch to calibrate TRX to optimal performance,\n
		but it takes a long time (around 100ms), so some applications can sacrifice performance so that WIFI initializes and switches faster.\n
		Bit0: DIS_DPK;\n
		Other bits: reserved*/
	u16 rf_calibration_disable;

	/*! The maximum number of roaming attempts triggered by BTM*/
	unsigned char max_roaming_times;

	/*!	AP periodically sends null pkt to check whether the STA is offline, not support right now*/
	unsigned char ap_polling_sta;

	/*!	Refer to ameba_wifi_country_code_table_usrcfg.c, e.g. China: country_code[0] = 'C', country_code[1] = 'N'.\n
		Specical value: country_code[0]~[1] = 0x0000: follow efuse; country_code[0]='0', country_code[1]='0' : WORLDWIDE */
	s8 country_code[2];

	/*!	Bandwidth 40MHz, some IC hardware does not support*/
	unsigned char bw_40_enable;

	/*!	Refe to 802.11d spec, obtain the country code information from beacon, and set the pwr limit and channel plan*/
	unsigned char rtw_802_11d_en;

	/*!	When disconnection, STA automatically reconnects, and auto_reconnect_count is the number of attempts*/
	unsigned char auto_reconnect_count;

	/*!	auto_reconnect_interval is Automatic reconnection interval, unit s*/
	unsigned char auto_reconnect_interval;

	/*!	no_beacon_disconnect_time is the disconnect time when no beacon occurs, unit 2s*/
	unsigned char no_beacon_disconnect_time;

	/*!	skb_num_np is wifi driver's trx buffer number, each buffer occupies about 1.8K bytes of heap, a little difference between different chips.\n
		These buffer are used for all traffics except tx data in INIC mode, and all traffics in single core mode.\n
		For higher throughput or a large number of STAs connected to softap, skb_num_np can be increased.\n
		Minimum: 7 (3 for Ameba lite). Default: 10*/
	int skb_num_np;

	/*!	These buffer are used for tx data packtes in INIC mode, not used in single core mode.\n
		For higher throughput or a large number of STAs connected to softap, skb_num_ap can be increased */
	int skb_num_ap;

	/*!	Specify the trx buffer size of each skb.\n
		Cache size(32 for amebadplus&amebalite and 64 for amebasmart)alignment will be applied to the input size.\n
		0: use default size*/
	unsigned int skb_buf_size;

	/*!	Every data tx is forced to start with cts2self */
	unsigned char force_cts2self;

	/*!	Multi Channel Concurrent mode, STA and Softap can work on different channels via TDMA, not support right now*/
	unsigned char en_mcc;

	unsigned char tx_shortcut_enable;

	unsigned char rx_shortcut_enable;

	/* for Concurrent Mode */
	/*!	0: STA or SoftAP only at any time, The MAC address of TA or Softap is the MAC address of chip;\n
		1: STA and SoftAP may run at the same time, Softap's mac address depends on softap_addr_offset_idx */
	unsigned char concurrent_enabled;

	/*!	It is valid only when concurrent_enabled =1. The range is 1~5. The lowest bit of mac[0] is 1, which represents the multicast address, so skip mac[0].\n
		e.g. softap_addr_offset_idx = 1, chip's mac = 00:e0:4c:01:02:03, softap's mac = 00:e1:4c:01:02:03;\n
		e.g. softap_addr_offset_idx = 5, chip's mac = 00:e0:4c:01:02:03, softap's mac = 00:e0:4c:01:02:04*/
	unsigned char softap_addr_offset_idx;

	/*!	The number of ampdu that Recipient claims to Originator for RX, it can be any value less than 64.\n
		skb_num_np needs to be adjusted simultaneously*/
	unsigned char rx_ampdu_num;

	/*!	Linux Fullmac architecture, ignore in RTOS*/
	unsigned char cfg80211;

	/*!	WPS */
	unsigned char wps_retry_count;

	/*!	in ms */
	unsigned int wps_retry_interval;

	/*!	For wifi speaker configuration\n
		BIT0: main switch, BIT1: enable tsf interrupt, BIT2: enable audio tsf */
	unsigned char wifi_speaker_feature;

	/*STA mode will periodically send null packet to AP to keepalive, unit: second */
	unsigned char keepalive_interval;

	/*Automatic channel selection*/
	unsigned char acs_en;
};
/** @} */
#else
struct wifi_user_conf {
	/*!	This effects EDCCA threshold, wifi won't TX if detected energy exceeds threshold.
		- \b RTW_EDCCA_NORM: Adjust EDCCA threshold dynamically;
		- \b RTW_EDCCA_ADAPT: For ESTI or SRRC;
		- \b RTW_EDCCA_CS: For japan;
		- \b RTW_EDCCA_DISABLE: Ingore EDCCA. */
	u8 rtw_edcca_mode;

	/*!	For power limit, see Ameba Wi-Fi TX power and Country code Setup Guideline.pdf.\n
		0: Disable, 1: Enable, 2: Depend on efuse(flash). */
	u8 rtw_tx_pwr_lmt_enable;

	/*!	For power by rate, see Ameba Wi-Fi TX power and Country code Setup Guideline.pdf.\n
		0: Disable, 1: Enable, 2: Depend on efuse(flash). */
	u8 rtw_tx_pwr_by_rate;

	/*!	Enabled during TRP TIS certification. */
	u8 rtw_trp_tis_cert_en;

	/*!	Force wpa mode:
		- \b RTW_WPA_AUTO_MODE: Auto mode, follow AP;
		- \b RTW_WPA_ONLY_MODE: Wpa only;
		- \b RTW_WPA2_ONLY_MODE: Wpa2 only;
		- \b RTW_WPA3_ONLY_MODE: Wpa3 only;
		- \b RTW_WPA_WPA2_MIXED_MODE: Wpa and wpa2 mixed;
		- \b RTW_WPA2_WPA3_MIXED_MODE: Wpa2 and wpa3 mixed.*/
	u8 wifi_wpa_mode_force;

	/*!	TDMA DIG affects the range of RX, when enabled, both near and distant devices can be received.\n
		0: TDMA DIG off, 1: TDMA DIG on. */
	u8 tdma_dig_enable;

	/*!	Antdiv mode:
		- \b RTW_ANTDIV_AUTO: Auto antdiv;
		- \b RTW_ANTDIV_FIX_MAIN: Fix main ant;
		- \b RTW_ANTDIV_FIX_AUX: Fix aux ant;
		- \b RTW_ANTDIV_DISABLE: Antdiv disable. */
	u8 antdiv_mode;

	/*!	The maximum number of STAs connected to the softap should not exceed the num specified in notes of func wifi_set_user_config(). */
	u8 ap_sta_num;

	/*!	IPS(Inactive power save), If disconnected for more than 2 seconds, WIFI will be powered off. */
	u8 ips_enable;

	/*!	Power save status:
		- \b RTW_IPS_WIFI_OFF: The WIFI is powered off during the IPS;
		- \b RTW_IPS_WIFI_PG: The WIFI enters the PG state during the IPS, and therefore it exits the IPS faster. Only dplus support this status. */
	u8 ips_level;

	/*!	The driver does not enter the IPS due to 2s disconnection. Instead, API wifi_set_ips_internal() controls the IPS.\n
		0: API wifi_set_ips_internal() control ips enable/disable, 1: Control ips enter/leave. */
	u8 ips_ctrl_by_usr;

	/*!	LPS(leisure power save), After connection, with low traffic, part of WIFI can be powered off and woken up upon packet interaction.\n
		0: Disable power save when wifi connected, 1: Enable. */
	u8 lps_enable;

	/*!	Power management mode:
		- \b RTW_PS_MODE_LEGACY: During TBTT, wake up to receive beacon; otherwise, WIFI remains in power-save mode;\n
		- \b RTW_PS_MODE_UAPSD_WMM: Not support right now. */
	u8 lps_mode;

	/*!	In LPS, the sta wakes up every legacy_ps_listen_interval* 102.4ms to receive beacon.*/
	u8 legacy_ps_listen_interval;

	/*! Enable or disable rx broadcast in tickless wowlan mode,
		1 means disable rx broadcast in tickless wowlan mode, 0 means enable(default) rx broadcast in tickless wowlan mode.*/
	u8 wowlan_rx_bcmc_dis;

	/*!	0: RTW_UAPSD_NO_LIMIT, 1: RTW_UAPSD_TWO_MSDU, 2: RTW_UAPSD_FOUR_MSDU, 3: RTW_UAPSD_SIX_MSDU. */
	u8 uapsd_max_sp_len;

	/*!	BIT0: AC_VO UAPSD, BIT1: AC_VI UAPSD, BIT2: AC_BK UAPSD, BIT3: AC_BE UAPSD. */
	u8 uapsd_ac_enable;

	/*!	0: Disable ampdu rx, 1: Enable ampdu rx. */
	u8 ampdu_rx_enable;

	/*!	0: Disable ampdu tx, 1: Enable ampdu tx. */
	u8 ampdu_tx_enable;

	/*!	0: If the pkt's destination address doesn't match, it won't be dropped, 1: If the pkt's destination address doesn't match, it will be dropped. */
	u8 check_dest_address_en;

	/*!	The ap_compatibilty_enabled is used to configure the wlan settings, each bit controls one aspect.
		- <b>bit 0:</b> 0: follow 802.11 spec, do not issue deauth, 1(default): issue deauth in 1st REAUTH_TO to be compatible with ap;
		- <b>bit 1:</b> 0: do not check beacon info to connect with AP with multiple SSID, 1(default): check beacon info;
		- <b>bit 2:</b> 0(default): do not issue deauth at start of auth, 1: issue deauth at start of auth;
		- <b>bit 3:</b> 0: do not switch WEP auth algo unless WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG, 1(default): switch WEP auth algo from shared key to open system in 1st REAUTH_TO;
		- <b>other bits:</b> reserved */
	u8 ap_compatibilty_enabled;

	/*!	0: API wifi_set_channel() does not trigger RFK;
		1: API wifi_set_channel() triggers RFK. */
	u8 set_channel_api_do_rfk;

	/*!	0: Do not limit the peak current of DPD;
		1: Limit the peak current of DPD. */
	u8 dpk_peak_limit;

	/*!	RF calibration is triggered during WIFI initialization and channel switch to calibrate TRX to optimal performance,\n
		but it takes a long time (around 100ms), so some applications can sacrifice performance so that WIFI initializes and switches faster.\n
		- \b Bit0: RTW_RFK_DIS_DPK;
		- <b>Other bits:</b> reserved.*/
	u16 rf_calibration_disable;

	/*! The maximum number of roaming attempts triggered by BTM.*/
	u8 max_roaming_times;

	/*!	AP periodically sends null pkt to check whether the STA is offline, not support right now.*/
	u8 ap_polling_sta;

	/*!	Refer to ameba_wifi_country_code_table_usrcfg.c, e.g. China: country_code[0] = 'C', country_code[1] = 'N'.\n
		Specical value: country_code[0]~[1] = 0x0000: follow efuse; country_code[0]='0', country_code[1]='0' : WORLDWIDE. */
	u8 country_code[2];

	/*!	Bandwidth 40MHz, some IC hardware does not support.*/
	u8 bw_40_enable;

	/*!	Refe to 802.11d spec, obtain the country code information from beacon, and set the pwr limit and channel plan.*/
	u8 rtw_802_11d_en;

	/*!	When booting the STA, it automatically reconnects to previously connected AP*/
	u8 fast_reconnect_en;

	/*!	When disconnection, STA automatically reconnects.*/
	u8 auto_reconnect_en;

	/*!	When disconnection, STA automatically reconnects, and auto_reconnect_count is the number of attempts.
		Specical value: 0xff means infinite retry count*/
	u8 auto_reconnect_count;

	/*!	auto_reconnect_interval is Automatic reconnection interval, unit s.*/
	u8 auto_reconnect_interval;

	/*!	no_beacon_disconnect_time is the disconnect time when no beacon occurs, unit 2s.*/
	u8 no_beacon_disconnect_time;

	/*!	skb_num_np is wifi driver's trx buffer number, each buffer occupies about 1.8K bytes of heap, a little difference between different chips.\n
		These buffer are used for all traffics except tx data in INIC mode, and all traffics in single core mode.\n
		For higher throughput or a large number of STAs connected to softap, skb_num_np can be increased.\n
		Minimum: 7 (3 for Ameba lite). Default: 10.*/
	s32 skb_num_np;

	/*!	These buffer are used for tx data packtes in INIC mode, not used in single core mode.\n
		For higher throughput or a large number of STAs connected to softap, skb_num_ap can be increased. */
	s32 skb_num_ap;

	/*!	Specify the trx buffer size of each skb.\n
		Cache size(32 for amebadplus&amebalite and 64 for amebasmart)alignment will be applied to the input size.\n
		Considering the length field of L-SIG is 12 bits, the max PSDU size is 4095 bytes, so skb_buf_size is suggested not exceed 4k.\n
		0: use default size. */
	u32 skb_buf_size;

	/*!	Every data tx is forced to start with cts2self. */
	u8 force_cts2self;

	/*!	Multi Channel Concurrent mode, STA and Softap can work on different channels via TDMA.
		@note Mcc performance has limitations, please contact Realtek before use to clarify your requirements. */
	u8 en_mcc;

	u8 tx_shortcut_enable;

	u8 rx_shortcut_enable;

	/*! For concurrent mode:
	    - \b 0: STA or SoftAP only at any time, The MAC address of TA or Softap is the MAC address of chip;
		- \b 1: STA and SoftAP may run at the same time, Softap's mac address depends on softap_addr_offset_idx. */
	u8 concurrent_enabled;

	/*!	It is valid only when concurrent_enabled =1. The range is 0~5.The lowest bit of mac[0] is 1, which represents the multicast address,
		therefore, the mac[0] of softap is incremented by 2, others is incremented by 1.
		- e.g. softap_addr_offset_idx = 0, chip's mac = 00:e0:4c:01:02:03, softap's mac = 02:e0:4c:01:02:03;
		- e.g. softap_addr_offset_idx = 1, chip's mac = 00:e0:4c:01:02:03, softap's mac = 00:e1:4c:01:02:03;
		- e.g. softap_addr_offset_idx = 5, chip's mac = 00:e0:4c:01:02:03, softap's mac = 00:e0:4c:01:02:04.*/
	u8 softap_addr_offset_idx;

	/*!	The number of ampdu that Recipient claims to Originator for RX, it can be any value less than 64.\n
		skb_num_np needs to be adjusted simultaneously.*/
	u8 rx_ampdu_num;

	/*!	Linux Fullmac architecture, ignore in RTOS.*/
	u8 cfg80211;

	/*!	WPS. */
	u8 wps_retry_count;

	/*!	Unit: ms. */
	u32 wps_retry_interval;

	/*!	For wifi speaker configuration.\n
		BIT0: Main switch, BIT1: Enable tsf interrupt, BIT2: Enable audio tsf. */
	u8 wifi_speaker_feature;

	/*! STA mode will periodically send null packet to AP to keepalive, unit: second. */
	u8 keepalive_interval;

	/*! Automatic channel selection.*/
	u8 acs_en;

	/*! 0: Disable R-mesh function, 1: Enable R-mesh function.*/
	u8 wtn_en;

	/*! R-mesh AP strong RSSI thresh, when AP rssi larger than this thresh, will try to switch to AP.*/
	s8 wtn_strong_rssi_thresh;

	/*! R-mesh father refresh timeout, when not receive beacon from father for this timeout, will switch to other node, unit: millisecond.*/
	u16 wtn_father_refresh_timeout;

	/*! R-mesh child refresh timeout, when not receive beacon from child for this timeout, will delete this child, unit: millisecond.*/
	u16 wtn_child_refresh_timeout;

	/*! 0: Disable R-mesh NAT feature, 1: Enable R-mesh NAT feature.*/
	u8 wtn_rnat_en;

	/*! 0: Determine whether to become RNAT node based on the rssi to AP, 1: Become RNAT node regardless of the rssi to AP.*/
	u8 wtn_fixed_rnat_node;

	/*! 0: This device can connect to R-Mesh group or R-NAT group, 1: this device will only connect to R-NAT group.*/
	u8 wtn_connect_only_to_rnat;

	/*! Max node number in R-mesh network, this is used for decide each node's beacon window.*/
	u16 wtn_max_node_num;
};
#endif
/**********************************************************************************************
 *                                     Function Declarations
 *********************************************************************************************/
/** @defgroup WIFI_Exported_Functions WIFI Exported Functions
  * @{
  */
/** @defgroup Basic_Functions
  * @{
  */
/**
 * @brief  Enable Wi-Fi.
 * - Bring the Wireless interface "Up".
 * @param[in]  mode: Should always set to RTW_MODE_STA
 * @return  RTW_SUCCESS: if the WiFi chip initialized successfully.
 * @return  RTW_ERROR: if the WiFi chip initialization failed.
 * @note  Call wifi_start_ap afther this API if want to use AP mode
 */
int wifi_on(u8 mode);

/**
 * @brief  Check if the specified wlan interface  is running.
 * @param[in]  wlan_idx: can be set as STA_WLAN_INDEX or SOFTAP_WLAN_INDEX.
 * @return  If the function succeeds, the return value is 1.
 * 	Otherwise, return 0.
 * @note  For STA mode, use STA_WLAN_INDEX
 * 	For AP mode, use SOFTAP_WLAN_INDEX
 */
int wifi_is_running(unsigned char wlan_idx);

/**
 * @brief  Join a Wi-Fi network.
 * 	Scan for, associate and authenticate with a Wi-Fi network.
 * @param[in]  connect_param: the pointer of a struct which store the connection
 * 	info, including ssid, bssid, password, etc, for details, please refer to struct
 * 	struct _rtw_network_info_t in wifi_conf.h
 * @param[in]  block: if block is set to 1, it means synchronized wifi connect, and this
* 	API will return until connect is finished; if block is set to 0, it means asynchronized
* 	wifi connect, and this API will return immediately.
 * @return  RTW_SUCCESS: Join successfully for synchronized wifi connect,
 *  or connect cmd is set successfully for asynchronized wifi connect.
 * @return  RTW_ERROR: An error occurred.
 * @return  RTW_BUSY: Wifi connect or scan is ongoing.
 * @return  RTW_NOMEM: Malloc fail during wifi connect.
 * @return  RTW_TIMEOUT: More than RTW_JOIN_TIMEOUT(~70s) without successful connection.
 * @return  RTW_CONNECT_INVALID_KEY: Password format wrong.
 * @return  RTW_CONNECT_SCAN_FAIL: Scan fail.
 * @return  RTW_CONNECT_AUTH_FAIL: Auth fail.
 * @return  RTW_CONNECT_AUTH_PASSWORD_WRONG: Password error causing auth failure, not entirely accurate.
 * @return  RTW_CONNECT_ASSOC_FAIL: Assoc fail.
 * @return  RTW_CONNECT_4WAY_HANDSHAKE_FAIL: 4 way handshake fail.
 * @return  RTW_CONNECT_4WAY_PASSWORD_WRONG: Password error causing 4 way handshake failure,not entirely accurate.
 * @note  Please make sure the Wi-Fi is enabled before invoking this function.
 * 	(@ref wifi_on())
 * @note  if bssid in connect_param is set, then bssid will be used for connect, otherwise ssid
 * 	is used for connect.
 */
int wifi_connect(struct _rtw_network_info_t *connect_param, unsigned char block);

/**
 * @brief  Disassociates from current Wi-Fi network.
 * @param  None
 * @return  RTW_SUCCESS: On successful disassociation from the AP.
 * @return  RTW_ERROR: If an error occurred.
 */
int wifi_disconnect(void);

/**
 * @brief  get join status during wifi connectection
 * @param  None
 * @return RTW_JOINSTATUS_UNKNOWN: unknown
 * @return RTW_JOINSTATUS_STARTING:	starting phase
 * @return RTW_JOINSTATUS_SCANNING: scanning phase
 * @return RTW_JOINSTATUS_AUTHENTICATING: authenticating phase
 * @return RTW_JOINSTATUS_AUTHENTICATED: authenticated phase
 * @return RTW_JOINSTATUS_ASSOCIATING: associating phase
 * @return RTW_JOINSTATUS_ASSOCIATED: associated phase
 * @return RTW_JOINSTATUS_4WAY_HANDSHAKING: 4 way handshaking phase
 * @return RTW_JOINSTATUS_4WAY_HANDSHAKE_DONE: 4 way handshake done phase
 * @return RTW_JOINSTATUS_SUCCESS: join success
 * @return RTW_JOINSTATUS_FAIL:	join fail
 * @return RTW_JOINSTATUS_DISCONNECT: disconnect
 */
u8 wifi_get_join_status(void);

/**
 * @brief  Initiate a scan to search for 802.11 networks.
  * Synchronized scan and asynchronized scan can be confgiured by the input param block.
  * For asynchronized scan, there are two different ways about how the scan result will be reported.
  * The first way is that when scan is done ,the total number of scanned APs will be reported through
  * scan_user_callback, and the detailed scanned AP infos can be get by calling wifi_get_scan_records,
  * so in this way, scan_user_callback need to be registered in scan_param.
  * The second way is that every time a AP is scanned, this AP info will be directly reported by
  * scan_report_each_mode_user_callback, and when scan is done, scan_report_each_mode_user_callback will
  * report a NULL pointer for notification. So in this way, scan_report_each_mode_user_callback need to
  * be registered in scan_param, and RTW_SCAN_REPORT_EACH need to be set in scan_param->options.Also in
  * this mode, scan_user_callback is no need to be registered.
 * @param[in]  scan_param: refer to struct struct _rtw_scan_param_t in wifi_conf.h.
 * @param[in]  block: If set to 1, it's synchronized scan and this API will return
 * 	after scan is done. If set to 0, it's asynchronized scan and this API will return
 * 	immediately.
 * @return  RTW_SUCCESS or RTW_ERROR for asynchronized scan, return RTW_ERROR or
 * 	scanned AP number for Synchronized scan.
 * @note  If this API is called, the scanned APs are stored in WiFi driver dynamic
 * 	allocated memory, for synchronized scan or asynchronized scan which not use RTW_SCAN_REPORT_EACH,
 * 	these memory will be freed when wifi_get_scan_records is called.
 */
int wifi_scan_networks(struct _rtw_scan_param_t *scan_param, unsigned char block);

/**
 * @brief  Trigger Wi-Fi driver to start an infrastructure Wi-Fi network.
 * @param[in]  softAP_config:the pointer of a struct which store the softAP
 * 	configuration, please refer to struct struct _rtw_softap_info_t in wifi_conf.h
 * @warning  If a STA interface is active when this function is called,
 * 	the softAP will start on the same channel as the STA.
 * 	It will NOT use the channel provided!
 * @return  RTW_SUCCESS: If successfully creates an AP.
 * @return  RTW_ERROR: If an error occurred.
 * @note  if hidden_ssid in softAP_config is set to 1, then this softAP will start
 * 	with hidden ssid.
 * @note  Please make sure the Wi-Fi is enabled before invoking this function.
 * 	(@ref wifi_on())
 */
int wifi_start_ap(struct _rtw_softap_info_t *softAP_config);

/**
 * @brief  Disable Wi-Fi interface-2.
 * @param  None
 * @return  RTW_SUCCESS: deinit success,
 * 	wifi mode is changed to RTW_MODE_STA.
 * @return  RTW_ERROR: otherwise.
 */
int wifi_stop_ap(void);
/**
 * @brief  Enable Wi-Fi interface-2.
 * @param  None
 * @return  RTW_SUCCESS: success,
 * 	wifi open RTW_MODE_AP .
 * @return  RTW_ERROR: otherwise.
 */
int _wifi_on_ap(void);
/**
 * @brief  Disable Wi-Fi interface-2.
 * @param  None
 * @return  RTW_SUCCESS: close ap mode,
 * @return  RTW_ERROR: otherwise.
 */
int _wifi_off_ap(void);
/**
* @}
*/

/** @} */

/** @} */
#ifdef __cplusplus
}
#endif

#endif // __WIFI_API_H


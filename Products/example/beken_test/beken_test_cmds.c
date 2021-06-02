/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <aos/aos.h>
#include <aos/yloop.h>
#include "netmgr.h"
#include "iot_export.h"
#include "iot_import.h"
#include "aos/kv.h"

#include <k_api.h>

#include <hal/wifi.h>

#include "app_entry.h"
#include "beken_test_cmds.h"
#include "ble_api.h"
#include "beken_test_ble.h"
#include "ping.h"
#include "wlan_ui_pub.h"
///#include "sys_config.h"

static void handle_sta_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	hal_wifi_init_type_t wifi_type;

	if (argc < 2) {
		aos_cli_printf("sta <ssid> [password]\r\n");
		return;
	}
	
	memset(&wifi_type, 0, sizeof(wifi_type));
	wifi_type.wifi_mode = STATION;
	strcpy(wifi_type.wifi_ssid, argv[1]);
	if (argc == 2) {
		strcpy(wifi_type.wifi_key, "1");
	} else {
		strcpy(wifi_type.wifi_key, argv[2]);
	}
	wifi_type.dhcp_mode = DHCP_CLIENT;

	hal_wifi_start(NULL, &wifi_type);
}

static void handle_softap_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	hal_wifi_init_type_t wifi_type;
	char default_ip[] = "192.168.1.1";
	char default_gateway[] = "192.168.1.1";
	char default_netmask[] = "255.255.255.0";

	if (argc < 2) {
		aos_cli_printf("softap <ssid> [password]\r\n");
		return;
	}

	memset(&wifi_type, 0, sizeof(wifi_type));
	wifi_type.wifi_mode = SOFT_AP;
	strcpy(wifi_type.wifi_ssid, argv[1]);
	if (argc == 2) {
		strcpy(wifi_type.wifi_key, "1");
	} else {
		strcpy(wifi_type.wifi_key, argv[2]);
	}
	strcpy(wifi_type.local_ip_addr, default_ip);
	strcpy(wifi_type.gateway_ip_addr, default_gateway);
	strcpy(wifi_type.net_mask, default_netmask);
	strcpy(wifi_type.dns_server_ip_addr, default_ip);
	wifi_type.dhcp_mode = DHCP_SERVER;

	hal_wifi_start(NULL, &wifi_type);
}

static int wifi_scan_result_hdl(const char ssid[MAX_SSID_LEN],
								      const uint8_t bssid[ETH_ALEN],
								      enum NETMGR_AWSS_AUTH_TYPE auth,
								      enum NETMGR_AWSS_ENC_TYPE encry,
								      uint8_t channel, signed char rssi,
								      int is_last_ap)
{
	aos_cli_printf("ssid:%s, bssid:%02x:%02x:%02x:%02x:%02x:%02x, auth:%d, channel:%d, rssi:%d\r\n",
		ssid, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], auth, channel, rssi);
	return 0;
}

static void handle_scan_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	netmgr_register_wifi_scan_result_callback(wifi_scan_result_hdl);
	hal_wifi_start_scan_adv(NULL);
}

static void handle_ps_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	uint8_t dtim;

	if (argc < 3) {
		aos_cli_printf("ps [rfdtim|mcudtim] [0|1]\r\n");
		return;
	}

	if (!strcmp(argv[1], "rfdtim")) {
		dtim = atoi(argv[2]);
		if (dtim == 1) {
			aos_cli_printf("enable rf ditm.\r\n");
			hal_wifi_enter_powersave(NULL, 0);
		} else if (dtim == 0) {
			aos_cli_printf("disable rf dtim.\r\n");
			hal_wifi_exit_powersave(NULL);
		} else {
			aos_cli_printf("ps rfdtim %d error.\r\n", dtim);;
		}
	} else if (!strcmp(argv[1], "mcudtim")) {
		dtim = atoi(argv[2]);
		if (dtim == 1) {
			LOG("enable mcu dtim.\r\n");
			bk_wlan_mcu_ps_mode_enable();
		} else if (dtim == 0) {
			LOG("disable mcu dtim.\r\n");
			bk_wlan_mcu_ps_mode_disable();
		} else {
			aos_cli_printf("ps mcudtim %d error.\r\n", dtim);
		}
	} else {
		aos_cli_printf("ps [rfdtim|mcudtim] [0|1]\r\n");
	}
}

static void handle_deepsleep_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	uint32_t gpio_index_map;
	uint32_t gpio_edge_map;	
	uint32_t gpio_last_index_map;
	uint32_t gpio_last_edge_map;
	uint32_t sleep_time;
	uint32_t wake_up_way;
	uint32_t gpio_stay_lo_map;
	uint32_t gpio_stay_hi_map;

	if (argc != 9) {
		aos_cli_printf("deep_sleep [gpio_index_map] [gpio_edge_map] [gpio_last_index_map] [gpio_last_edge_map]");
		aos_cli_printf("           [sleep_time] [wake_up_way] [gpio_stay_lo_map] [gpio_stay_hi_map]");
		return;
	}

	gpio_index_map      = atoi(argv[1]);
	gpio_edge_map       = atoi(argv[2]);
	gpio_last_index_map = atoi(argv[3]);
	gpio_last_edge_map  = atoi(argv[4]);
	sleep_time          = atoi(argv[5]);
	wake_up_way         = atoi(argv[6]);
	gpio_stay_lo_map    = atoi(argv[7]);
	gpio_stay_hi_map    = atoi(argv[8]);

	aos_cli_printf("---deep sleep test param : 0x%0X 0x%0X 0x%0X 0x%0X %d %d\r\n",
			gpio_index_map, gpio_edge_map, gpio_last_index_map, gpio_last_edge_map, sleep_time, wake_up_way);

	bk_enter_deep_sleep(gpio_index_map, gpio_edge_map,
						gpio_last_index_map, gpio_last_edge_map,
						sleep_time, wake_up_way,
						gpio_stay_lo_map, gpio_stay_hi_map);
}

static void handle_delif_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	if (argc < 2) {
		aos_cli_printf("delif [sta|softap]\r\n");
		return;
	}

	if (!strcmp(argv[1], "sta")) {
		aos_cli_printf("remove station.\r\n");
		hal_wifi_suspend_station(NULL);
	} else if (!strcmp(argv[1], "softap")) {
		aos_cli_printf("remove soft ap.\r\n");
		hal_wifi_suspend_soft_ap(NULL);
	} else {
		aos_cli_printf("delif [sta|softap]\r\n");
	}
}

#if (BEKEN_TEST_BLE_TYPE == BEKEN_TEST_BLE_5_0)
//#include "app_ble.h"
//#include "app_sdp.h"
static void ble_notice_cb(ble_notice_t notice, void *param)
{
	switch (notice) {
	case BLE_5_STACK_OK:
		bk_printf("ble stack ok");
		break;
	case BLE_5_WRITE_EVENT:
	{
		write_req_t *w_req = (write_req_t *)param;
		bk_printf("write_cb:conn_idx:%d, prf_id:%d, add_id:%d, len:%d, data[0]:%02x\r\n",
			w_req->conn_idx, w_req->prf_id, w_req->att_idx, w_req->len, w_req->value[0]);
		break;
	}
	case BLE_5_READ_EVENT:
	{
		read_req_t *r_req = (read_req_t *)param;
		bk_printf("read_cb:conn_idx:%d, prf_id:%d, add_id:%d\r\n",
			r_req->conn_idx, r_req->prf_id, r_req->att_idx);
		r_req->value[0] = 0x12;
		r_req->value[1] = 0x34;
		r_req->value[2] = 0x56;
		r_req->length = 3;
		break;
	}
	case BLE_5_REPORT_ADV:
	{
		recv_adv_t *r_ind = (recv_adv_t *)param;
		bk_printf("r_ind:actv_idx:%d, adv_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			r_ind->actv_idx, r_ind->adv_addr[0], r_ind->adv_addr[1], r_ind->adv_addr[2],
			r_ind->adv_addr[3], r_ind->adv_addr[4], r_ind->adv_addr[5]);
		break;
	}
	case BLE_5_MTU_CHANGE:
	{
		mtu_change_t *m_ind = (mtu_change_t *)param;
		bk_printf("m_ind:conn_idx:%d, mtu_size:%d\r\n", m_ind->conn_idx, m_ind->mtu_size);
		break;
	}
	case BLE_5_CONNECT_EVENT:
	{
		conn_ind_t *c_ind = (conn_ind_t *)param;
		bk_printf("c_ind:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1],
			c_ind->peer_addr[2], c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
		break;
	}
	case BLE_5_DISCONNECT_EVENT:
	{
		discon_ind_t *d_ind = (discon_ind_t *)param;
		bk_printf("d_ind:conn_idx:%d,reason:%d\r\n", d_ind->conn_idx,d_ind->reason);
		break;
	}
	case BLE_5_ATT_INFO_REQ:
	{
		att_info_req_t *a_ind = (att_info_req_t *)param;
		bk_printf("a_ind:conn_idx:%d\r\n", a_ind->conn_idx);
		a_ind->length = 128;
		a_ind->status = ERR_SUCCESS;
		break;
	}
	case BLE_5_CREATE_DB:
	{
		create_db_t *cd_ind = (create_db_t *)param;
		bk_printf("cd_ind:prf_id:%d, status:%d\r\n", cd_ind->prf_id, cd_ind->status);
		break;
	}
	case BLE_5_INIT_CONNECT_EVENT:
	{
		conn_ind_t *c_ind = (conn_ind_t *)param;
		bk_printf("BLE_5_INIT_CONNECT_EVENT:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1],
			c_ind->peer_addr[2], c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
		break;
	}
	case BLE_5_INIT_DISCONNECT_EVENT:
	{
		discon_ind_t *d_ind = (discon_ind_t *)param;
		bk_printf("BLE_5_INIT_DISCONNECT_EVENT:conn_idx:%d,reason:%d\r\n", d_ind->conn_idx,d_ind->reason);
		break;
	}
	default:
		break;
	}
}

void ble_cmd_cb(ble_cmd_t cmd, ble_cmd_param_t *param)
{
	bk_printf("cmd:%d idx:%d status:%d\r\n", cmd, param->cmd_idx, param->status);
}

#define OPEN_MASRER_BLE_SDP   1

#if BLE_SDP_CLIENT || OPEN_MASRER_BLE_SDP
__maybe_unused static void ble_app_sdp_characteristic_cb(unsigned char conidx,uint16_t chars_val_hdl,unsigned char uuid_len,unsigned char *uuid);
static void ble_app_sdp_characteristic_cb(unsigned char conidx,uint16_t chars_val_hdl,unsigned char uuid_len,unsigned char *uuid)
{
	bk_printf("[APP]characteristic conidx:%d,handle:0x%02x(%d),UUID:0x",conidx,chars_val_hdl,chars_val_hdl);
	for(int i = 0; i< uuid_len; i++)
	{
		bk_printf("%02x ",uuid[i]);
	}
	bk_printf("\r\n");
}

///void app_sdp_charac_cb(CHAR_TYPE type,uint8 conidx,uint16_t hdl,uint16_t len,uint8 *data)
void app_sdp_charac_cb(int type,uint8 conidx,uint16_t hdl,uint16_t len,uint8 *data)
{
	bk_printf("[APP]type:%x conidx:%d,handle:0x%02x(%d),len:%d,0x",type,conidx,hdl,hdl,len);
	for(int i = 0; i< len; i++)
	{
		bk_printf("%02x ",data[i]);
	}
	bk_printf("\r\n");
}
#endif
#define BLE_VSN5_DEFAULT_MASTER_IDX      0

#endif
static void handle_ble_cmd(char *pwbuf, int blen, int argc, char **argv)
{
#if (BEKEN_TEST_BLE_TYPE == BEKEN_TEST_BLE_4_2)
	if ((argc < 2) || (strcmp(argv[1], "help") == 0)) {
		beken_test_ble_command_usage();
		return;
	}

	if (strcmp(argv[1], "active") == 0) {
		ble_set_write_cb(beken_test_ble_write_callback);
		ble_set_read_cb(beken_test_ble_read_callback);
		ble_set_event_cb(beken_test_ble_event_callback);
		ble_activate(NULL);
	} else if (strcmp(argv[1], "start_adv") == 0)
		beken_test_ble_advertise();
	else if (strcmp(argv[1], "stop_adv") == 0) {
		if (ERR_SUCCESS != appm_stop_advertising())
			aos_cli_printf("stop advertising fail.\r\n");
	} else if (strcmp(argv[1], "notify") == 0) {
		uint8_t len;
		uint16_t prf_id;
		uint16_t att_id;
		uint8_t write_buffer[20];

		if (argc != 5) {
			beken_test_ble_command_usage();
			return;
		}

		len = strlen(argv[4]);
		if ((len % 2 != 0) || (len > 40)) {
			aos_cli_printf("notify length error %d.\r\n", len);
			return;
		}
		hexstr2bin(argv[4], write_buffer, len / 2);

		prf_id = atoi(argv[2]);
		att_id = atoi(argv[3]);

		if (ERR_SUCCESS != bk_ble_send_ntf_value(len / 2, write_buffer, prf_id, att_id))
			aos_cli_printf("send notify fail.\r\n");
	} else if (os_strcmp(argv[1], "indicate") == 0) {
		uint8_t len;
		uint16_t prf_id;
		uint16_t att_id;
		uint8_t write_buffer[20];

		if (argc != 5) {
			beken_test_ble_command_usage();
			return;
		}

		len = strlen(argv[4]);
		if ((len % 2 != 0) || (len > 40)) {
			aos_cli_printf("indicate length error %d.\r\n", len);
			return;
		}
		hexstr2bin(argv[4], write_buffer, len / 2);

		prf_id = atoi(argv[2]);
		att_id = atoi(argv[3]);

		if (ERR_SUCCESS != bk_ble_send_ind_value(len / 2, write_buffer, prf_id, att_id))
			aos_cli_printf("send indicate fail.\r\n");
	} else if (strcmp(argv[1], "disc") == 0)
		appm_disconnect();

#elif (BEKEN_TEST_BLE_TYPE == BEKEN_TEST_BLE_5_0)
	unsigned char adv_data[31];
	unsigned char actv_idx;

   if (os_strcmp(argv[1], "active") == 0) {
		ble_set_notice_cb(ble_notice_cb);
		beken_test_bk_ble_init();
	}
	if (os_strcmp(argv[1], "create_adv") == 0) {
		actv_idx = app_ble_get_idle_actv_idx_handle();
		bk_ble_create_advertising(actv_idx, 7, 160, 160, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "set_adv_data") == 0) {
		adv_data[0] = 0x02;
		adv_data[1] = 0x01;
		adv_data[2] = 0x06;
		adv_data[3] = 0x0B;
		adv_data[4] = 0x09;
		memcpy(&adv_data[5], "7231N_BLE", 10);
		bk_ble_set_adv_data(os_strtoul(argv[2], NULL, 10), adv_data, 0xF, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "set_rsp_data") == 0) {
		adv_data[0] = 0x07;
		adv_data[1] = 0x08;
		memcpy(&adv_data[2], "7231N", 6);
		bk_ble_set_scan_rsp_data(os_strtoul(argv[2], NULL, 10), adv_data, 0x8, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "start_adv") == 0) {
		bk_ble_start_advertising(os_strtoul(argv[2], NULL, 10), 0, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "stop_adv") == 0) {
		bk_ble_stop_advertising(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "delete_adv") == 0) {
		bk_ble_delete_advertising(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "create_scan") == 0) {
		actv_idx = app_ble_get_idle_actv_idx_handle();
		bk_ble_create_scaning(actv_idx, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "start_scan") == 0) {
		bk_ble_start_scaning(os_strtoul(argv[2], NULL, 10), 100, 30, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "stop_scan") == 0) {
		bk_ble_stop_scaning(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "delete_scan") == 0) {
		bk_ble_delete_scaning(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "update_conn") == 0) {
		bk_ble_update_param(os_strtoul(argv[2], NULL, 10), 50, 50, 0, 800, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "dis_conn") == 0) {
		bk_ble_disconnect(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "mtu_change") == 0) {
		bk_ble_gatt_mtu_change(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "init_adv") == 0) {
		struct adv_param adv_info;
		adv_info.channel_map = 7;
		adv_info.duration = 0;
		adv_info.interval_min = 160;
		adv_info.interval_max = 160;
		adv_info.advData[0] = 0x02;
		adv_info.advData[1] = 0x01;
		adv_info.advData[2] = 0x06;
		adv_info.advData[3] = 0x0B;
		adv_info.advData[4] = 0x09;
		memcpy(&adv_info.advData[5], "7231N_BLE", 10);
		adv_info.advDataLen = 0xF;
		adv_info.respData[0] = 0x07;
		adv_info.respData[1] = 0x08;
		memcpy(&adv_info.respData[2], "7231N", 6);
		adv_info.respDataLen = 0x8;
		actv_idx = app_ble_get_idle_actv_idx_handle();
		bk_ble_adv_start(actv_idx, &adv_info, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "deinit_adv") == 0) {
		bk_ble_adv_stop(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "init_scan") == 0) {
		struct scan_param scan_info;
		scan_info.channel_map = 7;
		scan_info.interval = 100;
		scan_info.window = 30;
		actv_idx = app_ble_get_idle_actv_idx_handle();
		bk_ble_scan_start(actv_idx, &scan_info, ble_cmd_cb);
	}
	if (os_strcmp(argv[1], "deinit_scan") == 0) {
		bk_ble_scan_stop(os_strtoul(argv[2], NULL, 10), ble_cmd_cb);
	}
#if CFG_BLE_MASTER_ROLE_NUM
	if (os_strcmp(argv[1], "con_create") == 0)
	{
		ble_set_notice_cb(ble_notice_cb);
	#if BLE_SDP_CLIENT || OPEN_MASRER_BLE_SDP
		register_app_sdp_characteristic_callback(ble_app_sdp_characteristic_cb);
		register_app_sdp_charac_callback(app_sdp_charac_cb);
	#endif
		actv_idx = app_ble_get_idle_conn_idx_handle();
		bk_printf("------------->actv_idx:%d\r\n",actv_idx);
		///actv_idx = BLE_VSN5_DEFAULT_MASTER_IDX;
		///appm_create_init(actv_idx, 0, 0, 0);
		bk_ble_create_init(actv_idx, 0, 0, 0,ble_cmd_cb);
	}
	else if ((os_strcmp(argv[1], "con_start") == 0) && (argc >= 3))
	{
		struct ///bd_addr
		{
		    ///6-byte array address value
		    unsigned char  addr[6];
		}bdaddr;
		///struct bd_addr bdaddr;
		unsigned char addr_type = 0;//ADDR_PUBLIC;
		int addr_type_str = atoi(argv[3]);
		int actv_idx_str = atoi(argv[4]);
		bk_printf("idx:%d,addr_type:%d\r\n",actv_idx_str,addr_type_str);
		if((addr_type_str > 3/*ADDR_RPA_OR_RAND*/)||(actv_idx_str >= 0xFF)){
			return;
		}
		actv_idx = actv_idx_str;
		hexstr2bin(argv[2], bdaddr.addr, 6);
		addr_type = addr_type_str;
		bk_ble_init_set_connect_dev_addr(actv_idx,&bdaddr,addr_type);
		bk_ble_init_start_conn(actv_idx,ble_cmd_cb);
	}
	else if ((os_strcmp(argv[1], "con_stop") == 0) && (argc >= 3))
	{
		int actv_idx_str = atoi(argv[2]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		bk_ble_init_stop_conn(actv_idx,ble_cmd_cb);
	}
	else if ((os_strcmp(argv[1], "con_dis") == 0) && (argc >= 3))
	{
		int actv_idx_str = atoi(argv[2]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		app_ble_master_appm_disconnect(actv_idx);
	}
#if BLE_SDP_CLIENT || OPEN_MASRER_BLE_SDP
	else if (os_strcmp(argv[1], "con_read") == 0)
	{
		if(argc < 4){
			bk_printf("param error\r\n");
			return;
		}
		int actv_idx_str = atoi(argv[3]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		int handle = atoi(argv[2]);
		if(handle >=0 && handle <= 0xFFFF){
			bk_ble_read_service_data_by_handle_req(actv_idx,handle,ble_cmd_cb);
			///appm_read_service_data_by_handle_req(BLE_VSN5_DEFAULT_MASTER_IDX,handle);
		}
		else{
			bk_printf("handle(%x) error\r\n",handle);
		}
	}
	else if (os_strcmp(argv[1], "con_write") == 0)
	{
		if(argc < 4){
			bk_printf("param error\r\n");
			return;
		}
		int handle = atoi(argv[2]);
		int actv_idx_str = atoi(argv[3]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		unsigned char test_buf[4] = {0x01,0x02,0x22,0x32};
		if(handle >=0 && handle <= 0xFFFF){
			bk_ble_write_service_data_req(actv_idx,handle,4,test_buf,ble_cmd_cb);
			///appc_write_service_data_req(BLE_VSN5_DEFAULT_MASTER_IDX,handle,4,test_buf);
		}else{
			bk_printf("handle(%x) error\r\n",handle);
		}
	}
	else if (os_strcmp(argv[1], "con_rd_sv_ntf_int_cfg") == 0)
	{
		if(argc < 4){
			bk_printf("param error\r\n");
			return;
		}
		int actv_idx_str = atoi(argv[3]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		int handle = atoi(argv[2]);
		if(handle >=0 && handle <= 0xFFFF){
			appm_read_service_ntf_ind_cfg_by_handle_req(actv_idx,handle);
		}else{
			bk_printf("handle(%x) error\r\n",handle);
		}
	}
	else if (os_strcmp(argv[1], "con_rd_sv_ud_cfg") == 0)
	{
		if(argc < 4){
			bk_printf("param error\r\n");
			return;
		}
		int actv_idx_str = atoi(argv[3]);
		bk_printf("idx:%d\r\n",actv_idx_str);
		if(actv_idx_str >= 0xFF){
			return;
		}
		actv_idx = actv_idx_str;
		int handle = atoi(argv[2]);
		if(handle >=0 && handle <= 0xFFFF){
			appm_read_service_userDesc_by_handle_req(actv_idx,handle);
		}else{
			bk_printf("handle(%x) error\r\n",handle);
		}
	}
#endif
#endif
#else
	aos_cli_printf("NOT SUPPORT BLE.\r\n");
#endif
}

static void handle_ping_cmd(char *pwbuf, int blen, int argc, char **argv)
{    
	if (argc < 2) {
        aos_cli_printf("ping <host address>\r\n");
		return;
    }

	aos_cli_printf("ping IP address:%s\n", argv[1]);
	ping(argv[1], 4, 0);
}

static void handle_iperf_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	iperf(argc, argv);
}

static void handle_ifconfig_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	hal_wifi_ip_stat_t ip_status;

	memset(&ip_status, 0x0, sizeof(hal_wifi_ip_stat_t));
	hal_wifi_get_ip_stat(NULL, &ip_status, STATION);

	aos_cli_printf("dhcp=%d ip=%s gate=%s mask=%s mac=%02x:%02x:%02x:%02x:%02x:%02x \r\n",
				ip_status.dhcp, ip_status.ip, ip_status.gate, ip_status.mask,
				(uint8_t)ip_status.mac[0], (uint8_t)ip_status.mac[1], (uint8_t)ip_status.mac[2],
				(uint8_t)ip_status.mac[3], (uint8_t)ip_status.mac[4], (uint8_t)ip_status.mac[5]);
}

static void handle_wifistate_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	hal_wifi_link_stat_t linkStatus;
	network_InitTypeDef_ap_st ap_info;
	char ssid[33] = {0};

#if CFG_IEEE80211N
	aos_cli_printf("station: %d, softap: %d, b/g/n\r\n",sta_ip_is_start(),uap_ip_is_start());
#else
	aos_cli_printf("station: %d, softap: %d, b/g\r\n",sta_ip_is_start(),uap_ip_is_start());
#endif

	if (sta_ip_is_start()) {
		memset(&linkStatus, 0x0, sizeof(hal_wifi_link_stat_t));
		hal_wifi_get_link_stat(NULL, &linkStatus);
		memcpy(ssid, linkStatus.ssid, 32);

		aos_cli_printf("sta:rssi=%d,ssid=%s,bssid=%02x:%02x:%02x:%02x:%02x:%02x,channel=%d,cipher_type:",
			linkStatus.wifi_strength, ssid, linkStatus.bssid[0], linkStatus.bssid[1], linkStatus.bssid[2],
			linkStatus.bssid[3], linkStatus.bssid[4], linkStatus.bssid[5], linkStatus.channel);
		switch (bk_sta_cipher_type()) {
			case BK_SECURITY_TYPE_NONE:
				aos_cli_printf("OPEN\r\n");
				break;
			case BK_SECURITY_TYPE_WEP :
				aos_cli_printf("WEP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA_TKIP:
				aos_cli_printf("TKIP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA2_AES:
				aos_cli_printf("CCMP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA2_MIXED:
				aos_cli_printf("WPA/WPA2 MIXED\r\n");
				break;
			case BK_SECURITY_TYPE_AUTO:
				aos_cli_printf("AUTO\r\n");
				break;
			case BK_SECURITY_TYPE_WPA3_SAE:
				aos_cli_printf("WPA3\r\n");
				break;
			case BK_SECURITY_TYPE_WPA3_WPA2_MIXED:
				aos_cli_printf("WPA2/WPA3 MIXED\r\n");
				break;
			default:
				aos_cli_printf("Error\r\n");
				break;
		}
	}

	if (uap_ip_is_start()) {
		memset(&ap_info, 0x0, sizeof(network_InitTypeDef_ap_st));
		bk_wlan_ap_para_info_get(&ap_info);
		memcpy(ssid, ap_info.wifi_ssid, 32);
		aos_cli_printf("softap:ssid=%s,channel=%d,dhcp=%d,cipher_type:",
			ssid, ap_info.channel,ap_info.dhcp_mode);
		switch (ap_info.security) {
			case BK_SECURITY_TYPE_NONE:
				aos_cli_printf("OPEN\r\n");
				break;
			case BK_SECURITY_TYPE_WEP :
				aos_cli_printf("WEP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA_TKIP:
				aos_cli_printf("TKIP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA2_AES:
				aos_cli_printf("CCMP\r\n");
				break;
			case BK_SECURITY_TYPE_WPA2_MIXED:
				aos_cli_printf("WPA/WPA2 MIXED\r\n");
				break;
			case BK_SECURITY_TYPE_AUTO:
				aos_cli_printf("AUTO\r\n");
				break;
			case BK_SECURITY_TYPE_WPA3_SAE:
				aos_cli_printf("WPA3\r\n");
				break;
			case BK_SECURITY_TYPE_WPA3_WPA2_MIXED:
				aos_cli_printf("WPA2/WPA3 MIXED\r\n");
				break;
			default:
				aos_cli_printf("Error\r\n");
				break;
		}
		aos_cli_printf("ip=%s,gate=%s,mask=%s,dns=%s\r\n",
			ap_info.local_ip_addr, ap_info.gateway_ip_addr, ap_info.net_mask, ap_info.dns_server_ip_addr);
	}
}

static const struct cli_command beken_test_cmds[] = {
	{"sta", "sta [ssid] [password]", handle_sta_cmd},
	{"softap", "softap [ssid] [password]", handle_softap_cmd},
	{"scan", "scan", handle_scan_cmd},
	{"ps", "ps [rfdtim|mcudtim] [0|1]", handle_ps_cmd},
	{"deep_sleep", "deep_sleep", handle_deepsleep_cmd},
	{"delif", "delif [sta|softap]", handle_delif_cmd},
	{"ble", "ble", handle_ble_cmd},
	{"ping", "ping [ip]", handle_ping_cmd},
	{"iperf", "iperf", handle_iperf_cmd},
	{"ifconfig", "show IP address", handle_ifconfig_cmd},
	{"wifistate", "show wifi state", handle_wifistate_cmd}
};

int beken_test_register_cmds(void)
{
	int ret;

	if ((ret = aos_cli_register_commands(&beken_test_cmds[0],
										 sizeof(beken_test_cmds) / sizeof(struct cli_command))) != 0)
		aos_cli_printf("register beken test commands fail.\r\n");

	return ret;
}


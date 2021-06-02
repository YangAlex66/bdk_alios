/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos_main.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <aos/init.h>
#include <aos/aos.h>
#include "wlan_ui_pub.h"

#include "ate_app.h"
#include "cmd_evm.h"
#include "cmd_rx_sensitivity.h"
#include "arm_arch.h"
#include "board.h"
#include "uart_pub.h"
#include "bk7011_cal_pub.h"
#include "osk_revision.h"
#include "sys_ctrl_pub.h"

#define AOS_START_STACK 2048

ktask_t *g_aos_init;

extern void board_init(void);

static kinit_t kinit = {
    .argc = 0,
    .argv = NULL,
    .cli_enable = 1
};

#if ATE_APP_FUN 
static void tx_evm_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int ret = do_evm(NULL, 0, argc, argv);
    if(ret)
    {
        printf("tx_evm bad parameters\r\n");
    }
}

static void rx_sens_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int ret = do_rx_sensitivity(NULL, 0, argc, argv);
    if(ret)
    {
        printf("rx sensitivity bad parameters\r\n");
    }
}

static void efuse_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    uint8_t addr, data;
    
    if(argc == 3)
    {
        if (strncmp(argv[1], "-r", 2) == 0) {
            hexstr2bin(argv[2], &addr, 1);
            aos_cli_printf("efuse read: addr-0x%02x, data-0x%02x\r\n",
                        addr, wifi_read_efuse(addr));
        } 
    } 
    else if(argc == 4) 
    {
        if(strncmp(argv[1], "-w", 2) == 0)  {
            hexstr2bin(argv[2], &addr, 1);
            hexstr2bin(argv[3], &data, 6);
            aos_cli_printf("efuse write: addr-0x%02x, data-0x%02x, ret:%d\r\n",
                        addr, data, wifi_write_efuse(addr, data));
        }
    }
    else {
        printf("efuse [-r addr] [-w addr data]\r\n");
    }
}

static void efuse_mac_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	uint8_t mac[6];
	
	if (argc == 1)
	{
		if(wifi_get_mac_address_from_efuse(mac))
			aos_cli_printf("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else if(argc == 2)
	{
		if (strncmp(argv[1], "-r", 2) == 0) {
			if(wifi_get_mac_address_from_efuse(mac))
				aos_cli_printf("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		} 
	} 
	else if(argc == 3) 
	{
		if(strncmp(argv[1], "-w", 2) == 0)	{
			hexstr2bin(argv[2], mac, 6);
			//if(wifi_set_mac_address_to_efuse(mac))
				aos_cli_printf("Set MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
	else {
		printf("efusemac [-r] [-w] [mac]\r\n");
	}
}

static void reg_write_read_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	UINT32 reg_addr, reg_value;
	if(strncmp(argv[1], "-r", 2) == 0) {
		if(argc != 3) {
			printf("bkreg -r addr\r\n");
			return;
		}

		hexstr2bin(argv[2], &reg_addr, 4);
		reg_addr = ntohl(reg_addr);
		aos_cli_printf("bkreg R: addr:0x%08x, value:0x%08x\r\n", reg_addr, REG_READ(reg_addr));
	}
	else if(strncmp(argv[1], "-w", 2) == 0) {
		if(argc != 4) {
			printf("bkreg -w addr value\r\n");
			return;
		}

		hexstr2bin(argv[2], &reg_addr, 4);
		reg_addr = ntohl(reg_addr);

		hexstr2bin(argv[3], &reg_value, 4);
		reg_value = ntohl(reg_value);

		REG_WRITE(reg_addr, reg_value);

		extern INT32 rwnx_cal_save_trx_rcbekn_reg_val(void);
		// when write trx and rc beken regs, updata registers save.
		if( (reg_addr & 0xfff0000) == 0x1050000)
			rwnx_cal_save_trx_rcbekn_reg_val();

		aos_cli_printf("bkreg W: addr:0x%08x, value:0x%08x - check:0x%08x\r\n",
			reg_addr, reg_value, REG_READ(reg_addr));
	}
	else {
		printf("bkreg -w/r addr [value]\r\n");
	}
}

#if ((CFG_SOC_NAME != SOC_BK7231) && (CFG_SUPPORT_BLE == 1))
#define NAME_DIF_LEN            4
#define MAX_BLE_NAME_LEN        9

static void linkkit_ble_usage(void)
{
    aos_cli_printf("ble help           - Help information\n");
    aos_cli_printf("ble dut            - Active ble to do BLE DUT\n");
    aos_cli_printf("ble info           - get ble app info\n");     
}

static void ble_get_info_Command(void)
{
    UINT8 ble_mac[6];
    aos_cli_printf("\r\n****** ble information ************\r\n");

    ble_get_mac(ble_mac);
    aos_cli_printf("* mac:%02x-%02x-%02x-%02x-%02x-%02x\r\n", ble_mac[0], ble_mac[1],ble_mac[2],ble_mac[3],ble_mac[4],ble_mac[5]);
    aos_cli_printf("***********  end  *****************\r\n");
}

static void ble_entry_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if ((argc == 1) || (strcmp(argv[1], "help") == 0))
    {
        linkkit_ble_usage();
        return;
    }
    else if (strcmp(argv[1], "dut") == 0)
    {
		ble_dut_start();
    }
    else if (strcmp(argv[1], "info") == 0)
    {
		ble_get_info_Command();
    }

	return;
}
#endif

static void linkkit_wlan_start_ap(const char *ap_ssid, char *ap_key)
{
#define LINK_WLAN_DEFAULT_IP         "192.168.0.1"
#define LINK_WLAN_DEFAULT_GW         "192.168.0.1"
#define LINK_WLAN_DEFAULT_MASK       "255.255.255.0"

    network_InitTypeDef_st wNetConfig;
    int len;
    
    os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    len = os_strlen(ap_ssid);
    if(32 < len)
    {
        bk_printf("ssid name more than 32 Bytes\r\n");
        return;
    }

    os_strcpy((char *)wNetConfig.wifi_ssid, ap_ssid);
    os_strcpy((char *)wNetConfig.wifi_key, ap_key);
    
    wNetConfig.wifi_mode = BK_SOFT_AP;
    wNetConfig.dhcp_mode = DHCP_SERVER;
    wNetConfig.wifi_retry_interval = 100;
    os_strcpy((char *)wNetConfig.local_ip_addr, LINK_WLAN_DEFAULT_IP);
    os_strcpy((char *)wNetConfig.net_mask, LINK_WLAN_DEFAULT_MASK);
    os_strcpy((char *)wNetConfig.gateway_ip_addr, LINK_WLAN_DEFAULT_GW);
    os_strcpy((char *)wNetConfig.dns_server_ip_addr, LINK_WLAN_DEFAULT_GW);
    
    bk_printf("ssid:%s  key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
	bk_wlan_start(&wNetConfig);
}

static void linkkit_softap_usage(void)
{
    aos_cli_printf("softap help           - Help information\n");
    aos_cli_printf("softap info           - list softap info\n");
    aos_cli_printf("softap [ssid] [key]   - start wlan with softap with ssdi+key\n"); 
    aos_cli_printf("softap                - start wlan with BK7231AP-V1-xxxx,no key\r\n");     
}

static void softap_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    char *ap_ssid = NULL;
    char *ap_key;

    aos_cli_printf("SOFTAP_COMMAND\r\n\r\n");
    if (argc == 2)
    {
        if(!os_strcasecmp(argv[1], "help")){
            linkkit_softap_usage();
            return;
        }
        
        ap_ssid = argv[1];
        ap_key = "1";
    }
    else if (argc == 3)
    {
        ap_ssid = argv[1];
        ap_key = argv[2];
    } else {
        linkkit_softap_usage();
    }

    if(ap_ssid)
    {
        linkkit_wlan_start_ap(ap_ssid, ap_key);
    }
}

static void linkkit_wlan_connect_ap(const char *oob_ssid, char *connect_key)
{
	network_InitTypeDef_st wNetConfig;
    int len;
	os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    len = os_strlen(oob_ssid);
    if(32 < len)
    {
        bk_printf("ssid name more than 32 Bytes\r\n");
        return;
    }
    
	os_strcpy((char *)wNetConfig.wifi_ssid, oob_ssid);
	os_strcpy((char *)wNetConfig.wifi_key, connect_key);
    
	wNetConfig.wifi_mode = BK_STATION;
	wNetConfig.dhcp_mode = DHCP_CLIENT;
	wNetConfig.wifi_retry_interval = 100;
    
	bk_printf("ssid:%s key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
	bk_wlan_start(&wNetConfig);
}

static void linkkit_sta_usage(void)
{
    aos_cli_printf("sta help           - Help information\n");
    aos_cli_printf("sta info           - list sta info\n");
    aos_cli_printf("sta [ssid] [key]   - start wlan as sta to connect to router with ssdi+key\n");
    aos_cli_printf("sta                - start wlan as sta to connect BK7231STA-V1,no key\n");    
}

static void sta_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    char *oob_ssid = NULL;
    char *connect_key;

    aos_cli_printf("sta_Command\r\n");
    if (argc == 2)
    {
        if(!os_strcasecmp(argv[1], "help")){
            linkkit_sta_usage();
            return;
        }

        oob_ssid = argv[1];
        connect_key = "1";
    }
    else if (argc == 3)
    {
        oob_ssid = argv[1];
        connect_key = argv[2];
    }
    else
    {
        linkkit_sta_usage();
    }

    if(oob_ssid)
    {
        linkkit_wlan_connect_ap(oob_ssid, connect_key);
    }
}

extern void cmd_rfcali_cfg_mode(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
extern void cmd_rfcali_cfg_rate_dist(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
extern void cmd_rfcali_cfg_tssi_g(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
extern void cmd_rfcali_cfg_tssi_b(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);
extern void cmd_rfcali_show_data(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);

static const struct cli_command cli_cmd_rftest[] = {
    {"txevm",       "txevm [-m] [-c] [-l] [-r] [-w]", tx_evm_cmd_test},
    {"rxsens",      "rxsens [-m] [-d] [-c] [-l]",     rx_sens_cmd_test},
    {"efuse",       "efuse [-r addr] [-w addr data]", efuse_cmd_test},
    {"efusemac",    "efusemac [-r] [-w] [mac]",       efuse_mac_cmd_test},
    {"bkreg",       "bkreg -w/r addr [value]",        reg_write_read_test},
	#if ((CFG_SOC_NAME != SOC_BK7231) && (CFG_SUPPORT_BLE == 1))
	{"ble",         "ble help/active/dut",            ble_entry_Command},
	#endif
    {"softap",      "softap ssid key",                softap_Command},      
    {"sta",         "sta ap_ssid key",                sta_Command},  
};

const struct cli_command cli_cmd_auto_rfcali[] = {
    {"rfcali_cfg_mode",      "1:manual, 0:auto",      cmd_rfcali_cfg_mode},
    {"rfcali_cfg_tssi_g",    "0-255",                 cmd_rfcali_cfg_tssi_g},
    {"rfcali_cfg_tssi_b",    "0-255",                 cmd_rfcali_cfg_tssi_b},
    {"rfcali_show_data",     "",                      cmd_rfcali_show_data},
    {"rfcali_cfg_rate_dist", "b g n40 ble (0-31)",    cmd_rfcali_cfg_rate_dist},
};
#endif

UINT32 rwnx_cal_load_user_rfcali_mode(int *rfcali_mode)
{
    if(rfcali_mode)
    {
        //*rfcali_mode = CALI_MODE_AUTO;
        *rfcali_mode = CALI_MODE_MANUAL;
        return 1;
    }
    else
        return 0;
}

UINT32 rwnx_cal_load_user_g_tssi_threshold(int *tssi_g)
{
    if(tssi_g)
    {
        *tssi_g = 110;   // range 0-225
        return 1;
    }
    else
        return 0;
}

UINT32 rwnx_cal_load_user_b_tssi_threshold(int *tssi_b)
{
    if(tssi_b)
    {
        *tssi_b = 115;   // range 0-225
        return 1;
    }
    else
        return 0;
}

UINT32 rwnx_cal_is_auto_rfcali_printf_on(void)
{
    return 0;
}

static void sys_init(void)
{
    soc_system_init();

    board_init();

#if ATE_APP_FUN
	if(get_ate_mode_state()) 
	{
		sctrl_rf_ps_enable_clear();

    	#ifdef CONFIG_AOS_CLI
		cli_service_init(&kinit);
    	#endif
		aos_cli_register_commands(&cli_cmd_rftest[0],
			sizeof(cli_cmd_rftest) / sizeof(struct cli_command));

		aos_cli_register_commands(&cli_cmd_auto_rfcali[0],
			sizeof(cli_cmd_auto_rfcali) / sizeof(struct cli_command));
	} 
	else
#endif
	{
    	aos_kernel_init(&kinit);
		
    }		
}

static void aos_show_osk_revision(void)
{
	aos_cli_printf("\nOSK Rev: %s\r\n", BEKEN_OSK_REV);
}

void sys_start(void)
{
	uart_print_port = STDIO_UART;

    aos_init();

    soc_driver_init();

    aos_show_osk_revision();

    krhino_task_dyn_create(&g_aos_init, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, sys_init, 1);

    aos_start();
}


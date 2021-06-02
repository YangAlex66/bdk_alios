#include "ble_pub.h"
#include "ble_api.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "breeze_hal_ble.h"


#define BLE_CONNECTION_MAX   2
#define BLE_ACTIVITY_MAX     3

extern struct bd_addr common_default_bdaddr;
static ais_bt_init_t ais_bt_init_info;
static void (*g_indication_txdone)(uint8_t res);
breeze_config_t breeze_cfg; 
uint8_t g_connected = 0;

#define BK_ATT_DECL_PRIMARY_SERVICE     {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_DECL_CHARACTERISTIC      {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_DESC_CLIENT_CHAR_CFG     {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define BK_ATT_FEB3S_IDX_FED4_VAL_VALUE {0xD4,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_FEB3S_IDX_FED5_VAL_VALUE {0xD5,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_FEB3S_IDX_FED6_VAL_VALUE {0xD6,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_FEB3S_IDX_FED7_VAL_VALUE {0xD7,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_FEB3S_IDX_FED8_VAL_VALUE {0xD8,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

uint8_t uuid_feb3[16] = {0xB3,0xFE,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

const bk_attm_desc_t feb3_att_db[FEB3S_IDX_NB] =
{
	// FEB3 Service Declaration
	[FEB3S_IDX_SVC]                  =   {BK_ATT_DECL_PRIMARY_SERVICE, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed4  Characteristic Declaration
	[FEB3S_IDX_FED4_VAL_CHAR]        =   {BK_ATT_DECL_CHARACTERISTIC, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed4  Characteristic Value
	[FEB3S_IDX_FED4_VAL_VALUE]       =   {BK_ATT_FEB3S_IDX_FED4_VAL_VALUE, BK_PERM_SET(RD, ENABLE), BK_PERM_SET(RI, ENABLE), FEB3_CHAR_DATA_LEN},
	
	// fed5  Characteristic Declaration
	[FEB3S_IDX_FED5_VAL_CHAR]        =   {BK_ATT_DECL_CHARACTERISTIC, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed5  Characteristic Value
	[FEB3S_IDX_FED5_VAL_VALUE]       =   {BK_ATT_FEB3S_IDX_FED5_VAL_VALUE, BK_PERM_SET(WRITE_REQ, ENABLE)|BK_PERM_SET(RD, ENABLE), BK_PERM_SET(RI, ENABLE), FEB3_CHAR_DATA_LEN},
	
	// fed6  Characteristic Declaration
	[FEB3S_IDX_FED6_VAL_CHAR]        =   {BK_ATT_DECL_CHARACTERISTIC, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed6  Characteristic Value
	[FEB3S_IDX_FED6_VAL_VALUE]       =   {BK_ATT_FEB3S_IDX_FED6_VAL_VALUE, BK_PERM_SET(IND, ENABLE)|BK_PERM_SET(RD, ENABLE), BK_PERM_SET(RI, ENABLE), FEB3_CHAR_DATA_LEN},
	[FFB3S_IDX_FED6_VAL_IND_CFG]     =   {BK_ATT_DESC_CLIENT_CHAR_CFG, BK_PERM_SET(RD, ENABLE)|BK_PERM_SET(WRITE_REQ, ENABLE), 0, 0},
	
	// fed7  Characteristic Declaration
	[FEB3S_IDX_FED7_VAL_CHAR]        =   {BK_ATT_DECL_CHARACTERISTIC, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed7  Characteristic Value
	[FEB3S_IDX_FED7_VAL_VALUE]       =   {BK_ATT_FEB3S_IDX_FED7_VAL_VALUE, BK_PERM_SET(WRITE_COMMAND, ENABLE)|BK_PERM_SET(RD, ENABLE), BK_PERM_SET(RI, ENABLE), FEB3_CHAR_DATA_LEN},
	
	// fed8  Characteristic Declaration
	[FEB3S_IDX_FED8_VAL_CHAR]        =   {BK_ATT_DECL_CHARACTERISTIC, BK_PERM_SET(RD, ENABLE), 0, 0},
	// fed8  Characteristic Value
	[FEB3S_IDX_FED8_VAL_VALUE]       =   {BK_ATT_FEB3S_IDX_FED8_VAL_VALUE, BK_PERM_SET(NTF, ENABLE)|BK_PERM_SET(RD, ENABLE), BK_PERM_SET(RI, ENABLE), FEB3_CHAR_DATA_LEN},
	[FFB3S_IDX_FED8_VAL_NTF_CFG]     =   {BK_ATT_DESC_CLIENT_CHAR_CFG, BK_PERM_SET(RD, ENABLE)|BK_PERM_SET(WRITE_REQ, ENABLE), 0, 0},
};/// Macro used to retrieve permission value from access and rights on attribute.

struct{
	unsigned char adv_idx;
	unsigned char connidx;
	unsigned char channel_map;
	unsigned char  advDataLen;
	unsigned char  respDataLen;
	unsigned char  advData[MAX_ADV_DATA_LEN];
    unsigned char  respData[MAX_ADV_DATA_LEN];
	unsigned short interval_min;
	unsigned short interval_max;
}hal_ble_env = {
	.adv_idx = 0xFF,
	.connidx = 0xFF,
	.channel_map = 7,
	.interval_min = 160,
	.interval_max = 160,
	.advDataLen = 0,
	.respDataLen = 0,
};

void hal_ble_cmd_cb(ble_cmd_t cmd, ble_cmd_param_t *param)
{
	bk_printf("[hal_ble_cmd_cb]cmd:%d idx:%d status:%d\r\n", cmd, param->cmd_idx, param->status);
	switch(cmd){
		case BLE_CREATE_ADV:
			bk_ble_set_adv_data(hal_ble_env.adv_idx, hal_ble_env.advData, hal_ble_env.advDataLen, hal_ble_cmd_cb);
			break;
		case BLE_SET_ADV_DATA:
			bk_ble_set_scan_rsp_data(hal_ble_env.adv_idx, hal_ble_env.respData, hal_ble_env.respDataLen, hal_ble_cmd_cb);
			break;
		case BLE_SET_RSP_DATA:
			bk_ble_start_advertising(hal_ble_env.adv_idx, 0, hal_ble_cmd_cb);
			break;
		case BLE_START_ADV:
			bk_printf("BLE_START_ADV\r\n");
			break;
		case BLE_STOP_ADV:
			break;
		case BLE_DELETE_ADV:
			break;
		case BLE_CREATE_SCAN:
			break;
		case BLE_START_SCAN:
			break;
		case BLE_STOP_SCAN:
			break;
		case BLE_DELETE_SCAN:
			break;
		case BLE_CONN_UPDATE_MTU:
			break;
		case BLE_CONN_UPDATE_PARAM:
			break;
		case BLE_CONN_DIS_CONN:
			break;
		default:
			break;
	}
}

void hal_ble_notice_cb(ble_notice_t notice, void *param)
{
	bk_printf("[hal_ble_notice_cb]notice:%d\r\n",notice);
	switch (notice) {
	case BLE_5_STACK_OK:
		bk_printf("ble stack ok");
		break;
	case BLE_5_WRITE_EVENT:
	{
		write_req_t *w_req = (write_req_t *)param;
		bk_printf("BLE_5_WRITE_EVENT:conn_idx:%d, prf_id:%d, add_id:%d, len:%d, data[0]:%02x\r\n",
			w_req->conn_idx, w_req->prf_id, w_req->att_idx, w_req->len, w_req->value[0]);
		if (w_req->att_idx == FFB3S_IDX_FED6_VAL_IND_CFG)
		{
			uint16_t ind_cfg = (w_req->value[0]) | (w_req->value[1] << 8);
			breeze_cfg.ind_cfg = ind_cfg;
			bk_printf("BLE_CFG_INDICATE:0x%x\r\n", breeze_cfg.ind_cfg);
			ais_bt_init_info.ic.on_ccc_change((uint8)breeze_cfg.ind_cfg);
			///ble_event_callback(BLE_CFG_INDICATE, (void *)(&(breeze_cfg.ind_cfg)));
		}
		else if (w_req->att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG)
		{
			uint16_t ntf_cfg = (w_req->value[0]) | (w_req->value[1] << 8);
			breeze_cfg.ntf_cfg = ntf_cfg;
			bk_printf("BLE_CFG_NTFIND:0x%x\r\n", breeze_cfg.ntf_cfg);
			ais_bt_init_info.nc.on_ccc_change((uint8)breeze_cfg.ntf_cfg);
			////ble_event_callback(BLE_CFG_NOTIFY, (void *)(&(breeze_cfg.ntf_cfg)));
		}
		else if (w_req->att_idx == FEB3S_IDX_FED5_VAL_VALUE)
		{
			ais_bt_init_info.wc.on_write(w_req->value, w_req->len);
		}
		else if (w_req->att_idx == FEB3S_IDX_FED7_VAL_VALUE)
		{
			ais_bt_init_info.wwnrc.on_write(w_req->value, w_req->len);
		}
		else
		{
			bk_printf("Write Not support\r\n");
		}
		break;
	}
	case BLE_5_READ_EVENT:
	{
		read_req_t *r_req = (read_req_t *)param;
		bk_printf("BLE_5_READ_EVENT:conn_idx:%d, prf_id:%d, add_id:%d\r\n",
			r_req->conn_idx, r_req->prf_id, r_req->att_idx);
		//r_req->value[0] = 0x12;
		//r_req->value[1] = 0x34;
		//r_req->value[2] = 0x56;
		//r_req->length = 3;
		if (r_req->att_idx == FFB3S_IDX_FED6_VAL_IND_CFG)
		{
			r_req->value[0] = breeze_cfg.ind_cfg&0xFF;
			r_req->value[1] = (breeze_cfg.ind_cfg&0xFF00)>>8;
			r_req->length = 2;
		}
		else if (r_req->att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG)
		{
			r_req->value[0] = breeze_cfg.ntf_cfg&0xFF;
			r_req->value[1] = (breeze_cfg.ntf_cfg&0xFF00)>>8;
			r_req->length = 2;
		}
		else if (r_req->att_idx == FEB3S_IDX_FED4_VAL_VALUE)
		{
			r_req->length = ais_bt_init_info.rc.on_read(r_req->value, r_req->size);
		}
		else
		{
			bk_printf("Read Not Support\r\n");
		}
		break;
	}
	case BLE_5_REPORT_ADV:
	{
		recv_adv_t *r_ind = (recv_adv_t *)param;
		bk_printf("BLE_5_REPORT_ADV:actv_idx:%d, adv_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			r_ind->actv_idx, r_ind->adv_addr[0], r_ind->adv_addr[1], r_ind->adv_addr[2],
			r_ind->adv_addr[3], r_ind->adv_addr[4], r_ind->adv_addr[5]);
		break;
	}
	case BLE_5_MTU_CHANGE:
	{
		mtu_change_t *m_ind = (mtu_change_t *)param;
		bk_printf("BLE_5_MTU_CHANGE:conn_idx:%d, mtu_size:%d\r\n", m_ind->conn_idx, m_ind->mtu_size);
		break;
	}
	case BLE_5_CONNECT_EVENT:
	{
		conn_ind_t *c_ind = (conn_ind_t *)param;
		bk_printf("BLE_5_CONNECT_EVENT:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
			c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1],
			c_ind->peer_addr[2], c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
		hal_ble_env.connidx = c_ind->conn_idx;
		g_connected = 1;
		ais_bt_init_info.on_connected();
		break;
	}
	case BLE_5_DISCONNECT_EVENT:
	{
		discon_ind_t *d_ind = (discon_ind_t *)param;
		bk_printf("BLE_5_DISCONNECT_EVENT:conn_idx:%d,reason:%d\r\n", d_ind->conn_idx,d_ind->reason);
		hal_ble_env.connidx = 0xFF;
		g_connected = 0;
		ais_bt_init_info.on_disconnected();
		break;
	}
	case BLE_5_ATT_INFO_REQ:
	{
		att_info_req_t *a_ind = (att_info_req_t *)param;
		bk_printf("BLE_5_ATT_INFO_REQ:conn_idx:%d\r\n", a_ind->conn_idx);
		a_ind->length = 128;
		a_ind->status = ERR_SUCCESS;
		break;
	}
	case BLE_5_CREATE_DB:
	{
		create_db_t *cd_ind = (create_db_t *)param;
		bk_printf("BLE_5_CREATE_DB:prf_id:%d, status:%d\r\n", cd_ind->prf_id, cd_ind->status);
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
	case BLE_5_TX_DONE:
		{
			bk_printf("BLE_TX_DONE\r\n");
			if(g_indication_txdone)
			{
				(*g_indication_txdone)(AIS_ERR_SUCCESS);
			}
		}
		break;
	default:
		bk_printf("UNKNOW EVENT\r\n");
		break;
	}
}


static ble_err_t ble_create_db(void)
{
    ble_err_t status;
    struct bk_ble_db_cfg ble_db_cfg;
	bk_printf("[%s]\r\n",__FUNCTION__);

    ble_db_cfg.att_db = (bk_attm_desc_t*)feb3_att_db;
    ble_db_cfg.att_db_nb = FEB3S_IDX_NB;
    ble_db_cfg.prf_task_id = 0;
    ble_db_cfg.start_hdl = 0;
    ble_db_cfg.svc_perm = 0;
    memcpy(&(ble_db_cfg.uuid[0]), &uuid_feb3[0], 16);

    status = bk_ble_create_db(&ble_db_cfg);
	
	return status;
}

static int ble_create_adv(void)
{
	unsigned char actv_idx;
	bk_printf("[%s]\r\n",__FUNCTION__);

	actv_idx = app_ble_get_idle_actv_idx_handle();
	bk_printf("[%s]actv_idx:%d\r\n",__FUNCTION__,actv_idx);
	if(actv_idx >= BLE_ACTIVITY_MAX){
		return -1;
	}

	hal_ble_env.adv_idx = actv_idx;
	if(ERR_SUCCESS != bk_ble_create_advertising(actv_idx, hal_ble_env.channel_map, hal_ble_env.interval_min, hal_ble_env.interval_max, hal_ble_cmd_cb)){
		bk_printf("[%s]\r\n",__FUNCTION__);
		return -2;
	}

	return 0;
}

#if 0
static void ble_set_adv_data(void)
{
	int ret = 0;
	if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		ret = bk_ble_set_adv_data(hal_ble_env.adv_idx, hal_ble_env.advData, hal_ble_env.advDataLen, hal_ble_cmd_cb);;
	}
	bk_printf("[%s]ret:%d\r\n",__func__, ret);
}

static void ble_set_scan_rsp_data(void)
{
    bk_printf("[%s]\r\n",__func__);

	// Update advertising state
    if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		bk_ble_set_scan_rsp_data(hal_ble_env.adv_idx, hal_ble_env.respData, hal_ble_env.respDataLen, hal_ble_cmd_cb);
	}
}

static void ble_start_adv_cmd(void)
{
    bk_printf("[%s]\r\n",__func__);
	if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		bk_ble_start_advertising(hal_ble_env.adv_idx, 0, hal_ble_cmd_cb);
	}
}

static void ble_delete_advertising(void)
{
    bk_printf("[%s]\r\n",__func__);
	if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		bk_ble_delete_advertising(hal_ble_env.adv_idx, hal_ble_cmd_cb);
	}
}

void ble_adv_status_run(void)
{
    bk_printf("[%s] cur adv_state:%x\r\n",__func__,get_app_ble_adv_state());
    switch (get_app_ble_adv_state())
    {
        case (APP_ADV_STATE_IDLE): // 0
	        {
	            // Create advertising
	            ble_create_adv();
	        }
			break;
        case (APP_ADV_STATE_CREATING): // 1
			{
				// Set advertising data
	            ble_set_adv_data();
	        } 
			break;
        case (APP_ADV_STATE_SETTING_ADV_DATA): // 2
			{
	            // Set scan response data
	            ble_set_scan_rsp_data();
	        }
			break;
        case (APP_ADV_STATE_CREATED): // 5
        case (APP_ADV_STATE_SETTING_SCAN_RSP_DATA): // 3
			{
				// Start advertising activity
	            ble_start_adv_cmd();
	        } 
			break;
        case (APP_ADV_STATE_UPDATA_ADV_DATA): // 4
        case (APP_ADV_STATE_STARTING): // 6
	        {
	            // Go to started state
	           set_app_ble_adv_state(APP_ADV_STATE_STARTED);
	        }
			break;
        case (APP_ADV_STATE_STARTED): // 7
	        {
	            // Stop advertising activity
	            //appm_stop_advertising();
	        }
			break;
        case (APP_ADV_STATE_STOPPING): // 8
	        {
	            // Go delete state
	            ble_delete_advertising();
	        }
			break;
        case (APP_ADV_STATE_DELETEING):// 10
	        {
				set_app_ble_adv_state(APP_ADV_STATE_IDLE);
	        }
       		break;
        default:
            //BLE_ASSERT_ERR(0);
        	break;
    }
    
    bk_printf("end adv_state:%x\r\n",get_app_ble_adv_state());
}

void ble_event_callback(ble_event_t event, void *param)
{
	switch(event)	  
	{ 	   
		case BLE_COMMON_EVT:
			{
				bk_printf("BLE_COMMON_EVT\r\n");
				ble_adv_status_run();
			}
			break;
		case BLE_CREATE_DB_OK:
			{
				bk_printf("BLE_CREATE_DB_OK\r\n");			
			} 	   
			break;		 
		case BLE_CONNECT:		  
			{ 		   
				bk_printf("BLE CONNECT\r\n");			
				g_connected = 1;
				ais_bt_init_info.on_connected();		
			}		 
			break; 	   
		case BLE_DISCONNECT: 	   
			{			
				bk_printf("BLE DISCONNECT\r\n");			
				g_connected = 0;
				ais_bt_init_info.on_disconnected(); 	   
			}		
			break;		  
		case BLE_MTU_CHANGE:		  
			{ 		   
				bk_printf("BLE_MTU_CHANGE:%d\r\n", *(uint16_t *)param);		  
			} 	   
			break;		 
		case BLE_CFG_NOTIFY:		 
			{			  
				bk_printf("BLE_CFG_NTFIND:0x%x\r\n", *(uint8_t *)param);			  
				ais_bt_init_info.nc.on_ccc_change(*(uint8_t *)param); 	   
			}		
			break;		  
		case BLE_CFG_INDICATE:		
			{			 
				bk_printf("BLE_CFG_INDICATE:0x%x\r\n", *(uint8_t *)param);			 
				ais_bt_init_info.ic.on_ccc_change(*(uint8_t *)param);		  
			} 	   
			break;	
		case BLE_TX_DONE:
			{
				bk_printf("BLE_TX_DONE\r\n");
				if(g_indication_txdone)
				{
					(*g_indication_txdone)(AIS_ERR_SUCCESS);
				}
			}
			break;
		default:			 
			bk_printf("UNKNOW EVENT %d.\r\n", event); 	   
			break;	 
	}
}
#endif
ais_err_t ble_stack_init(ais_bt_init_t *ais_init)
{
	bk_printf("[%s]\r\n",__FUNCTION__);
    memcpy((uint8_t *)&ais_bt_init_info,(uint8_t *)ais_init,sizeof(ais_bt_init_t));
    memset(&breeze_cfg, 0x0, sizeof(breeze_config_t));
	ble_set_notice_cb(hal_ble_notice_cb);

	ble_create_db();
	return AIS_ERR_SUCCESS;
}

ais_err_t ble_stack_deinit()
{
    memset((uint8_t *)&ais_bt_init_info, 0, sizeof(ais_bt_init_t));

    return AIS_ERR_SUCCESS;
}

ais_err_t ble_send_notification(uint8_t *p_data, uint16_t length)
{
    ais_err_t status = AIS_ERR_SUCCESS;
	
    status = bk_ble_send_ntf_value(length, p_data, 0, FEB3S_IDX_FED8_VAL_VALUE);

    bk_printf("status:%d\r\n", status);

    return status;
}

ais_err_t ble_send_indication(uint8_t *p_data, uint16_t length, void (*txdone)(uint8_t res))
{
    ais_err_t status = AIS_ERR_SUCCESS;

    status = bk_ble_send_ind_value(length, p_data, 0, FEB3S_IDX_FED6_VAL_VALUE);
	
	g_indication_txdone = txdone;
	
    return status;
}

void ble_disconnect(uint8_t reason)
{
	bk_printf("[%s]\r\n",__func__);
	bk_ble_disconnect(hal_ble_env.connidx, hal_ble_cmd_cb);
}

static void ble_set_advertising(ais_adv_init_t *adv)
{
    unsigned char adv_idx, adv_name_len;

    adv_idx = 0;
    hal_ble_env.advData[adv_idx] = 0x02; adv_idx++;
    hal_ble_env.advData[adv_idx] = 0x01; adv_idx++;
    hal_ble_env.advData[adv_idx] = 0x06; adv_idx++;

    hal_ble_env.advData[adv_idx] = adv->vdata.len + 1; adv_idx +=1;
    hal_ble_env.advData[adv_idx] = 0xFF; adv_idx +=1;
    memcpy(&hal_ble_env.advData[adv_idx], adv->vdata.data, adv->vdata.len); adv_idx += adv->vdata.len;

    adv_name_len = strlen(adv->name.name) + 1;
    hal_ble_env.advData[adv_idx] = adv_name_len; adv_idx +=1;
    hal_ble_env.advData[adv_idx] = adv->name.ntype + 0x08; adv_idx +=1; //name
    memcpy(&hal_ble_env.advData[adv_idx], adv->name.name, strlen(adv->name.name)); adv_idx +=strlen(adv->name.name);

    hal_ble_env.advDataLen = adv_idx;

    adv_idx = 0;

    hal_ble_env.respData[adv_idx] = adv_name_len; adv_idx +=1;
    hal_ble_env.respData[adv_idx] = adv->name.ntype + 0x07; adv_idx +=1; //name
    memcpy(&hal_ble_env.respData[adv_idx], adv->name.name, strlen(adv->name.name)); adv_idx +=strlen(adv->name.name);
    hal_ble_env.respDataLen = adv_idx;
}

ais_err_t ble_advertising_start(ais_adv_init_t *adv)
{
    ais_err_t status = AIS_ERR_SUCCESS;

	ble_set_advertising(adv);
	if(hal_ble_env.adv_idx == 0xFF){
		if(ble_create_adv() != 0){
			status = AIS_ERR_ADV_FAIL;
		}
	}
	else if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		bk_ble_set_adv_data(hal_ble_env.adv_idx, hal_ble_env.advData, hal_ble_env.advDataLen, hal_ble_cmd_cb);
	}
    else{
        status = AIS_ERR_ADV_FAIL;
    }

    return status;
}

ais_err_t ble_advertising_stop(void)
{
	ais_err_t status = AIS_ERR_SUCCESS;
	bk_printf("[%s]\r\n",__func__);

	if(hal_ble_env.adv_idx < BLE_ACTIVITY_MAX){
		bk_ble_stop_advertising(hal_ble_env.adv_idx, hal_ble_cmd_cb);
	}else{
		status = AIS_ERR_STOP_ADV_FAIL;	
	}

    return  status;
}

ais_err_t ble_get_mac(uint8_t *mac)
{
    bk_printf("%s\r\n", __func__);
    ais_err_t status = AIS_ERR_SUCCESS;
    
    memcpy(mac, &common_default_bdaddr, AIS_BT_MAC_LEN);

    return status;
}

#ifdef EN_LONG_MTU
int ble_get_att_mtu(uint16_t *att_mtu)
{
    if(att_mtu == NULL || g_connected == 0){
        bk_printf("Failed to get ble connection\r\n");
        return -1;
    }
	if(hal_ble_env.connidx >= BLE_CONNECTION_MAX)
		return -1;
	*att_mtu = bk_ble_get_conn_mtu(hal_ble_env.connidx);
    ///*att_mtu = gattc_get_mtu(hal_ble_env.connidx);
    return 0;
}
#endif

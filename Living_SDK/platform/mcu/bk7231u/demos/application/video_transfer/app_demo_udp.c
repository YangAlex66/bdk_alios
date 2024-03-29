#include "include.h"
#include "app_demo_config.h"

#if (APP_DEMO_VIDEO_TRANSFER && APP_DEMO_CFG_USE_UDP)
//#include "FreeRTOS.h"
//#include "task.h"
#include "rtos_pub.h"
#include "error.h"
#include "lwip/sockets.h"

#include "app_demo_udp.h"
#include "BkDriverUart.h"

#include "uart_pub.h"
#include "mem_pub.h"

#include "app_demo_config.h"
#include "app_demo_softap.h"
#include "video_transfer.h"


#define APP_DEMO_UDP_DEBUG              1
#if APP_DEMO_UDP_DEBUG
#define APP_DEMO_UDP_PRT                warning_prf
#define APP_DEMO_UDP_WARN               warning_prf
#define APP_DEMO_UDP_FATAL              fatal_prf
#else
#define APP_DEMO_UDP_PRT                null_prf
#define APP_DEMO_UDP_WARN               null_prf
#define APP_DEMO_UDP_FATAL              null_prf
#endif


#define APP_DEMO_UDP_RCV_BUF_LEN        1472
#define APP_DEMO_UDP_SOCKET_TIMEOUT     100  // ms

int app_demo_udp_img_fd = -1;
volatile int app_demo_udp_romote_connected = 0;
volatile int app_demo_udp_run = 0;
beken_thread_t app_demo_udp_hdl = NULL;
struct sockaddr_in *app_demo_remote = NULL;

typedef struct tvideo_hdr_st
{
    UINT8 id;
    UINT8 is_eof;
    UINT8 pkt_cnt; 
    UINT8 size;
#if SUPPORT_TIANZHIHENG_DRONE
    UINT32 unused;
#endif
}HDR_ST, *HDR_PTR;

void app_demo_add_pkt_header(TV_HDR_PARAM_PTR param)
{
    HDR_PTR elem_tvhdr = (HDR_PTR)param->ptk_ptr;

    elem_tvhdr->id = (UINT8)param->frame_id;
    elem_tvhdr->is_eof = param->is_eof;
    elem_tvhdr->pkt_cnt = param->frame_len;
    elem_tvhdr->size = 0;

#if SUPPORT_TIANZHIHENG_DRONE
    elem_tvhdr->unused = 0;
#endif
}

static void app_demo_udp_handle_cmd_data(UINT8 *data, UINT16 len)
{
    uint8_t crc_cal;
    uint32_t wrlen = 0;
    
    if((data[0] != CMD_HEADER_CODE) && (len != CMD_LEN) && (data[len-1] != CMD_TAIL_CODE))
        return;

    crc_cal = (data[1]^data[2]^data[3]^data[4]^data[5]);
    
    if(crc_cal != data[6]) {
        if(((crc_cal == CMD_HEADER_CODE) || (crc_cal == CMD_TAIL_CODE)) 
            && (crc_cal+1 == data[6]))
            // drop this paket for crc is the same with Header or Tailer
            return;
        else // change to right crc
            data[6] = crc_cal;
    }

    {
    extern void bk_send_byte(UINT8 uport, UINT8 data);
    for (int i=0; i<len; i++)
    {
        bk_send_byte(UART1_PORT, data[i]);
    }
    }
}

static void app_demo_udp_app_connected(void)
{
    app_demo_softap_send_msg(DAP_APP_CONECTED, 0);
}

static void app_demo_udp_app_disconnected(void)
{
    app_demo_softap_send_msg(DAP_APP_DISCONECTED, 0);
}

#if CFG_SUPPORT_HTTP_OTA
TV_OTA_ST ota_param = 
{
    NULL,
    0,
    0
};
static void app_demo_udp_http_ota_handle(char *rev_data)
{

    if(app_demo_softap_is_ota_doing() == 0)
    {
        // to do
        //
        
        app_demo_softap_send_msg(DAP_START_OTA, (u32)&ota_param);

        os_memset(&ota_param, 0, sizeof(TV_OTA_ST));
    }
}
#endif

static void app_demo_udp_receiver(UINT8 *data, UINT32 len, struct sockaddr_in *app_demo_remote)
{
    GLOBAL_INT_DECLARATION();
    
    if(len < 2)
        return;
    
    if(data[0] == CMD_IMG_HEADER) {
        if(data[1] == CMD_START_IMG) {
 
            UINT8 *src_ipaddr = (UINT8 *)&app_demo_remote->sin_addr.s_addr;
            APP_DEMO_UDP_PRT("src_ipaddr: %d.%d.%d.%d\r\n", src_ipaddr[0], src_ipaddr[1],
                                                   src_ipaddr[2], src_ipaddr[3]);
            APP_DEMO_UDP_PRT("udp connect to new port:%d\r\n", app_demo_remote->sin_port);

            GLOBAL_INT_DISABLE();
            app_demo_udp_romote_connected = 1;
            GLOBAL_INT_RESTORE();    

            #if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)
            TVIDEO_SETUP_DESC_ST setup;

            setup.open_type = TVIDEO_OPEN_SCCB;
            setup.send_type = TVIDEO_SND_UDP;
            setup.send_func = app_demo_udp_send_packet;
            setup.start_cb = app_demo_udp_app_connected;
            setup.end_cb = app_demo_udp_app_disconnected;

            setup.pkt_header_size = sizeof(HDR_ST);
            setup.add_pkt_header = app_demo_add_pkt_header;
            
            video_transfer_init(&setup);
            #endif
        }
        else if(data[1] == CMD_STOP_IMG) {
            APP_DEMO_UDP_PRT("udp close\r\n");

            #if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)  
            video_transfer_deinit();               
            #endif

            GLOBAL_INT_DISABLE();
            app_demo_udp_romote_connected = 0;
            GLOBAL_INT_RESTORE(); 
        }
        #if CFG_SUPPORT_HTTP_OTA
        else if(data[1] == CMD_START_OTA)
        {
            app_demo_udp_http_ota_handle(&data[2]);
        }
        #endif		
    }

}

static void app_demo_udp_main( beken_thread_arg_t data )
{
    OSStatus err = kNoErr;
    GLOBAL_INT_DECLARATION();
    int maxfd, udp_cmd_fd = -1, ret = 0;
    int snd_len = 0, rcv_len = 0;
    struct sockaddr_in cmd_remote;
    socklen_t srvaddr_len = 0;
    fd_set watchfd;
    struct timeval timeout;
    u8 *rcv_buf = NULL;

    APP_DEMO_UDP_FATAL("app_demo_udp_main entry\r\n");
    (void)(data);

    rcv_buf = (u8*) os_malloc((APP_DEMO_UDP_RCV_BUF_LEN + 1) * sizeof(u8));
    if(!rcv_buf) {
        APP_DEMO_UDP_PRT("udp os_malloc failed\r\n");
        goto app_udp_exit;
    }

    app_demo_remote = (struct sockaddr_in *)os_malloc(sizeof(struct sockaddr_in));
    if(!app_demo_remote) {
        APP_DEMO_UDP_PRT("udp os_malloc failed\r\n");
        goto app_udp_exit;
    }    

    // for data transfer
    app_demo_udp_img_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (app_demo_udp_img_fd == -1) {
        APP_DEMO_UDP_PRT("socket failed\r\n");
        goto app_udp_exit;
    }
 
    app_demo_remote->sin_family = AF_INET;
    app_demo_remote->sin_port = htons(APP_DEMO_UDP_IMG_PORT);
    app_demo_remote->sin_addr.s_addr = htonl(INADDR_ANY);

    srvaddr_len = (socklen_t)sizeof(struct sockaddr_in);
    if (bind(app_demo_udp_img_fd, (struct sockaddr *)app_demo_remote, srvaddr_len) == -1) {
        APP_DEMO_UDP_PRT("bind failed\r\n");
        goto app_udp_exit;
    }

    //  for recv uart cmd 
    udp_cmd_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_cmd_fd == -1) {
        APP_DEMO_UDP_PRT("socket failed\r\n");
        goto app_udp_exit;
    }
 
    cmd_remote.sin_family = AF_INET;
    cmd_remote.sin_port = htons(APP_DEMO_UDP_CMD_PORT);
    cmd_remote.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udp_cmd_fd, (struct sockaddr *)&cmd_remote, srvaddr_len) == -1) {
        APP_DEMO_UDP_PRT("bind failed\r\n");
        goto app_udp_exit;
    }

    maxfd = (udp_cmd_fd > app_demo_udp_img_fd)? udp_cmd_fd : app_demo_udp_img_fd;

    timeout.tv_sec = APP_DEMO_UDP_SOCKET_TIMEOUT / 1000;
    timeout.tv_usec = (APP_DEMO_UDP_SOCKET_TIMEOUT % 1000) * 1000;

    GLOBAL_INT_DISABLE();
    app_demo_udp_romote_connected = 0;
    app_demo_udp_run = 1;
    GLOBAL_INT_RESTORE(); 

    while (app_demo_udp_run)
    {    
        FD_ZERO(&watchfd);
        FD_SET(app_demo_udp_img_fd, &watchfd);
        FD_SET(udp_cmd_fd, &watchfd);
        
        ret = select(maxfd+1, &watchfd, NULL, NULL, &timeout);
        if (ret < 0) {
            APP_DEMO_UDP_PRT("select ret:%d\r\n", ret);
            break;
        } 
        else 
        {
            // is img fd, data transfer
            if(FD_ISSET(app_demo_udp_img_fd, &watchfd)) 
            { 
                rcv_len = recvfrom(app_demo_udp_img_fd, rcv_buf, APP_DEMO_UDP_RCV_BUF_LEN, 0,
                    (struct sockaddr *)app_demo_remote, &srvaddr_len);
                
                if(rcv_len <= 0) 
                {
                    // close this socket
                    APP_DEMO_UDP_PRT("recv close fd:%d\r\n", app_demo_udp_img_fd);
                    break;
                } 
                else 
                {
                    rcv_len = (rcv_len > APP_DEMO_UDP_RCV_BUF_LEN)? APP_DEMO_UDP_RCV_BUF_LEN: rcv_len;
                    rcv_buf[rcv_len] = 0;

                    app_demo_udp_receiver(rcv_buf, rcv_len, app_demo_remote);
                }
            }
            else if(FD_ISSET(udp_cmd_fd, &watchfd)) 
            {
                rcv_len = recvfrom(udp_cmd_fd, rcv_buf, APP_DEMO_UDP_RCV_BUF_LEN, 0,
                    (struct sockaddr *)&cmd_remote, &srvaddr_len);
                
                if(rcv_len <= 0) 
                {
                    // close this socket
                    APP_DEMO_UDP_PRT("recv close fd:%d\r\n", udp_cmd_fd);
                    break;
                } 
                else 
                {
                    rcv_len = (rcv_len > APP_DEMO_UDP_RCV_BUF_LEN)? APP_DEMO_UDP_RCV_BUF_LEN: rcv_len;
                    rcv_buf[rcv_len] = 0;
                    
                    app_demo_udp_handle_cmd_data(rcv_buf, rcv_len);
                }

            }

        }
    }
	
app_udp_exit:
    
    APP_DEMO_UDP_FATAL("app_demo_udp_main exit %d\r\n", app_demo_udp_run);

#if (CFG_USE_SPIDMA || CFG_USE_CAMERA_INTF)
    video_transfer_deinit();
#endif

    if(rcv_buf) {
        os_free(rcv_buf);
        rcv_buf = NULL;
    }

    if(app_demo_remote) {
        os_free(app_demo_remote);
        app_demo_remote = NULL;
    }
    
    if(app_demo_udp_img_fd != -1) {
        close(app_demo_udp_img_fd);
        app_demo_udp_img_fd = -1;
    }

    if(udp_cmd_fd != -1) {
        close(udp_cmd_fd);
        udp_cmd_fd = -1;
    }    

    GLOBAL_INT_DISABLE();
    app_demo_udp_romote_connected = 0;
    app_demo_udp_run = 0;
    GLOBAL_INT_RESTORE(); 

    app_demo_udp_hdl = NULL;
    rtos_delete_thread(NULL);
}

UINT32 app_demo_udp_init(void)
{
    int ret;

    os_printf("dasfaf\r\n");
    APP_DEMO_UDP_PRT("app_demo_udp_init\r\n");
    if(!app_demo_udp_hdl)
    {
        ret = rtos_create_thread(&app_demo_udp_hdl,
                                      4,
                                      "app_udp",
                                      (beken_thread_function_t)app_demo_udp_main,
                                      1024,
                                      (beken_thread_arg_t)NULL);
        if (ret != kNoErr)
        {
            APP_DEMO_UDP_PRT("Error: Failed to create spidma_intfer: %d\r\n", ret);
            return kGeneralErr;
        }
    }

    return kNoErr;
}

int app_demo_udp_send_packet (UINT8 *data, UINT32 len)
{
    int send_byte = 0;

    if(!app_demo_udp_romote_connected)
        return 0;

    send_byte = sendto(app_demo_udp_img_fd, data, len, MSG_DONTWAIT|MSG_MORE,
        (struct sockaddr *)app_demo_remote, sizeof(struct sockaddr_in));
    
    if (send_byte < 0) {
        /* err */
        //APP_DEMO_UDP_PRT("send return fd:%d\r\n", send_byte);
        send_byte = 0;
    }

	return send_byte;
}

void app_demo_udp_deinit(void)
{
    GLOBAL_INT_DECLARATION();

    APP_DEMO_UDP_PRT("app_demo_udp_deinit\r\n");
    if(app_demo_udp_run == 0)
        return;

    GLOBAL_INT_DISABLE();
    app_demo_udp_run = 0;
    GLOBAL_INT_RESTORE();

    while(app_demo_udp_hdl)
        rtos_delay_milliseconds(10);
}

#endif  // (APP_DEMO_VIDEO_TRANSFER && APP_DEMO_CFG_USE_UDP)

// EOF


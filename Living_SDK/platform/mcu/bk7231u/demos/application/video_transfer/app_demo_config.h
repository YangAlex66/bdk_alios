#ifndef __APP_DEMO_CONFIG_H__
#define __APP_DEMO_CONFIG_H__

#include "demos_config.h"

#if APP_DEMO_VIDEO_TRANSFER
#include "include.h"

#if (CFG_USE_APP_DEMO_VIDEO_TRANSFER == 0)
    #error "video transfer need open CFG_USE_APP_DEMO_VIDEO_TRANSFER"
#endif

#if ((CFG_USE_CAMERA_INTF == 0) && ((CFG_USE_SPIDMA == 0) && (CFG_USE_HSLAVE_SPI == 0)))
    #error "video transfer need open jpeg / hspi moudel"
#endif

#if (((CFG_USE_SPIDMA == 0) && (CFG_USE_HSLAVE_SPI == 1)) || ((CFG_USE_SPIDMA == 1) && (CFG_USE_HSLAVE_SPI == 0)))
    #error "if use hspi, need open CFG_USE_SPIDMA and CFG_USE_HSLAVE_SPI"
#endif

#if ((CFG_SOC_NAME != SOC_BK7231U) && (CFG_SOC_NAME != SOC_BK7221U))
    #if (CFG_USE_CAMERA_INTF == 1)
    #error "chip not support CFG_USE_CAMERA_INTF"
    #endif
#endif

#endif // APP_DEMO_VIDEO_TRANSFER


#define SUPPORT_TIANZHIHENG_DRONE         0


#if SUPPORT_TIANZHIHENG_DRONE
#define APP_DEMO_SOFTAP_DEF_SSID          "WIFI_FPV_000000"
#define APP_DEMO_SOFTAP_DEF_NET_IP        "192.168.4.153"
#define APP_DEMO_SOFTAP_DEF_NET_MASK      "255.255.255.0"
#define APP_DEMO_SOFTAP_DEF_NET_GW        "192.168.4.153"
#define APP_DEMO_SOFTAP_DEF_CHANNEL       1   

#define APP_DEMO_UDP_CMD_PORT             8090
#define APP_DEMO_UDP_IMG_PORT             8080	
#define CMD_IMG_HEADER                    0x42
#define CMD_START_IMG                     0x76
#define CMD_STOP_IMG                      0x77
#define CMD_START_OTA                     0x38

#define APP_DEMO_TCP_SERVER_PORT          8050
#define APP_DEMO_TCP_SERVER_PORT_VICOE    8040 

#define APP_DEMO_UDP_VOICE_PORT           8070 
#define CMD_VOICE_HEADER                  0x30
#define CMD_VOICE_START                   0x66
#define CMD_VOICE_STOP                    0x67
#define APP_DEMO_CFG_USE_TCP              1
#define APP_DEMO_CFG_USE_UDP              1

#define CMD_HEADER_CODE                   0x66
#define CMD_TAIL_CODE                     0x99
#define CMD_LEN                           8
#define APP_DEMO_TCP_LISTEN_MAX           1  

#if (CFG_USE_AUDIO && CFG_USE_AUD_ADC)
#define APP_DEMO_EN_VOICE_TRANSFER        1
#else
#define APP_DEMO_EN_VOICE_TRANSFER        0
#endif // (CFG_SOC_NAME == SOC_BK7221U)

#else  // SUPPORT_TIANZHIHENG_DRONE

// for softap configuration
#define APP_DEMO_SOFTAP_DEF_SSID          "BEKEN_WIFI_000000"
#define APP_DEMO_SOFTAP_DEF_NET_IP        "192.168.1.1"
#define APP_DEMO_SOFTAP_DEF_NET_MASK      "255.255.255.0"
#define APP_DEMO_SOFTAP_DEF_NET_GW        "192.168.1.1"
#define APP_DEMO_SOFTAP_DEF_CHANNEL       1   

#define APP_DEMO_UDP_CMD_PORT             7090 
#define APP_DEMO_UDP_IMG_PORT             7080 
#define CMD_IMG_HEADER                    0x20
#define CMD_START_IMG                     0x36
#define CMD_STOP_IMG                      0x37
#define CMD_START_OTA                     0x38
  
#define APP_DEMO_TCP_SERVER_PORT          7050
#define APP_DEMO_TCP_SERVER_PORT_VICOE    7040 

#define APP_DEMO_UDP_VOICE_PORT           7070 
#define CMD_VOICE_HEADER                  0x30
#define CMD_VOICE_START                   0x66
#define CMD_VOICE_STOP                    0x67

#define APP_DEMO_CFG_USE_TCP              0
#define APP_DEMO_CFG_USE_UDP              1

#define CMD_HEADER_CODE                   0x66
#define CMD_TAIL_CODE                     0x99
#define CMD_LEN                           8
#define APP_DEMO_TCP_LISTEN_MAX           1

#if (CFG_USE_AUDIO && CFG_USE_AUD_ADC)
#define APP_DEMO_EN_VOICE_TRANSFER        1
#else
#define APP_DEMO_EN_VOICE_TRANSFER        0
#endif // (CFG_SOC_NAME == SOC_BK7221U)

#endif  // SUPPORT_TIANZHIHENG_DRONE

#endif // __APP_DEMO_CONFIG_H__

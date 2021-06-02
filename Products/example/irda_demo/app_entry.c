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

#include "hal/soc/irda.h"

#define GET_KEY_TYPE(msg) ((msg >> 24) & 0xff)
#define GET_KEY_VALUE(msg) (msg & 0xff)

typedef void (*irkey_handle_func)(void);
#define IR_USER_CODE    (0x7f80) /*each remote-contrl unit has its own user_code*/
#define IR_CODE_SIZE    (0xde)
enum
{
	IR_KEY_POWEROFF = 0,/*0xdd*/
	IR_KEY_POWERON,     /*0x9c*/
	IR_KEY_VOL_PLUS,    /*0x99*/
	IR_KEY_VOL_MINUS,   /*0xc5*/
	IR_KEY_PREV,        /*0xce*/
	IR_KEY_NEXT,        /*0xd2*/
	IR_KEY_PLAY,        /*0xc1*/
	IR_KEY_MAX,
};

enum
{
	IR_KEY_TYPE_SHORT = 0,
	IR_KEY_TYPE_LONG,
	IR_KEY_TYPE_HOLD,
	IR_KEY_TYPE_MAX,
};
const static uint8_t IR_key_map[IR_CODE_SIZE] =
{
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,IR_KEY_VOL_PLUS,0xff,0xff,IR_KEY_POWERON,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,IR_KEY_PLAY,0xff,0xff,0xff,IR_KEY_VOL_MINUS,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,IR_KEY_PREV,0xff,
	0xff,0xff,IR_KEY_NEXT,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,IR_KEY_POWEROFF
};
/*IR_KEY_POWEROFF*/
static void poweroff_short_handler(void)
{
	LOG("key poweroff short handler\r\n");
}
static void poweroff_long_handler(void)
{
	LOG("key poweroff long handler\r\n");
}
static void poweroff_hold_handler(void)
{
	LOG("key poweroff hold handler\r\n");
}
/*IR_KEY_POWERON*/
static void poweron_short_handler(void)
{
	LOG("key poweron short handler\r\n");
}
static void poweron_long_handler(void)
{
	LOG("key poweron long handler\r\n");
}
static void poweron_hold_handler(void)
{
	LOG("key poweron hold handler\r\n");
}
/*IR_KEY_VOL_PLUS*/
static void volp_short_handler(void)
{
	LOG("key volp short handler\r\n");
}
static void volp_long_handler(void)
{
	LOG("key volp long handler\r\n");
}
static void volp_hold_handler(void)
{
	LOG("key volp hold handler\r\n");
}
/*IR_KEY_VOL_MINUS*/
static void volm_short_handler(void)
{
	LOG("key volm short handler\r\n");
}
static void volm_long_handler(void)
{
	LOG("key volm long handler\r\n");
}
static void volm_hold_handler(void)
{
	LOG("key volm hold handler\r\n");
}
/*IR_KEY_PREV*/
static void prev_short_handler(void)
{
	LOG("key prev short handler\r\n");
}
static void prev_long_handler(void)
{
	LOG("key prev long handler\r\n");
}
static void prev_hold_handler(void)
{
	LOG("key prev hold handler\r\n");
}
/*IR_KEY_NEXT*/
static void next_short_handler(void)
{
	LOG("key next short handler\r\n");
}
static void next_long_handler(void)
{
	LOG("key next long handler\r\n");
}
static void next_hold_handler(void)
{
	LOG("key next hold handler\r\n");
}

/*IR_KEY_PLAY*/
static void play_short_handler(void)
{
	LOG("key play short handler\r\n");
}
static void play_long_handler(void)
{
	LOG("key play long handler\r\n");
}
static void play_hold_handler(void)
{
	LOG("key play hold handler\r\n");
}

const static irkey_handle_func IRKey_handler[IR_KEY_TYPE_MAX][IR_KEY_MAX] =
{
	{
		poweroff_short_handler,
		poweron_short_handler,
		volp_short_handler,
		volm_short_handler,
		prev_short_handler,
		next_short_handler,
		play_short_handler
	},

	{
		poweroff_long_handler,
		poweron_long_handler,
		volp_long_handler,
		volm_long_handler,
		prev_long_handler,
		next_long_handler,
		play_long_handler
	},

	{
		poweroff_hold_handler,
		poweron_hold_handler,
		volp_hold_handler,
		volm_hold_handler,
		prev_hold_handler,
		next_hold_handler,
		play_hold_handler
	}
};

void ir_key_handle_thread(void *parameter)
{
	uint32_t IR_msg;
	uint8_t key_type;
	uint8_t key_value;
	uint8_t ir_code;
	while (1) {
		if (hal_irda_get_key((void *)&IR_msg, sizeof(IR_msg), AOS_WAIT_FOREVER) == 0) {
			key_type = GET_KEY_TYPE(IR_msg);
			ir_code = GET_KEY_VALUE(IR_msg);
			if (ir_code < IR_CODE_SIZE) {
				key_value = IR_key_map[ir_code];
				if ((key_type < IR_KEY_TYPE_MAX) && (key_value < IR_KEY_MAX)) {
					IRKey_handler[key_type][key_value]();
				}
			}
		}
	}
}

static void handle_irda_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	hal_irda_set_usrcode(IR_USER_CODE);
	hal_irda_init_app();

	aos_task_new("ir_key", ir_key_handle_thread, NULL, 1024);
}

static struct cli_command irda_cmd = {.name = "irda_test",
                                      .help = "irda test",
                                      .function = handle_irda_cmd};

static void show_firmware_version(void)
{
	printf("\n--------Firmware info--------");
	printf("\napp: %s,  board: %s", APP_NAME, PLATFORM);
#ifdef DEBUG
	printf("\nHost: %s", COMPILE_HOST);
#endif
	printf("\nBranch: %s", GIT_BRANCH);
	printf("\nHash: %s", GIT_HASH);
	printf("\nDate: %s %s", __DATE__, __TIME__);
	printf("\nKernel: %s", aos_get_kernel_version());
	printf("\nLinkKit: %s", LINKKIT_VERSION);
	printf("\nAPP ver: %s", aos_get_app_version());

	printf("\nRegion env: %s\n\n", REGION_ENV_STRING);
}

int application_start(int argc, char **argv)
{
	IOT_SetLogLevel(IOT_LOG_INFO);
	show_firmware_version();

	aos_cli_register_command(&irda_cmd);

	aos_loop_run();

	return 0;
}

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

#include <hal/soc/soc.h>

#define ADC_TEST_CHANN  3

static void adc_timer_cb(void* arg)
{
	uint16_t out;

	hal_adc_value_get(NULL, &out, 0);
	bk_printf("%d.\r\n", out);
}

static void handle_adc_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	adc_dev_t adc_dev;
	timer_dev_t tmr;
	uint16_t adc_output;

	adc_dev.port = ADC_TEST_CHANN;
	adc_dev.config.sampling_cycle = 0x22;
	hal_adc_init(&adc_dev);

	tmr.port = 0;
	tmr.config.period = 1000;
	tmr.config.reload_mode = TIMER_RELOAD_AUTO;
	tmr.config.cb = adc_timer_cb;
	tmr.config.arg = (void *)&adc_output;
	hal_timer_us_init(&tmr);
	hal_timer_us_start(&tmr);
}

static struct cli_command adc_cmd = {.name = "adc_test",
                                     .help = "adc test",
                                     .function = handle_adc_cmd};

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

	aos_cli_register_command(&adc_cmd);

	aos_loop_run();

	return 0;
}

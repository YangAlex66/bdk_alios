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

#include "app_entry.h"
#include "beken_test_cmds.h"

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
    show_firmware_version();

    netmgr_init();

    beken_test_register_cmds();

    aos_loop_run();

    return 0;
}

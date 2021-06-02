/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <aos/aos.h>
#include <aos/yloop.h>
#include "netmgr.h"
#include "app_entry.h"

#include <k_api.h>
#include "hal/soc/soc.h"

#define SPI_TEST_LENGTH    (255)
#define SPI_BAUDRATE       (8* 1000 * 1000)

struct spi_info
{
	uint8_t *send_buf;
	uint32_t send_len;

	uint8_t *recv_buf;
	uint32_t recv_len;
};

static void handle_spi_test_cmd(char *pwbuf, int blen, int argc, char **argv)
{
	spi_dev_t spi_dev;
	uint16_t tx_len, rx_len;
	int i, ret = 0;

	uint8_t *write_data;
	uint8_t *receive_data;

	struct spi_info spi_msg;

	spi_dev.port = 0;

	if (strcmp(argv[1], "master") == 0) {
		spi_dev.config.mode = HAL_SPI_MODE_MASTER;
	} else if (strcmp(argv[1], "slave") == 0) {
		spi_dev.config.mode = HAL_SPI_MODE_SLAVE;
	} else {
		aos_cli_printf("spi_test master/salve tx/rx len\r\n");
	}

	if (argc == 5) {
		spi_dev.config.freq = atoi(argv[4]);
	} else {
		spi_dev.config.freq = SPI_BAUDRATE;
	}

	aos_cli_printf("freq:%d\r\n",spi_dev.config.freq);

	if (strcmp(argv[2], "tx") == 0) {
		if (argc < 4) {
			tx_len = SPI_TEST_LENGTH;
		} else {
			tx_len = atoi(argv[3]);
		}
		aos_cli_printf("spi tx_len:%d\r\n", tx_len);

		write_data = (uint8_t *)aos_malloc(tx_len);
		if (!write_data) {
			aos_cli_printf("send msg allocate fail\n");
			goto _exit;
		}

		for (i = 0; i < tx_len; i++) {
			write_data[i] = 0x00 + i;
		}

		spi_msg.send_buf = write_data;
		spi_msg.send_len = tx_len;
		spi_msg.recv_buf = NULL;
		spi_msg.recv_len = 0;
		spi_dev.priv = &spi_msg;

		hal_spi_init(&spi_dev);

		ret = hal_spi_send(&spi_dev, write_data, tx_len, AOS_WAIT_FOREVER);

		aos_cli_printf("ret = %d\r\n", ret);

		for (i = 0; i < tx_len; i++) {
			aos_cli_printf("write_data[%d]=%x\r\n", i, write_data[i]);
		}

		aos_free(write_data);
	} else if (strcmp(argv[2], "rx") == 0) {
		if (argc < 4) {
			rx_len = SPI_TEST_LENGTH;
		} else {
			rx_len = atoi(argv[3]);
		}

		aos_cli_printf("spi rx_len:%d\r\n", rx_len);

		receive_data = (uint8_t *)aos_malloc(rx_len);
		if (!receive_data) {
			aos_cli_printf("recev msg allocate fail\n");
			goto _exit;
		}

		spi_msg.send_buf = NULL;
		spi_msg.send_len = 0;
		spi_msg.recv_buf = receive_data;
		spi_msg.recv_len = rx_len;
		spi_dev.priv = &spi_msg;

		hal_spi_init(&spi_dev);

		ret = hal_spi_recv(&spi_dev, receive_data, rx_len, AOS_WAIT_FOREVER);

		aos_cli_printf("ret = %d\r\n", ret);

		for (i=0; i<rx_len; i++) {
			aos_cli_printf("receive_data[%d]=%x\r\n", i, receive_data[i]);
		}

		aos_free(receive_data);
	}

_exit:
	if (ret) {
		hal_spi_finalize(&spi_dev);
		aos_cli_printf("\r\nspi test error\r\n");
	}
}

static struct cli_command spi_test_cmd = {.name     = "spi_test",
                                          .help     = "spi_test [start]",
                                          .function = handle_spi_test_cmd};

int application_start(int argc, char **argv)
{
	aos_set_log_level(AOS_LL_DEBUG);

	aos_cli_register_command(&spi_test_cmd);

	aos_loop_run();

	return 0;
}

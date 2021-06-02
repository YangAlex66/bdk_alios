
#! /bin/bash
# generate SDK for specified platform after a new git clone

source ./tools/scripts/build_include.sh

if [ "$1" == "" ]; then
	target_mcu="bk7231n"
else
	target_mcu="$1"
fi
target_type="example"
target_app="beken_test"
target_board="${target_mcu}devkitc"
target_region=MAINLAND
target_env=ONLINE
target_debug=0

BEKEN_SDK_DIR=Living_SDK/platform/mcu/${target_mcu}/beken
BEKEN_LIB_DIR=Living_SDK/out/$target_app@$target_board/libraries
BEKEN_TEST_BLE_CFG_FILE=Products/example/beken_test/beken_test_ble.h

function libs_cp()
{
	cp ${BEKEN_LIB_DIR}/beken.a ${BEKEN_SDK_DIR}/
	cp ${BEKEN_LIB_DIR}/ip.a ${BEKEN_SDK_DIR}/ip/

	mkdir ${BEKEN_SDK_DIR}/func/rf_test
        cp ${BEKEN_LIB_DIR}/rf_test.a ${BEKEN_SDK_DIR}/func/rf_test/

	mkdir ${BEKEN_SDK_DIR}/func/rf_use
        cp ${BEKEN_LIB_DIR}/rf_use.a ${BEKEN_SDK_DIR}/func/rf_use/

	mkdir ${BEKEN_SDK_DIR}/func/uart_debug
        cp ${BEKEN_LIB_DIR}/uart_debug.a ${BEKEN_SDK_DIR}/func/uart_debug/

	mkdir ${BEKEN_SDK_DIR}/func/bk7011_cal
        cp ${BEKEN_LIB_DIR}/bk7011_cal.a ${BEKEN_SDK_DIR}/func/bk7011_cal/

	mkdir ${BEKEN_SDK_DIR}/func/wpa_supplicant_2_9
        cp ${BEKEN_LIB_DIR}/wpa_supplicant_2_9.a ${BEKEN_SDK_DIR}/func/wpa_supplicant_2_9/

	if [ "$target_mcu" == "bk7231u" ]; then
		mkdir ${BEKEN_SDK_DIR}/driver/ble
		cp ${BEKEN_LIB_DIR}/ble.a ${BEKEN_SDK_DIR}/driver/ble/
	else
		mkdir ${BEKEN_SDK_DIR}/driver/ble_5_x_rw
		cp ${BEKEN_LIB_DIR}/ble.a ${BEKEN_SDK_DIR}/driver/ble_5_x_rw/
	fi
}

function files_rm()
{
	rm -rf ${BEKEN_SDK_DIR}/alios/entry/arch_main.c
	rm -rf ${BEKEN_SDK_DIR}/alios/lwip-2.0.2/apps/ping
	rm -rf ${BEKEN_SDK_DIR}/alios/lwip-2.0.2/port/*.c
	rm -rf ${BEKEN_SDK_DIR}/alios/os/*.c
	rm -rf ${BEKEN_SDK_DIR}/alios/flash_hal.c
	
	rm -rf ${BEKEN_SDK_DIR}/app/ftp
	rm -rf ${BEKEN_SDK_DIR}/app/http
	rm -rf ${BEKEN_SDK_DIR}/app/led
	rm -rf ${BEKEN_SDK_DIR}/app/standalone-ap
	rm -rf ${BEKEN_SDK_DIR}/app/standalone-station
	rm -rf ${BEKEN_SDK_DIR}/app/tftp
	rm -rf ${BEKEN_SDK_DIR}/app/app.c
	rm -rf ${BEKEN_SDK_DIR}/app/app.h
	rm -rf ${BEKEN_SDK_DIR}/app/config/param_config.c
	rm -rf ${BEKEN_SDK_DIR}/app/config/sys_config_*.h
	rm -rf ${BEKEN_SDK_DIR}/app/avi_work
	rm -rf ${BEKEN_SDK_DIR}/app/net_work
	
	rm -rf ${BEKEN_SDK_DIR}/bugzilla
	rm -rf ${BEKEN_SDK_DIR}/README.md
	rm -rf ${BEKEN_SDK_DIR}/demo
	
	rm -rf ${BEKEN_SDK_DIR}/driver/audio
	rm -rf ${BEKEN_SDK_DIR}/driver/ble
	rm -rf ${BEKEN_SDK_DIR}/driver/ble_5_x_rw	
	rm -rf ${BEKEN_SDK_DIR}/driver/calendar
	rm -rf ${BEKEN_SDK_DIR}/driver/codec
	rm -rf ${BEKEN_SDK_DIR}/driver/common/*.c
	rm -rf ${BEKEN_SDK_DIR}/driver/dma
	rm -rf ${BEKEN_SDK_DIR}/driver/entry
	rm -rf ${BEKEN_SDK_DIR}/driver/fft
	rm -rf ${BEKEN_SDK_DIR}/driver/flash
	rm -rf ${BEKEN_SDK_DIR}/driver/general_dma
	rm -rf ${BEKEN_SDK_DIR}/driver/gpio
	rm -rf ${BEKEN_SDK_DIR}/driver/i2c/i2c1.c
	rm -rf ${BEKEN_SDK_DIR}/driver/i2c/i2c1.h
	rm -rf ${BEKEN_SDK_DIR}/driver/i2c/i2c2.c
	rm -rf ${BEKEN_SDK_DIR}/driver/i2c/i2c2.h
	rm -rf ${BEKEN_SDK_DIR}/driver/i2s
	rm -rf ${BEKEN_SDK_DIR}/driver/icu
	rm -rf ${BEKEN_SDK_DIR}/driver/intc
	rm -rf ${BEKEN_SDK_DIR}/driver/irda
	rm -rf ${BEKEN_SDK_DIR}/driver/jpeg
	rm -rf ${BEKEN_SDK_DIR}/driver/macphy_bypass
	rm -rf ${BEKEN_SDK_DIR}/driver/phy
	rm -rf ${BEKEN_SDK_DIR}/driver/phy_karst
	rm -rf ${BEKEN_SDK_DIR}/driver/pwm
	rm -rf ${BEKEN_SDK_DIR}/driver/qspi
	rm -rf ${BEKEN_SDK_DIR}/driver/rc_beken
	rm -rf ${BEKEN_SDK_DIR}/driver/rw_pub
	rm -rf ${BEKEN_SDK_DIR}/driver/saradc
	rm -rf ${BEKEN_SDK_DIR}/driver/sdcard
	rm -rf ${BEKEN_SDK_DIR}/driver/sdio
	rm -rf ${BEKEN_SDK_DIR}/driver/security
	rm -rf ${BEKEN_SDK_DIR}/driver/spi
	rm -rf ${BEKEN_SDK_DIR}/driver/spidma
	rm -rf ${BEKEN_SDK_DIR}/driver/sys_ctrl
	rm -rf ${BEKEN_SDK_DIR}/driver/uart
	rm -rf ${BEKEN_SDK_DIR}/driver/usb
	rm -rf ${BEKEN_SDK_DIR}/driver/wdt
	rm -rf ${BEKEN_SDK_DIR}/driver/dsp
	rm -rf ${BEKEN_SDK_DIR}/driver/mailbox
	rm -rf ${BEKEN_SDK_DIR}/driver/driver.c
	rm -rf ${BEKEN_SDK_DIR}/driver/SConscript
	
	rm -rf ${BEKEN_SDK_DIR}/func/airkiss
	rm -rf ${BEKEN_SDK_DIR}/func/audio
	rm -rf ${BEKEN_SDK_DIR}/func/base64
	rm -rf ${BEKEN_SDK_DIR}/func/bk7011_cal
	rm -rf ${BEKEN_SDK_DIR}/func/ble_wifi_exchange
	rm -rf ${BEKEN_SDK_DIR}/func/camera_intf
	rm -rf ${BEKEN_SDK_DIR}/func/easy_flash
	rm -rf ${BEKEN_SDK_DIR}/func/ethernet_intf
	rm -rf ${BEKEN_SDK_DIR}/func/fatfs
	rm -rf ${BEKEN_SDK_DIR}/func/hostapd_intf
	rm -rf ${BEKEN_SDK_DIR}/func/hostapd-2.5
	rm -rf ${BEKEN_SDK_DIR}/func/joint_up
	rm -rf ${BEKEN_SDK_DIR}/func/key
	rm -rf ${BEKEN_SDK_DIR}/func/key_handle
	rm -rf ${BEKEN_SDK_DIR}/func/lwip_intf
	rm -rf ${BEKEN_SDK_DIR}/func/mbedtls
	rm -rf ${BEKEN_SDK_DIR}/func/misc
	rm -rf ${BEKEN_SDK_DIR}/func/music_player
	rm -rf ${BEKEN_SDK_DIR}/func/mutually_exclusive_pwm
	rm -rf ${BEKEN_SDK_DIR}/func/net_param_intf
	rm -rf ${BEKEN_SDK_DIR}/func/paho-mqtt
	rm -rf ${BEKEN_SDK_DIR}/func/power_save
	rm -rf ${BEKEN_SDK_DIR}/func/rf_test
	rm -rf ${BEKEN_SDK_DIR}/func/rf_use
	rm -rf ${BEKEN_SDK_DIR}/func/rwnx_intf
	rm -rf ${BEKEN_SDK_DIR}/func/saradc_intf
	rm -rf ${BEKEN_SDK_DIR}/func/sd_music
	rm -rf ${BEKEN_SDK_DIR}/func/sdio_intf
	rm -rf ${BEKEN_SDK_DIR}/func/sdio_trans
	rm -rf ${BEKEN_SDK_DIR}/func/security
	rm -rf ${BEKEN_SDK_DIR}/func/sensor
	rm -rf ${BEKEN_SDK_DIR}/func/sim_uart
	rm -rf ${BEKEN_SDK_DIR}/func/spidma_intf
	rm -rf ${BEKEN_SDK_DIR}/func/temp_detect
	rm -rf ${BEKEN_SDK_DIR}/func/uart_debug/*.c
	rm -rf ${BEKEN_SDK_DIR}/func/uart_debug/uart_debug.mk
	rm -rf ${BEKEN_SDK_DIR}/func/udisk_mp3
	rm -rf ${BEKEN_SDK_DIR}/func/usb
	rm -rf ${BEKEN_SDK_DIR}/func/usb_plug
	rm -rf ${BEKEN_SDK_DIR}/func/user_driver
	rm -rf ${BEKEN_SDK_DIR}/func/utf8
	rm -rf ${BEKEN_SDK_DIR}/func/vad
	rm -rf ${BEKEN_SDK_DIR}/func/video_transfer
	rm -rf ${BEKEN_SDK_DIR}/func/voice_transfer
	rm -rf ${BEKEN_SDK_DIR}/func/wlan_ui
	rm -rf ${BEKEN_SDK_DIR}/func/wolfssl
	rm -rf ${BEKEN_SDK_DIR}/func/wpa_supplicant_2_9
	rm -rf ${BEKEN_SDK_DIR}/func/pcm_resampler
	rm -rf ${BEKEN_SDK_DIR}/func/motor
	rm -rf ${BEKEN_SDK_DIR}/func/func.c
	rm -rf ${BEKEN_SDK_DIR}/func/SConscript
	
	rm -rf ${BEKEN_SDK_DIR}/ip/common/*.c
	rm -rf ${BEKEN_SDK_DIR}/ip/ke
	rm -rf ${BEKEN_SDK_DIR}/ip/lmac
	rm -rf ${BEKEN_SDK_DIR}/ip/mac
	rm -rf ${BEKEN_SDK_DIR}/ip/umac
	rm -rf ${BEKEN_SDK_DIR}/ip/ip.mk
	rm -rf ${BEKEN_SDK_DIR}/ip/ip_src.mk
	rm -rf ${BEKEN_SDK_DIR}/ip/lib_files.sh
	rm -rf ${BEKEN_SDK_DIR}/ip/SConscript
	
	rm -rf ${BEKEN_SDK_DIR}/ip_ax

	rm -rf ${BEKEN_SDK_DIR}/lib
	rm -rf ${BEKEN_SDK_DIR}/os
	rm -rf ${BEKEN_SDK_DIR}/rttos
	rm -rf ${BEKEN_SDK_DIR}/beken.mk
	rm -rf ${BEKEN_SDK_DIR}/SConscript
}

echo "generating $target_mcu sdk ..."

if [ -f ${BEKEN_TEST_BLE_CFG_FILE} ]; then
	echo "backup ${BEKEN_TEST_BLE_CFG_FILE}"
	cp -f ${BEKEN_TEST_BLE_CFG_FILE} ${BEKEN_TEST_BLE_CFG_FILE}.bak
	if [ "$target_mcu" == "bk7231u" ]; then
		modify_config ${BEKEN_TEST_BLE_CFG_FILE} CONFIG_BEKEN_BLE_4_2 1
	else
		modify_config ${BEKEN_TEST_BLE_CFG_FILE} CONFIG_BEKEN_BLE_4_2 0
	fi
fi

./build.sh clean
if [ $? != 0 ]; then
	echo "generate ${target_mcu} sdk fail"
	exit 1
fi

./build.sh ${target_type} ${target_app} ${target_board} ${target_region} ${target_env} ${target_debug}
if [ $? != 0 ]; then
	echo "generate ${target_mcu} sdk fail"
	exit 1
fi

# clean lib source files
./tools/scripts/clean_src_files.sh ${BEKEN_SDK_DIR}
if [ $? != 0 ]; then
	echo "clean src files fail"
	exit 1
fi

files_rm
libs_cp

./build.sh clean
if [ $? != 0 ]; then
	echo "generate ${target_mcu} sdk fail"
	exit 1
fi

cp Living_SDK/platform/mcu/${target_mcu}/${target_mcu}_sdk.mk Living_SDK/platform/mcu/${target_mcu}/$target_mcu.mk

if [ -f ${BEKEN_TEST_BLE_CFG_FILE} ]; then
	echo "restore ${BEKEN_TEST_BLE_CFG_FILE}"
	rm -f ${BEKEN_TEST_BLE_CFG_FILE}
	mv -f ${BEKEN_TEST_BLE_CFG_FILE}.bak ${BEKEN_TEST_BLE_CFG_FILE}
fi

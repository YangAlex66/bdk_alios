#ifndef __BEKEN_TEST_BLE_H__
#define __BEKEN_TEST_BLE_H__


#define CONFIG_BEKEN_BLE_4_2 1

#define BEKEN_TEST_BLE_4_2   1
#define BEKEN_TEST_BLE_5_0   2
#if CONFIG_BEKEN_BLE_4_2
#define BEKEN_TEST_BLE_TYPE  BEKEN_TEST_BLE_4_2
#else
#define BEKEN_TEST_BLE_TYPE  BEKEN_TEST_BLE_5_0
#endif

#if BEKEN_TEST_BLE_TYPE == BEKEN_TEST_BLE_4_2
extern ble_err_t beken_test_bk_ble_init(void);
extern void beken_test_ble_write_callback(write_req_t *write_req);
extern uint8_t beken_test_ble_read_callback(read_req_t *read_req);
extern void beken_test_ble_event_callback(ble_event_t event, void *param);
#endif

extern void beken_test_ble_command_usage(void);
extern void beken_test_ble_advertise(void);
#endif

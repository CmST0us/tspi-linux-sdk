#ifndef __BT_TEST_H_
#define __BT_TEST_H_
//#include "libbluetooth/bluetooth/bluetooth.h"
//#include "libbluetooth/bluetooth/hci.h"
//#include "libbluetooth/bluetooth/hci_lib.h"
//#include "rk_pcba_test_led.h"
void* bt_test(void* argv);
struct bt_msg {
	int result;
	int x;
	int y;
	int w;
	int h;
	char *ssid;
};

#endif

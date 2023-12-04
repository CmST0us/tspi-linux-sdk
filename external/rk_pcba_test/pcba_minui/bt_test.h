#ifndef __BT_TEST_H_
#define __BT_TEST_H_
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

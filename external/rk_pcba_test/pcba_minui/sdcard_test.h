#ifndef __SDCARD_TEST_H
#define __SDCARD_TEST_H

struct sd_msg {
	int result;
	int y;
};

void *sdcard_test(void *argv);

#endif

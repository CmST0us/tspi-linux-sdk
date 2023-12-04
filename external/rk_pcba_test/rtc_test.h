#ifndef __RTC_TEST_H_
#define __RTC_TEST_H_
#include "rk_pcba_test_led.h"
void *rtc_test(void *argc);

struct rtc_msg {
	int result;
	char *date;
};

#endif


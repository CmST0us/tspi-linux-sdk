#ifndef __RTC_TEST_H_
#define __RTC_TEST_H_

void *rtc_test(void *argc);

struct rtc_msg {
	int result;
	char *date;
};

#endif

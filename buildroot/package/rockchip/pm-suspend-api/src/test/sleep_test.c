#include <stdio.h>
#include "dbus_suspend_api.h"

int main()
{
	printf("hello sleep_test\n");
	while (1) {
		sleep(10);
		printf("time elapse 10 seconds,request sleep\n");
		request_system_suspend();
	}
	return 0;
}

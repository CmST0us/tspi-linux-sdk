#include <stdio.h>
#include "dbus_suspend_api.h"

int suspend(void *data)
{
	printf("app_test1 suspending\n");
	usleep(100*1000);
	//to do...
	printf("app_test1 suspend succeed\n");
	return 0;
}

int resume(void *data)
{
    printf("app_test1 resumeing \n");
    usleep(100*1000);
	//to do...
    printf("app_test1 resume succeed\n");
    return 0;
}

int main()
{
	printf("hello world\n");
	register_suspend_listener("com.rockchip.suspend.app_test1", suspend, resume);
	while(1)
		sleep(2);
	return 0;
}

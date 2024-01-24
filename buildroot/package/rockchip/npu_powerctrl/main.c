// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
// Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd

#define LOG_TAG "NPU_POWER"
#include <npu_powerctrl.h>
#include <iostream>
#include <unistd.h>

static void usage(void)
{
	printf("Usage:npu_powerctrl [-s] [-r] [-o] [-i] [-d]\n");
	printf("-s	npu enter sleep\n");
	printf("-r	wakup npu\n");
	printf("-o	power up or reset npu\n");
	printf("-i	gpio init\n");
	printf("-d	power down\n");
}

int main(int argc, char* argv[])
{
	int ch, ret = -1;

	while ((ch = getopt(argc, argv, "s::r::o::i::d::")) != -1) {
		switch (ch) {
			case 's':
				ret = npu_suspend();
				printf("suspend %d\n", ret);
				break;
			case 'r':
				ret = npu_resume();
				printf("resume %d\n", ret);
				break;
			case 'o':
				ret = 0;
				npu_reset();
				printf("powerup\n");
				break;
			case 'i':
				npu_power_gpio_init();
				ret = 0;
				printf("gpio init\n");
				break;
			case 'd':
				npu_poweroff();
				ret = 0;
				printf("powerdown\n");
				break;
			default:
				usage();
				break;
		}
		return ret;
	}
	usage();

	return ret;
}

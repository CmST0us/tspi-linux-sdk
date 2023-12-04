// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
// Copyright (c) 2018 Fuzhou Rockchip Electronics Co., Ltd

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <npu_powerctrl.h>

#define VERSION "V1.1 for power combine board"
#define FNAME_SIZE 50
#define GPIO_BASE_PATH "/sys/class/gpio"
#define GPIO_EXPORT_PATH GPIO_BASE_PATH "/export"
#define GPIO_UNEXPORT_PATH GPIO_BASE_PATH "/unexport"
#define CLKEN_24M_PATH "/sys/kernel/debug/clk/clk_wifi_pmu/clk_enable_count"
#define CLKEN_32k_PATH "/sys/kernel/debug/clk/rk808-clkout2/clk_enable_count"
#define PCIE_RESET_EP "/sys/devices/platform/f8000000.pcie/pcie_reset_ep"
#define ACM_HIGHSPEED_ID "/sys/bus/platform/devices/fe380000.usb/usb*/*/idProduct"
#define ACM_FULLSPEED_ID "/sys/bus/platform/devices/fe3a0000.usb/usb*/*/idProduct"

#define CPU_RESET_NPU_GPIO 	"32" //GPIO1_PA0
#define NPU_PMU_SLEEP_GPIO 	"35" //GPIO1_A3
#define CPU_INT_NPU_GPIO 	"36" //GPIO1_A4

#define NPU_VDD_EN1_GPIO 	"4"  //GPIO0_PA4
#define NPU_VDD_EN4_GPIO 	"54" //GPIO1_PC6  vdd_cpp/vdd

static char gpio_list[][4] = {NPU_VDD_EN1_GPIO, NPU_VDD_EN4_GPIO, CPU_RESET_NPU_GPIO, \
	NPU_PMU_SLEEP_GPIO, CPU_INT_NPU_GPIO};

static int sysfs_write(char *path, char *val) {
	char buf[80];
	int len;
	int fd = open(path, O_WRONLY);

	if (fd < 0) {
		strerror_r(errno, buf, sizeof(buf));
		printf("Error opening %s value=%s:%s\n", path, val, buf);
		return -1;
	}

	len = write(fd, val, sizeof(val));
	if (len < 0) {
		strerror_r(errno, buf, sizeof(buf));
		printf("Error writing to %s value=%s: %s\n", path, val, buf);
		return -1;
	}

	close(fd);
	return 0;
}

static void sysfs_read(char *path, char *val)
{
	char buf[80];
	int len;
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		strerror_r(errno, buf, sizeof(buf));
		printf("Error opening %s value=%s:%s\n", path, val, buf);
		return;
	}

	len = read(fd, val, 1);
	if (len < 0) {
		strerror_r(errno, buf, sizeof(buf));
		printf("Error reading to %s value=%s: %s\n", path, val, buf);
	}

	close(fd);
}

static int clk_enable(char *enable) {
	char val;

	sysfs_read(CLKEN_24M_PATH, &val);
	if (*enable == val)
		return 0;

	printf("set clk_en  %c to %s\n", val, enable);
	sysfs_write(CLKEN_24M_PATH, enable);
	return 0;
}

static int request_gpio(char *gpio_num) {
	int ret;
	ret = sysfs_write(GPIO_EXPORT_PATH, gpio_num);
	return ret;
}

static void free_gpio(char *gpio_num) {
	sysfs_write(GPIO_UNEXPORT_PATH, gpio_num);
}

static void set_gpio_dir(char *gpio_num, char *dir) {
	char gpio_dir_name[FNAME_SIZE];

	snprintf(gpio_dir_name, sizeof(gpio_dir_name), "%s/gpio%s/direction",
			GPIO_BASE_PATH, gpio_num);
	sysfs_write(gpio_dir_name, dir);
}

static int get_gpio(char *gpio_number) {
	char gpio_name[FNAME_SIZE];
	char value;

	snprintf(gpio_name, sizeof(gpio_name), "%s/gpio%s/value",
			GPIO_BASE_PATH, gpio_number);

	sysfs_read(gpio_name, &value);
	if (value == 48 || value == 49)
		value -= 48;
	else
		value = -1;
	return (int)value;
}

static int set_gpio(char *gpio_number, char *val) {
	char gpio_val_name[FNAME_SIZE];

	snprintf(gpio_val_name, sizeof(gpio_val_name), "%s/gpio%s/value",
			GPIO_BASE_PATH, gpio_number);
	sysfs_write(gpio_val_name, val);
	return 0;
}

void npu_power_gpio_init(void) {
	int ret, index = 0, gpio_cnt = sizeof(gpio_list)/sizeof(int);

	printf("version: %s\n", VERSION);
	while (index != gpio_cnt) {
		printf("init gpio: %s\n", gpio_list[index]);
		ret = request_gpio(gpio_list[index]);
		if (ret) {
			return;
		}
		set_gpio_dir(gpio_list[index], "out");
		index ++;
	}
	set_gpio_dir(NPU_PMU_SLEEP_GPIO, "in");
}

void npu_power_gpio_exit(void) {
	int index = 0, gpio_cnt = sizeof(gpio_list)/sizeof(int);

	while (index != gpio_cnt) {
		printf("init gpio: %s\n", gpio_list[index]);
		free_gpio(gpio_list[index]);
		index ++;
	}
}

void npu_reset(void) {
	//sysfs_write("/sys/power/wake_lock", "npu_lock");
	sysfs_write(CLKEN_32k_PATH, "1");
	clk_enable("0");
	set_gpio(NPU_VDD_EN1_GPIO, "0");
	set_gpio(NPU_VDD_EN4_GPIO, "0");
	set_gpio(CPU_RESET_NPU_GPIO, "0");
	usleep(2000);

	/*power en*/
	set_gpio(NPU_VDD_EN1_GPIO, "1");
	usleep(2000);
	clk_enable("1");
	set_gpio(NPU_VDD_EN4_GPIO, "1");

	usleep(25000);
	set_gpio(CPU_RESET_NPU_GPIO, "1");
}

void npu_poweroff(void) {
	set_gpio(NPU_VDD_EN1_GPIO, "0");
	set_gpio(NPU_VDD_EN4_GPIO, "0");
	clk_enable("0");
	//sysfs_write("/sys/power/wake_unlock", "npu_lock");
}

int disconnect_usb_acm(void)
{
	FILE *stream;
	char buf[100];
	int idx = 0, path_idx=0;

	memset( buf,'\0',sizeof(buf) );
	stream = popen( "cat " ACM_HIGHSPEED_ID, "r" );
	if (!stream) {
		path_idx=1;
		stream = popen( "cat " ACM_FULLSPEED_ID, "r" );
		if (!stream)
			return -1;
	}
	fread( buf, sizeof(char), sizeof(buf), stream);
	pclose( stream );
	printf("ACM idProduct: %s", buf);
	if (!strncmp(buf, "1005", 4)) {
		if (path_idx)
			stream = popen("find /sys/bus/platform/devices/fe3a0000.usb/usb*/*/ -name remove", "r" );
		else
			stream = popen("find /sys/bus/platform/devices/fe380000.usb/usb*/*/ -name remove", "r" );
		fread( buf, sizeof(char), sizeof(buf), stream);
		printf("usb remove patch is %s", buf);
		/* The path string adds end character */
		while(idx < 100) {
			if (buf[idx] == '\n') {
				buf[idx] = '\0';
				break;
			}
			idx ++;
		}
		sysfs_write(buf, "0");
		pclose( stream );
		return 0;
	}
	return -1;

}

int npu_suspend(void) {
	int retry=100;
	int is_pcie;

	if (get_gpio(NPU_PMU_SLEEP_GPIO)) {
		printf("It is sleeping state, noting to do!\n");
		return 0;
	}

	is_pcie = access(PCIE_RESET_EP, R_OK);
	if (!is_pcie) {
		sysfs_write(PCIE_RESET_EP, "2");
		disconnect_usb_acm();
	}

	set_gpio(CPU_INT_NPU_GPIO, "1");
	usleep(20000);
	set_gpio(CPU_INT_NPU_GPIO, "0");

	/*wait for npu enter sleep*/
	while (--retry) {
		if (get_gpio(NPU_PMU_SLEEP_GPIO)) {
			usleep(10000);
			set_gpio(NPU_VDD_EN4_GPIO, "0");
			clk_enable("0");
			/* wait 1s for usb disconnect */
			sleep(1);
			//sysfs_write("/sys/power/wake_unlock", "npu_lock");
			break;
		}
		usleep(10000);
	}

	if (!retry) {
		printf("npu suspend timeout in one second\n");
		return -1;
	}
	return 0;
}

int npu_resume(void) {
	int retry=100;
	int is_pcie;

	if (!get_gpio(NPU_PMU_SLEEP_GPIO)) {
		printf("It is awakening state, noting to do!\n");
		return 0;
	}

	clk_enable("1");
	set_gpio(NPU_VDD_EN4_GPIO, "1");

	usleep(10000);

	set_gpio(CPU_INT_NPU_GPIO, "1");
	usleep(20000);
	set_gpio(CPU_INT_NPU_GPIO, "0");

	/*wait for npu wakeup*/
	while (--retry) {
		if (!get_gpio(NPU_PMU_SLEEP_GPIO)) {
		//	sysfs_write("/sys/power/wake_lock", "npu_lock");
			break;
		}
		usleep(10000);
	}

	is_pcie = access(PCIE_RESET_EP, R_OK);
	if (!is_pcie)
		sysfs_write(PCIE_RESET_EP, "1");

	if (!retry) {
		printf("npu resume timeout in one second\n");
		return -1;
	}
	//waiting for userspase wakup
	/*usleep(500000);*/
	/*set_gpio(CPU_INT_NPU_GPIO, "0");*/

	return 0;
}

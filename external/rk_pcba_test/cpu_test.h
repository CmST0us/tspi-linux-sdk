#ifndef CPU_TEST_H
#define CPU_TEST_H
#include "rk_pcba_test_led.h"

#define _CPU_0_FREQ_TABLE "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies"
#define _CPU_1_FREQ_TABLE "/sys/devices/system/cpu/cpu1/cpufreq/scaling_available_frequencies"
#define _CPU_2_FREQ_TABLE "/sys/devices/system/cpu/cpu2/cpufreq/scaling_available_frequencies"
#define _CPU_3_FREQ_TABLE "/sys/devices/system/cpu/cpu3/cpufreq/scaling_available_frequencies"

#define _CPU_0_FREQ_GOVERNOR "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define _CPU_1_FREQ_GOVERNOR "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor"
#define _CPU_2_FREQ_GOVERNOR "/sys/devices/system/cpu/cpu2/cpufreq/scaling_governor"
#define _CPU_3_FREQ_GOVERNOR "/sys/devices/system/cpu/cpu3/cpufreq/scaling_governor"

#define _CPU_0_FREQ_SET "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed"
#define _CPU_1_FREQ_SET "/sys/devices/system/cpu/cpu1/cpufreq/scaling_setspeed"
#define _CPU_2_FREQ_SET "/sys/devices/system/cpu/cpu2/cpufreq/scaling_setspeed"
#define _CPU_3_FREQ_SET "/sys/devices/system/cpu/cpu3/cpufreq/scaling_setspeed"

#define _CPU_0_FREQ_GET "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
#define _CPU_1_FREQ_GET "/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq"
#define _CPU_2_FREQ_GET "/sys/devices/system/cpu/cpu2/cpufreq/scaling_cur_freq"
#define _CPU_3_FREQ_GET "/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq"

#define _CPU_MODE_USER "userspace"
#define _CPU_MODE_ONDEMAND "ondemand"
#define _CPU_MODE_INTERACTIVE "interactive"

//#define _CPU_FREQ_TXT "/data/cpu%d_freq_table.txt"

#define ACCELERATION_RATIO_ANDROID_TO_HW        (9.80665f / 1000000)
#define __MAX  32

#define BOOT_MODE_NORMAL		0
#define BOOT_MODE_FACTORY2		1
#define BOOT_MODE_RECOVERY		2
#define BOOT_MODE_CHARGE		3
#define BOOT_MODE_POWER_TEST		4
#define BOOT_MODE_OFFMODE_CHARGING	5
#define BOOT_MODE_REBOOT		6
#define BOOT_MODE_PANIC			7
#define BOOT_MODE_WATCHDOG		8

typedef struct _CPU_FREQ_ {
	int freq;
	struct _CPU_FREQ_ *next;
} CPU_FREQ;

typedef struct _CPU_ {
	int num0;
	struct CPU_FREQ *cpu_0;
	int num1;
	struct CPU_FREQ *cpu_1;
	int num2;
	struct CPU_FREQ *cpu_2;
	int num3;
	struct CPU_FREQ *cpu_3;
} CPU_INFO;

void *cpu_test(void *argv);

#endif

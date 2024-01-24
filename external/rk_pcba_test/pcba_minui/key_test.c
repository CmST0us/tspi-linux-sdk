#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <linux/input.h>
#include <fcntl.h>
#include <dirent.h>
#include "common.h"
#include "key_test.h"
#include "test_case.h"
#include "language.h"
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

#include "screen_test.h"

unsigned char keybitmask[(KEY_MAX + 1) / 8];
struct key key_code[KEY_MAX];
unsigned char key_cnt = 0;		/*key counter */
unsigned int gkey = 0;
static pthread_mutex_t gkeymutex = PTHREAD_MUTEX_INITIALIZER;

int g_key_test = 0;
static struct testcase_info *tc_info;

int set_gkey(unsigned int code)
{
	int i;
	for (i = 0; i < key_cnt; i++) {
		if (code == key_code[i].code) {
			ui_print_xy_rgba(0, tc_info->y, 0, 255, 0, 255,
				"%s:[%s] key down\n", PCBA_KEY,
				key_code[i].name);
			screenTest_key_detect(code);
			break;
		}
	}
	return 0;
}

int scan_key_code(void)
{
	unsigned temp = 0;

	pthread_mutex_lock(&gkeymutex);
	if (gkey) {
		temp = gkey;
		gkey = 0;
	}
	pthread_mutex_unlock(&gkeymutex);

	return temp;
}
void switch_key(int i)
{
	key_code[key_cnt].code = i;
	switch (i) {
	case KEY_VOLUMEDOWN:
		key_code[key_cnt].name = "vol-";
		break;
	case KEY_VOLUMEUP:
		key_code[key_cnt].name = "vol+";
		break;
	case KEY_POWER:
		key_code[key_cnt].name = "power";
		break;
	case KEY_F1:
		key_code[key_cnt].name = "menu";
#ifdef RK312X_PCBA
		key_code[key_cnt].name = "vol+";
#endif
		break;
	case KEY_HOME:
		key_code[key_cnt].name = "home";
		break;
	case KEY_BACK:
		key_code[key_cnt].name = "ESC";
		break;
	case KEY_UP:
		key_code[key_cnt].name = "UP";
		break;
	case KEY_DOWN:
		key_code[key_cnt].name = "DOWN";
		break;
	case KEY_LEFT:
		key_code[key_cnt].name = "LEFT";
		break;
	case KEY_RIGHT:
		key_code[key_cnt].name = "RIGHT";
		break;
	case KEY_MENU:
		key_code[key_cnt].name = "MENU";
		break;
	case KEY_ENTER:
	case KEY_REPLY:
		key_code[key_cnt].name = "OK";
		break;
	default:
		printf("un supported key:%d\n", i);
		break;
	}
	key_cnt++;
}
int key_code_probe(void)
{
	DIR *dir;
	struct dirent *de;
	int fd;
	char name[80];
	int i;

	dir = opendir("/dev/input");
	if (dir != 0) {
		while ((de = readdir(dir))) {
			if (strncmp(de->d_name, "event", 5))
				continue;
			fd = openat(dirfd(dir), de->d_name, O_RDONLY);
			if (fd < 0) {
				continue;
			} else {
				if (ioctl
				    (fd, EVIOCGNAME(sizeof(name) - 1),
				     &name) < 1) {
					name[0] = '\0';
				}
				/*do not open gsensor here */
				if (!strcmp(name, "gsensor"))
					continue;
				ioctl(fd,
				      EVIOCGBIT(EV_KEY, sizeof(keybitmask)),
				      keybitmask);
				for (i = 0; i < KEY_MAX; i++) {
					if (test_bit(i, keybitmask) &&
					    (i != KEY_WAKEUP))
						switch_key(i);
				}
			}
		}
	}

	return 0;
}
void *key_test(void *argc)
{
	int i = 0;
	int code;
	int run = 1;

	tc_info = (struct testcase_info *)argc;

	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();

	ui_print_xy_rgba(0, tc_info->y, 255, 255, 0, 255, "%s: Waiting press key...\n", PCBA_KEY);
	key_code_probe();
	g_key_test = 1;

	return NULL;
}

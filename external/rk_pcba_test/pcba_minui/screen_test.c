#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "screen_test.h"
#include "test_case.h"
#include "common.h"
#include "language.h"
#include "minui/minui.h"

#include <linux/input.h>
#include "script.h"

/*  variable for test_config.cfg  */
int key_times;
int all_key_change;
unsigned int start_key;

unsigned int code_tmp = 0;
unsigned int picture_change_key;

unsigned char code_cnt = 0;
unsigned char screen_start = 0;

int w1, h1, lcd_y;

void screen_change(uint8_t screen_cnt)
{
//    printf("////////// screen_chage : %d ///////////\n",screen_cnt);
	switch (screen_cnt) {
	case  0:
		FillColor(255, 0, 0, 255, 0, 0, w1, h1);    /*red*/
		break;
	case  1:
		tiles_count--;
		FillColor(0, 255, 0, 255, 0, 0, w1, h1);    /*green*/
		break;
	case  2:
		tiles_count--;
		FillColor(0, 0, 255, 255, 0, 0, w1, h1);    /*blue*/
		break;
	case  3:
		tiles_count--;
		FillColor(0, 0, 0, 255, 0, 0, w1, h1);      /*black*/
		break;
	case  4:
		tiles_count--;
		FillColor(255, 255, 255, 255, 0, 0, w1, h1);/*white*/
		break;
	case  5:
		tiles_count--;
		ui_print_xy_rgba(0, lcd_y, 0, 255, 0, 255,
			            "%s:[%s]\n", PCBA_LCD, PCBA_TESTED);
		break;
	default:
		screen_start = 0;
		ui_print_xy_rgba(0, lcd_y, 0, 255, 0, 255,
			            "%s:[%s]\n", PCBA_LCD, PCBA_TESTED);
		break;
	}
	/*printf("-----------%s--%s---screen_cnt=%d----------\n",
		__FILE__,__FUNCTION__,screen_cnt);*/
}

unsigned int get_key_code(char *key_char)
{
	if (!strcmp(key_char, "KEY_VOLUMEDOWN"))
		return KEY_VOLUMEDOWN;
	else if (!strcmp(key_char, "KEY_VOLUMEUP"))
		return KEY_VOLUMEUP;
	else if (!strcmp(key_char, "KEY_POWER"))
		return KEY_POWER;
	else if (!strcmp(key_char, "KEY_F1"))
		return KEY_F1;
	else if (!strcmp(key_char, "KEY_HOME"))
		return KEY_HOME;
	else if (!strcmp(key_char, "KEY_BACK"))
		return KEY_BACK;
	else if (!strcmp(key_char, "KEY_UP"))
		return KEY_UP;
	else if (!strcmp(key_char, "KEY_DOWN"))
		return KEY_DOWN;
	else if (!strcmp(key_char, "KEY_LEFT"))
		return KEY_LEFT;
	else if (!strcmp(key_char, "KEY_RIGHT"))
		return KEY_RIGHT;
	else if (!strcmp(key_char, "KEY_MENU"))
		return KEY_MENU;
	else if (!strcmp(key_char, "KEY_ENTER"))
		return KEY_ENTER;
	else if (!strcmp(key_char, "KEY_ALL"))
		return 0;
	else {
		printf("un supported key:%s\n", key_char);
		return 0;
	}
}

/*  key detect
*   start the test and change the picture
*/
void screenTest_key_detect(unsigned int code)
{
	if (!screen_start) {
		if (code != code_tmp) {
			code_tmp = code;
			code_cnt = 0;
		} else {
			if ((!start_key) || (code == start_key)) {
				if (++code_cnt >= (key_times-1)) {
					picture_change_key = code;
					screen_start = 1;
					code_cnt = 0;
					screen_change(code_cnt);
				}
			}
		}
	} else {
		if ((code == picture_change_key) || all_key_change) {
			code_cnt++;
			screen_change(code_cnt);
			if (code_cnt >= 5) {
				code_cnt = 0;
				screen_start = 0;
				code_tmp = !code;
			}
		}
	}
}

void *screen_test(void *argv)
{
	struct testcase_info *tc_info = (struct testcase_info *)argv;
	char key_name[16];

	if (script_fetch("Lcd", "start_key", (int *)key_name, 4) != 0) {
		printf("%s: key_times err : %s !!!\r\n",__func__, key_name);
		strcpy(key_name, "KEY_ALL");
	}
	printf("=========== code_cnt is:   %d ==========\n", code_cnt);
	start_key = get_key_code(key_name);

	if (script_fetch("Lcd", "key_times", &key_times, 1) != 0) {
		printf("%s: key_times err : %d !!!\r\n",__func__, key_times);
		key_times = 3;
	}
	if (script_fetch("Lcd", "all_key_change", &all_key_change, 1) != 0) {
		printf("%s: all_key_change err : %d !!!\r\n",__func__, all_key_change);
		all_key_change = 0;
	}

	w1 =  gr_fb_width();
	h1 =  gr_fb_height();

	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();

	ui_print_xy_rgba(0, tc_info->y, 255, 255, 0, 255, \
	                "%s: pls press [%s] %d times to start test.\n", \
	                PCBA_LCD, key_name, key_times);
	lcd_y = tc_info->y;

	int x =  gr_fb_width() >> 1;
	int y = (gr_fb_height()*2)/3;
	int w =  gr_fb_width() >> 1;
	int h = gr_fb_height()/3;

	FillColor(255, 0, 0, 255, x, y, w/3, h);
	FillColor(0, 255, 0, 255, x+w/3, y, w/3, h);
	FillColor(0, 0, 255, 255, x+(2*w)/3, y, w/3, h);
	sleep(3);

	tc_info->result = 0;
	return argv;
}

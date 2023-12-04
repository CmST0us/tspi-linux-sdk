#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <inttypes.h>
#include <errno.h>


#include "minui/minui.h"
#include "common.h"
#include "minzip/DirUtil.h"
#include "recovery_ui.h"

#include "script.h"
#include "test_case.h"
#include "script_parser.h"
#include "debug.h"

#include "Language/language.h"


#include "rtc_test.h"
#include "wlan_test.h"
#include "ddr_test.h"
#include "bt_test.h"
#include "sdcard_test.h"
#include "udisk_test.h"
#include "key_test.h"
#include "screen_test.h"
#include "emmc_test.h"
#include "audio_play_test.h"
#include "audio_record_test.h"

#define SCRIPT_NAME                     "/data/test_config.cfg"

int manual_p_y = 1;
/* current position for auto test tiem in y direction */
int cur_p_y;

pthread_t rtc_tid;
char *rtc_res;
char dt[30] = { "20120927.143045" };
struct rtc_msg *rtc_msg;
int err_rtc;
int rtc_p_y;

pthread_t screen_tid;
pthread_t wlan_tid;
pthread_t bt_tid;
pthread_t sd_tid;
pthread_t udisk_tid;
pthread_t ddr_tid;
pthread_t emmc_tid;
pthread_t led_tid;
pthread_t key_tid;

pthread_t audioplay_tid;
pthread_t audiorecord_tid;

static int total_testcases;
static struct testcase_base_info *base_info;
static struct list_head auto_test_list_head;
static struct list_head manual_test_list_head;


static pthread_mutex_t gCur_p_y = PTHREAD_MUTEX_INITIALIZER;

int get_cur_print_y(void)
{
	int tmp;

	pthread_mutex_lock(&gCur_p_y);
	/*if (gr_fb_height() > 1080)
		cur_p_y--;
	tmp = cur_p_y--;*/
	tmp = cur_p_y++;

	pthread_mutex_unlock(&gCur_p_y);
	printf("cur_print_y:%d\n", tmp);
	return tmp;
}

static int parse_testcase(void)
{
	int i, j, mainkey_cnt;
	struct testcase_base_info *info;
	char mainkey_name[32], display_name[64], binary[16];
	int activated, category, run_type, sim_counts;
	int len;

	mainkey_cnt = script_mainkey_cnt();
	info = (struct testcase_base_info *)
	    malloc(sizeof(struct testcase_base_info) * mainkey_cnt);
	if (info == NULL) {
		db_error("core: allocate memory for temporary test case basic "
			 "information failed(%s)\n", strerror(errno));
		return -1;
	}
	memset(info, 0, sizeof(struct testcase_base_info) * mainkey_cnt);

	for (i = 0, j = 0; i < mainkey_cnt; i++) {
		struct testcase_info *tc_info;

		memset(mainkey_name, 0, 32);
		script_mainkey_name(i, mainkey_name);

		if (script_fetch
		    (mainkey_name, "display_name", (int *)display_name, 16))
			continue;

		if (script_fetch(mainkey_name, "activated", &activated, 1))
			continue;

		if (display_name[0] && activated == 1) {
			strncpy(info[j].name, mainkey_name, 32);
			strncpy(info[j].display_name, display_name, 64);
			info[j].activated = activated;

			if (script_fetch
			    (mainkey_name, "program", (int *)binary, 4) == 0) {
				strncpy(info[j].binary, binary, 16);
			}

			info[j].id = j;

			if (script_fetch(mainkey_name, "category", &category, 1)
			    == 0) {
				info[j].category = category;
			}

			if (script_fetch(mainkey_name, "run_type", &run_type, 1)
			    == 0) {
				info[j].run_type = run_type;
			}

            /*
			if (script_fetch
			    (mainkey_name, "sim_counts", &sim_counts, 1) == 0) {
				simCounts = sim_counts;
			}
			*/

			tc_info = (struct testcase_info *)
			    malloc(sizeof(struct testcase_info));
			if (tc_info == NULL) {
				printf("malloc for tc_info[%d] fail\n", j);
				return -1;
			}
			tc_info->x = 0;
			tc_info->y = 0;
			tc_info->base_info = &info[j];
			if (tc_info->base_info->category)   //manual test
				list_add(&tc_info->list, &manual_test_list_head);
			else    //auto test
				list_add(&tc_info->list, &auto_test_list_head);
			j++;
		}
	}
	total_testcases = j;

	db_msg("core: total test cases #%d\n", total_testcases);
	if (total_testcases == 0)
		return 0;

	len = sizeof(struct testcase_base_info) * total_testcases;

	return total_testcases;
}

int start_test_pthread(struct testcase_info *tc_info)
{
	int err;
	printf(">>> Currnet test is : [ %s ]\n", tc_info->base_info->name);

#if 1
    if (!strcmp(tc_info->base_info->name, "rtc")) {
		err = pthread_create(&rtc_tid, NULL, rtc_test, tc_info);
		if (err != 0) {
			printf("create screen test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} if (!strcmp(tc_info->base_info->name, "Lcd")) {
		err = pthread_create(&screen_tid, NULL, screen_test, tc_info);
		if (err != 0) {
			printf("create screen test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "wifi")) {
        err = pthread_create(&wlan_tid, NULL, wlan_test, tc_info);
        if (err != 0) {
            printf("create screen test thread error: %s/n",
                   strerror(err));
            return -1;
        }
	} else if (!strcmp(tc_info->base_info->name, "ddr")) {
        err = pthread_create(&ddr_tid, NULL, ddr_test, tc_info);
        if (err != 0) {
            printf("create screen test thread error: %s/n",
                   strerror(err));
            return -1;
        }
	}else if (!strcmp(tc_info->base_info->name, "bluetooth")) {
        err = pthread_create(&bt_tid, NULL, bt_test, tc_info);
        if (err != 0) {
            printf("create screen test thread error: %s/n",
                   strerror(err));
            return -1;
        }
	} else if (!strcmp(tc_info->base_info->name, "sdcard")) {
		err = pthread_create(&sd_tid, NULL, sdcard_test, tc_info);
		if (err != 0) {
			printf("create sdcard test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "emmc")) {
		err = pthread_create(&emmc_tid, NULL, emmc_test, tc_info);
		if (err != 0) {
			printf("create sdcard test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "udisk")) {
		err = pthread_create(&udisk_tid, NULL, udisk_test, tc_info);
		if (err != 0) {
			printf("create usb host test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "play")) {
		err = pthread_create(&audioplay_tid, NULL, audio_play_test, tc_info);
		if (err != 0) {
			printf("create audio play test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "record")) {
		err = pthread_create(&audiorecord_tid, NULL, audio_record_test, tc_info);
		if (err != 0) {
			printf("create audio record test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "Key")) {
		err = pthread_create(&key_tid, NULL, key_test, tc_info);
		if (err != 0) {
			printf("create key test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else {
		printf(">>> unsupport test item: [ %s ] for now!\n", tc_info->base_info->name);
		return -1;
	}
#endif


#if 0

	if (!strcmp(tc_info->base_info->name, "Lcd")) {
		err = pthread_create(&screen_tid, NULL, screen_test, tc_info);
		if (err != 0) {
			printf("create screen test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "rtc")) {
		err = pthread_create(&rtc_tid, NULL, rtc_test, tc_info);
		if (err != 0) {
			printf("create rtc test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "battery")) {
		err = pthread_create(&battery_tid, NULL, battery_test, tc_info);
		if (err != 0) {
			printf("create battery_test test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "ddr_emmc")) {
		err = pthread_create(&ddr_emmc_tid, NULL,
			ddr_emmc_test, tc_info);
		if (err != 0) {
			printf("create ddr_emmc test  thread error: %s/n",
				   strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "Codec")) {
		hasCodec = 1;
		err = pthread_create(&codec_tid, NULL, codec_test, tc_info);
		if (err != 0) {
			printf("create codec test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "Key")) {
		err = pthread_create(&key_tid, NULL, key_test, tc_info);
		if (err != 0) {
			printf("create key test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "fm")) {
		err = pthread_create(&fm_tid, NULL, fm_test, tc_info);
		if (err != 0) {
			printf("create fm test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "camera")) {
		tc_info->dev_id = 0;
		err = pthread_create(&camera_tid, NULL, camera_test, tc_info);
		if (err != 0) {
		       printf("create camera test thread error: %s/n",
		              strerror(err));
		       return -1;
		}
    	}
	else if (!strcmp(tc_info->base_info->name, "wifi")) {
		err = pthread_create(&wlan_tid, NULL, wlan_test, tc_info);
		if (err != 0) {
			printf("create wifi test thread error: %s/n",
			       strerror(err));
		}
	} else if (!strcmp(tc_info->base_info->name, "nand")) {
		err = pthread_create(&nand_tid, NULL, nand_test, tc_info);
		if (err != 0) {
			printf("create nandflash test thread error: %s/n",
			       strerror(err));
		}
	} else if (!strcmp(tc_info->base_info->name, "bluetooth")) {
		printf("bluetooth_test thread created\n");

		err = pthread_create(&bt_tid, NULL, bt_test, tc_info);
		if (err != 0) {
			printf("create bt(bluetooth) test thread error: %s/n",
			       strerror(err));
		}
	} else if (!strcmp(tc_info->base_info->name, "gsensor")) {
		err = pthread_create(&gsensor_tid, NULL, gsensor_test, tc_info);
		if (err != 0) {
			printf("create gsensor test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "allsensor")) {
		err = pthread_create(&sensor_tid, NULL,
			all_sensor_test, tc_info);
		if (err != 0) {
			printf("create sensor test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "lsensor")) {
		err =
		    pthread_create(&lsensor_tid, NULL, lightsensor_test,
				   tc_info);
		if (err != 0) {
			printf("create lsensor test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "gps")) {
		err = pthread_create(&gps_tid, NULL, gps_test, tc_info);
		if (err != 0) {
			printf("create gps test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "psensor")) {
		err = pthread_create(&psensor_tid, NULL, psensor_test, tc_info);
		if (err != 0) {
			printf("create psensor test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "compass")) {
		err = pthread_create(&compass_tid, NULL, compass_test, tc_info);
		if (err != 0) {
			printf("create ST compass test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "udisk")) {
		err = pthread_create(&udisk_tid, NULL, udisk_test, tc_info);
		if (err != 0) {
			printf("create sdcard test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "sdcard")) {
		sd_err = pthread_create(&sd_tid, NULL, sdcard_test, tc_info);
		if (sd_err != 0) {
			printf("create sdcard test thread error: %s/n",
			       strerror(sd_err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "hdmi")) {
		hdmi_err = pthread_create(&hdmi_tid, NULL, hdmi_test, tc_info);
		if (hdmi_err != 0) {
			printf("create hdmi test thread error: %s/n",
			       strerror(hdmi_err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "sim")) {
		err = pthread_create(&sim_tid, NULL, sim_test, tc_info);
		if (err != 0) {
			printf("create sim test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "vibrator")) {
		err =
		    pthread_create(&vibrator_tid, NULL, vibrator_test, tc_info);
		if (err != 0) {
			printf("create vibrator test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "falshlight")) {
		err =
		    pthread_create(&falshlight_tid, NULL, flashlight_test,
				   tc_info);
		if (err != 0) {
			printf("create flashlight test thread error: %s/n",
			       strerror(err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "ddr")) {
		ddr_err = pthread_create(&ddr_tid, NULL, ddr_test, tc_info);
		if (ddr_err != 0) {
			printf("create ddr test thread error: %s/n",
			       strerror(ddr_err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "cpu")) {
		cpu_err = pthread_create(&cpu_tid, NULL, cpu_test, tc_info);
		if (cpu_err != 0) {
			printf("create cpu test thread error: %s/n",
			       strerror(cpu_err));
			return -1;
		}
	} else if (!strcmp(tc_info->base_info->name, "lan")) {
		lan_err = pthread_create(&lan_tid, NULL, lan_test, tc_info);
		if (lan_err != 0) {
			printf("create lan test thread error: %s/n",
			       strerror(lan_err));
			return -1;
		}
	} else {
		printf("unsupport test item:%s\n", tc_info->base_info->name);
		return -1;
	}
#endif
	return 0;
}


int init_manual_test_item(struct testcase_info *tc_info)
{
	int err = 0;
	printf("start_manual_test_item : %d, %s \r\n", tc_info->y,
	       tc_info->base_info->name);

	manual_p_y += 1;
	tc_info->y = manual_p_y;

	start_test_pthread(tc_info);

	return 0;
}

int start_manual_test_item(int x, int y)
{
	return 1;
//	Camera_Click_Event(x, y);
}

int start_auto_test_item(struct testcase_info *tc_info)
{
	printf("start_auto_test_item : LINE:%d, %s \r\n", tc_info->y,
	       tc_info->base_info->name);

	start_test_pthread(tc_info);

	return 0;
}

int run_test_item_cmd(char* item_bin)
{
    int ret = 0;
    int status = 0;
    if (item_bin == NULL)
        return -1;

    status = system(item_bin);
    if (status == -1) {
        printf("system cmd run :%s error...\n", item_bin);
        ret = -1;
    } else {
        printf("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
                printf("run item bin successfully.\n");
                ret = 0;
            } else {
                printf("run item bin fail, script exit code: %d\n", WEXITSTATUS(status));
                ret = -2;
            }
        } else {
            printf("exit status = [%d]\n", WEXITSTATUS(status));
            ret = -1;
        }
    }

    return ret;
}

int parse_test_result(char* rst_filename, char* test_item, char* para0)
{
    int ret = -1;
    char read_buf[128];
    char test_buf[10];
    int flag = 0;

    if (rst_filename == NULL || test_item == NULL)
        return -1;

    //read test result from /tmp dir
    if(access(rst_filename, F_OK) == 0) {
        //have find result file. we read this result file and parse.
        FILE *fp;

        fp = fopen(rst_filename, "r");
        memset(read_buf, 0 ,sizeof(read_buf));
        if (!fp) {
            printf("%s fopen err:%s\n",__func__,strerror(errno));
            return ret;
        }

        while(!feof(fp)) {
            char* test_ret= NULL;
            fgets(read_buf,sizeof(read_buf),fp);

            if((strstr(read_buf,test_item))!= NULL) {
                char *result = NULL;
                char delims[] = ",";

                result = strtok(read_buf, delims );
                while( result != NULL ) {
                    if(para0 != NULL && !flag) {
                        //just get extrernal first string, do'nt care others.
                        printf(">>>>> will get extern param is %s \n", result);
                        memcpy(para0, result, strlen(result));
                        flag = 1;
                    }

                    if(strstr(result,"PASS")!= NULL ||
                        strstr(result,"FAIL")!= NULL) {
                        test_ret = result;
                        break;
                    }
                    result = strtok( NULL, delims );
                }

                if(test_ret) {
                    char ch;
                    int i = 0;
                    ch = *test_ret;
                    while(ch != '\0') {
                        if (isalpha(ch)) {
                            test_buf[i++] = ch;
                            ch = *test_ret++;
                        } else {
                            ch = *test_ret++;
                        }
                    }
                    //printf(" >>>>> We get test result is %s <<<<<\n", test_buf);

                    if(0 == strcmp(test_buf, "PASS")) {
                        //pass
                        ret = 0;
                    } else if (0 == strcmp(test_buf, "FAIL")) {
                        //fail
                        ret= -1;
                    }
                } else {
                    printf(" >>>>> Error!!! wrong test result format. <<<<<\n");
                }
            }
        }
        fclose(fp);
    }

    return ret;
}

int main(int argc, char **argv)
{
	int ret, w;
	char *script_buf;
	struct list_head *pos;
	int success = 0;
	char rfCalResult[10];

#if 0
	freopen("/dev/ttyFIQ0", "a", stdout);
	setbuf(stdout, NULL);
	freopen("/dev/ttyFIQ0", "a", stderr);
	setbuf(stderr, NULL);
#endif
    printf("*****************************************\n");
    printf("***         pcba test start           ***\n");
    printf("***         Rockchip.Co.Ld.           ***\n");
    printf("*****************************************\n");

	ui_init();
	ui_set_background(BACKGROUND_ICON_INSTALLING);
	ui_print_init();
	w = gr_fb_width();
    printf(" @@@ w= %d \n", w);
	ui_print_xy_rgba((((w >> 1) - strlen(PCBA_VERSION_NAME)*CHAR_WIDTH/2)
		/CHAR_WIDTH), 0, 0, 255, 0, 255, "%s\n", PCBA_VERSION_NAME);

//	printf("Now in PCBA_test %d\n",__LINE__);
	ui_print_xy_rgba(((w >> 2) / CHAR_WIDTH - 4), 1, 255, 255, 0, 255,
			 "%s\n", PCBA_MANUAL_TEST);
//	printf("Now in PCBA_test %d\n",__LINE__);

//	drawline_4(255, 255, 0, 255, 0, (1 * CHAR_HEIGHT - (CHAR_HEIGHT>>2)),
//		w>>1, CHAR_HEIGHT, 3);

//	printf("Now in PCBA_test %d\n",__LINE__);
    //return 0;

	/*cur_p_y = (gr_fb_height() / CHAR_HEIGHT) - 1;*/

	INIT_LIST_HEAD(&manual_test_list_head);
	INIT_LIST_HEAD(&auto_test_list_head);
	script_buf = parse_script(SCRIPT_NAME);
	if (!script_buf) {
		printf("parse script failed\n");
		return -1;
	}

	ret = init_script(script_buf);
	if (ret) {
		db_error("core: init script failed(%d)\n", ret);
		return -1;
	}

	ret = parse_testcase();
	if (ret < 0) {
		db_error("core: parse all test case from script failed(%d)\n",
			 ret);
		return -1;
	} else if (ret == 0) {
		db_warn("core: NO TEST CASE to be run\n");
		return -1;
	}

	printf("\n\t manual testcase:\n");
	list_for_each(pos, &manual_test_list_head) {
		struct testcase_info *tc_info =
		    list_entry(pos, struct testcase_info, list);
		init_manual_test_item(tc_info);
	}
	manual_p_y += 1;

//	cur_p_y = manual_p_y+1;   /*for auto add items*/
    cur_p_y = MAX_ROWS / 2;
    ui_print_xy_rgba(((w >> 2) / CHAR_WIDTH - 4), cur_p_y - 1, 255, 255,
            0, 255, "%s\n", PCBA_AUTO_TEST);



//	ui_print_xy_rgba(((w >> 2) / CHAR_WIDTH - 4), manual_p_y, 255, 255,
//			 0, 255, "%s\n", PCBA_AUTO_TEST);
//	drawline_4(255, 255, 0, 255, 0,
//		   (CHAR_HEIGHT * (manual_p_y) - (CHAR_HEIGHT>>2)), w>>1,
//		   CHAR_HEIGHT, 3);

	printf("\n\t auto testcase:\n");
	list_for_each(pos, &auto_test_list_head) {
		struct testcase_info *tc_info =
		    list_entry(pos, struct testcase_info, list);
		start_auto_test_item(tc_info);
	}

	start_input_thread();

	printf("pcba test over!\n");

    //clear misc, and ready to entern main system.

	return success;
}


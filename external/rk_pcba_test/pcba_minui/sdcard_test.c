#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "extra-functions.h"
#include "common.h"
#include "sdcard_test.h"
#include "test_case.h"
#include "language.h"

#define LOG_TAG	"PCBA [sdcard]: "
#define LOG(x...)	printf(LOG_TAG x)

#define SCAN_RESULT_LENGTH	    128
#define SCAN_RESULT_FILE	    "/data/sd_capacity"
#define SD_INSERT_RESULT_FILE	"/data/sd_insert_info"

void *sdcard_test(void *argv)
{
	struct testcase_info *tc_info = (struct testcase_info *)argv;
	int ret, y;
	double cap;
	FILE *fp;
	char results[SCAN_RESULT_LENGTH];

	/* remind sdcard test */
	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();
	y = tc_info->y;

	LOG("start sdcard test.\n");

	//ui_print_xy_rgba(0, y, 255, 255, 0, 255, "%s\n", PCBA_SDCARD);
	ui_print_xy_rgba(0, y, 255, 255, 0, 255, "%s:[%s..]\n", PCBA_RTC,
			 PCBA_TESTING);

#if 0
#if defined(RK3288_PCBA)
	ret = __system("busybox chmod 777 /res/emmctester.sh");
#else
	ret = __system("busybox chmod 777 /res/mmctester.sh");
#endif

	if (ret)
		LOG("chmod mmctester.sh failed :%d\n", ret);

#if defined(RK3288_PCBA)
	ret = __system("/res/emmctester.sh");
#else
	ret = __system("/res/mmctester.sh");
#endif

	if (ret < 0) {
		LOG("mmc test failed.\n");
		ui_print_xy_rgba(0, y, 255, 0, 0, 255, "%s:[%s]\n", PCBA_SDCARD,
				 PCBA_FAILED);
		tc_info->result = -1;
		return argv;
	}

	fp = fopen(SCAN_RESULT_FILE, "r");
	if (fp == NULL) {
		LOG("can not open %s.\n", SCAN_RESULT_FILE);
		ui_print_xy_rgba(0, y, 255, 0, 0, 255, "%s:[%s]\n", PCBA_SDCARD,
				 PCBA_FAILED);
		tc_info->result = -1;
		return argv;
	}

	memset(results, 0, SCAN_RESULT_LENGTH);
	fgets(results, 50, fp);

	cap = strtod(results, NULL);
	if (cap) {
		ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s] { %2fG }\n",
				 PCBA_SDCARD, PCBA_SECCESS,
				 cap * 1.0 / 1024 / 1024);
		tc_info->result = 0;
	}
	fclose(fp);
#else

    int rst;
    char sd_buf[128]= {0};
    char sd_size[32] = {0};
    char result_filename[100] = {0};

    LOG("============= start sdcard test==============\n");
    rst = run_test_item_cmd("echo_auto_test echo_sdcard_test");

    if(rst == 0) {
        snprintf(result_filename, sizeof(result_filename),
		        "%s/echo_sdcard_test_result", "/tmp");
        ret = parse_test_result(result_filename, "sdcard_test", sd_buf);

    }else  {
        //rst < 0
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_SDCARD,
                 PCBA_FAILED, PCBA_SDCARD_NOINSERT);
        tc_info->result = -1;
        LOG("sdcard_test failed.\n");
        return NULL;
    }

    if(ret == 0) {
        if(strstr(sd_buf, "capacity") != NULL) {
            char *result = NULL;
            char delims[] = " ";

            memset(sd_size, 0, sizeof(sd_size));
            result = strtok(sd_buf, delims );
            while( result != NULL ) {
                LOG("result is \"%s\"\n", result);

                if(strstr(result,"capacity")!= NULL){
                    strcat(sd_size, result);
                }

                result = strtok( NULL, delims );
            }
        }
        ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s] { %s }\n", PCBA_SDCARD,
                 PCBA_SECCESS, sd_size);
        tc_info->result = 0;
        LOG("sdcard_test success.\n");
    } else {
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_SDCARD,
                 PCBA_FAILED, sd_size);
        tc_info->result = -1;
        LOG("sdcard_test failed.\n");
    }

#endif
	return argv;
}

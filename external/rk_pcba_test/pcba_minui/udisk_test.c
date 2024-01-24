#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"common.h"
//#include"extra-functions.h"

#include"udisk_test.h"
#include"test_case.h"
#include "language.h"

#define SCAN_RESULT_LENGTH 128
#define SCAN_RESULT_FILE "/data/udisk_capacity.txt"

#define LOG(x...)   printf("[UDISK_TEST] "x)


void * udisk_test(void * argv)
{
	struct testcase_info *tc_info = (struct testcase_info*)argv;
	int ret,y;
	double cap;
	FILE *fp;
	char results[SCAN_RESULT_LENGTH];

	/*remind ddr test*/
	if(tc_info->y <= 0)
		tc_info->y  = get_cur_print_y();

	y = tc_info->y;

#if 0   //Android pcba test

#ifdef SOFIA3GR_PCBA
	ui_print_xy_rgba(0,y,255,255,0,255,"%s:[%s..] \n",PCBA_UCARD,PCBA_TESTING);
#else
	ui_print_xy_rgba(0,y,255,255,0,255,"%s \n",PCBA_UCARD);
#endif


	#ifdef SOFIA3GR_PCBA
		ret =  __system("busybox chmod 777 /system/bin/udisktester.sh");
	#else
		ret =  __system("busybox chmod 777 /res/udisktester.sh");

	if(ret)
		printf("chmod udisktester.sh failed :%d\n",ret);
	#endif

	#ifdef SOFIA3GR_PCBA
		//ret = __system("/system/bin/udisktester.sh");
			int testCounts =0;
			while(1)
			{
				fp = NULL;
				LOG("%s::wait for insert udisk card...\n", __FUNCTION__);
				__system("/system/bin/udisktester.sh");

				fp = fopen(SCAN_RESULT_FILE, "r");
				if(fp != NULL)
				{
					LOG("%s line=%d find result file! \n", __FUNCTION__, __LINE__);
					break;
				}

				LOG("%s line=%d can't find result file! continue ... \n", __FUNCTION__, __LINE__);

				if(testCounts++ > 3)
				{
					LOG("can not open %s.\n", SCAN_RESULT_FILE);
					ui_print_xy_rgba(0,y,255,0,0,255,"%s:[%s]\n",PCBA_UCARD,PCBA_FAILED);
					tc_info->result = -1;
					return argv;
				}
				sleep(1);
			}

			//disable by wjh
			/*
			memset(results, 0, SCAN_RESULT_LENGTH);
			//fread(results, 1, SCAN_RESULT_LENGTH, fp);
			fgets(results,50,fp);
			//fgets(wlan_msg->ssid,50,fp); //we assume tha a AP's name is less of 50 charactes

			//LOG("%s.\n", results);

			cap = strtod(results,NULL);
			if(cap) {
				ui_print_xy_rgba(0,y,0,255,0,255,"%s:[%s] { %2fG } \n",PCBA_UCARD,PCBA_SECCESS,cap*1.0);
				tc_info->result = 0;
			}*/

			ui_print_xy_rgba(0,y,0,255,0,255,"%s:[%s]\n",PCBA_UCARD,PCBA_SECCESS);
				tc_info->result = 0;
		    fclose(fp);

			return argv;
	#else
		ret = __system("/res/udisktester.sh");
	if(ret < 0) {
		printf("udisk test failed.\n");
		ui_print_xy_rgba(0,y,255,0,0,255,"%s:[%s]\n",PCBA_UCARD,PCBA_FAILED);
		tc_info->result = -1;
		return argv;
	}
	#endif


	fp = fopen(SCAN_RESULT_FILE, "r");
	if(fp == NULL) {
		printf("can not open %s.\n", SCAN_RESULT_FILE);
		ui_print_xy_rgba(0,y,255,0,0,255,"%s:[%s]\n",PCBA_UCARD,PCBA_FAILED);
		tc_info->result = -1;
		return argv;
	}

  	memset(results, 0, SCAN_RESULT_LENGTH);
	fgets(results,50,fp);

	cap = strtod(results,NULL);
    printf("capacity : %s\n", results);
	if(cap > 0) {
		ui_print_xy_rgba(0,y,0,255,0,255,"%s:[%s] { %2fG }\n",PCBA_UCARD,PCBA_SECCESS,cap*1.0/1024/1024);
		tc_info->result = 0;
	}
    else {
        ui_print_xy_rgba(0,y,0,0,255,255,"%s:[%s]\n",PCBA_UCARD,PCBA_FAILED);
		tc_info->result = -1;
    }

        fclose(fp);

#else   //Linux pcba test

    int rst;
    char usbhost_buf[128]= {0};
    char usbhost_size[32] = {0};
    char result_filename[100] = {0};

    ui_print_xy_rgba(0,y,255,255,0,255,"%s:[wait %s..] : Pls plug usb host devices.\n",PCBA_UCARD,PCBA_TESTING);

    LOG("============= start udisk test==============\n");
    rst = run_test_item_cmd("echo_auto_test echo_usbhost_test");

    if(rst == 0) {
        snprintf(result_filename, sizeof(result_filename),
                "%s/echo_usbhost_test_result", "/tmp");
        ret = parse_test_result(result_filename, "usbhost_test", usbhost_buf);

    }else  {
        return NULL;
    }

    if(ret == 0) {
        if(strstr(usbhost_buf, "capacity") != NULL) {
            char *result = NULL;
            char delims[] = " ";

            memset(usbhost_size, 0, sizeof(usbhost_size));
            result = strtok(usbhost_buf, delims );
            while( result != NULL ) {
                LOG("result is \"%s\"\n", result);

                if(strstr(result,"capacity")!= NULL){
                    strcat(usbhost_size, result);
                }

                result = strtok( NULL, delims );
            }
        }
        ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s] { %s }\n", PCBA_UCARD,
                 PCBA_SECCESS, usbhost_size);
        tc_info->result = 0;
        LOG("usbhost_test success.\n");
    } else {
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s]\n", PCBA_UCARD,
                 PCBA_FAILED, usbhost_size);
        tc_info->result = -1;
        LOG("usbhost_test failed.\n");
    }

#endif
	return argv;

}

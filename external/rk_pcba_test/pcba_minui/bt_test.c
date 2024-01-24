#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include "common.h"
#include "language.h"
#include "test_case.h"

#define BLUETOOTH_TTY_TEST
//#define SOFIA3GR_PCBA
//#define BLUEDROID_TEST

#ifdef BLUEDROID_TEST
#include "libbluetooth/bluetooth/bluetooth.h"
#include "libbluetooth/bluetooth/hci.h"
#include "libbluetooth/bluetooth/hci_lib.h"

#ifndef HCI_DEV_ID
#define HCI_DEV_ID 0
#endif

#define HCID_START_DELAY_SEC   3
#define HCID_STOP_DELAY_USEC 500000

#define TAG	"[PCBA,BT]: "
#define LOG(x...)	printf(TAG x)

#define MIN(x,y) (((x)<(y))?(x):(y))

#if 0
#define BTHWCTL_DEV_NAME             "/dev/bthwctl"
#define BTHWCTL_IOC_MAGIC            0xf6
#define BTHWCTL_IOCTL_SET_POWER      _IOWR(BTHWCTL_IOC_MAGIC, 0, uint32_t)

#define TCC_BT_DEVICE_PATH                      "/dev/tcc_bt_dev"
#define BT_DEV_MAJOR_NUM                        234
#define IOCTL_BT_DEV_POWER              _IO(BT_DEV_MAJOR_NUM, 100)

enum WIFI_CHIP_TYPE_LIST{
    BT_UNKNOWN = -1,
	BCM4329 = 0,
	RTL8188CU,
	RTL8188EU,
	BCM4330,
	RK901,
	RK903,
	MT6620,
	RT5370,
    MT5931,
    RDA587x,
    RDA5990,
    RTK8723AS,
    RTK8723BS,
    RTK8723AU,
    RTK8723BU,
    BK3515,
    SOFIA_3GR,
};

static int rfkill_id = -1;
static char *rfkill_state_path = NULL;
static int bluetooth_power_status = 0;
static int chip_type;

#define WIFI_CHIP_TYPE_PATH "/sys/class/rkwifi/chip"
int getChipType(void) {
	  int wififd;
	  char buf[64];
	  int chip_type = RTL8188EU;

	  wififd = open(WIFI_CHIP_TYPE_PATH, O_RDONLY);
	  if( wififd < 0 ){
	          printf("Can't open %s, errno = %d\n", WIFI_CHIP_TYPE_PATH, errno);
	          goto done;
	  }

	  memset(buf, 0, 64);
	  if( 0 == read(wififd, buf, 10) ){
	          printf("read failed\n");
	          close(wififd);
	          goto done;
	  }
	  close(wififd);

	  if(0 == strncmp(buf, "BCM4329", strlen("BCM4329")) ) {
	          chip_type = BCM4329;
	          printf("Read wifi chip type OK ! chip_type = BCM4329\n");
	  }
	  else if (0 == strncmp(buf, "RTL8188CU", strlen("RTL8188CU")) ) {
	          chip_type = RTL8188CU;
	          printf("Read wifi chip type OK ! chip_type = RTL8188CU\n");
	  }
	  else if (0 == strncmp(buf, "RTL8188EU", strlen("RTL8188EU")) ) {
	          chip_type = RTL8188EU;
	          printf("Read wifi chip type OK ! chip_type = RTL8188EU\n");
	  }
	  else if (0 == strncmp(buf, "BCM4330", strlen("BCM4330")) ) {
	          chip_type = BCM4330;
	          printf("Read wifi chip type OK ! chip_type = BCM4330\n");
	  }
	  else if (0 == strncmp(buf, "RK901", strlen("RK901")) ) {
	          chip_type = RK901;
	          printf("Read wifi chip type OK ! chip_type = RK901\n");
	  }
	  else if (0 == strncmp(buf, "RK903", strlen("RK903")) ) {
	          chip_type = RK903;
	          printf("Read wifi chip type OK ! chip_type = RK903\n");
	  }
	  else if (0 == strncmp(buf, "MT6620", strlen("MT6620")) ) {
	          chip_type = MT6620;
	          printf("Read wifi chip type OK ! chip_type = MT6620\n");
	  }
	  else if (0 == strncmp(buf, "RT5370", strlen("RT5370")) ) {
	          chip_type = RT5370;
	          printf("Read wifi chip type OK ! chip_type = RT5370\n");
	  }
	  else if (0 == strncmp(buf, "MT5931", strlen("MT5931")) ) {
	          chip_type = MT5931;
	          printf("Read wifi chip type OK ! chip_type = MT5931\n");
	  }

done:
	  return chip_type;
}

static int init_rfkill() {
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;
    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            LOGW("open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 9 && memcmp(buf, "bluetooth", 9) == 0) {
            rfkill_id = id;
            break;
        }
    }

    asprintf(&rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", rfkill_id);
    return 0;
}

static int broadcom_set_bluetooth_power(int on) {
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    if (rfkill_id == -1) {
        if (init_rfkill()) goto out;
    }

    fd = open(rfkill_state_path, O_WRONLY);
    if (fd < 0) {
        printf("open(%s) for write failed: %s (%d)\n", rfkill_state_path,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        printf("write(%s) failed: %s (%d)\n", rfkill_state_path, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}

static int mtk_set_bluetooth_power(int on) {
    int sz;
    int fd = -1;
    int ret = -1;
    const uint32_t buf = (on ? 1 : 0);

    fd = open(BTHWCTL_DEV_NAME, O_RDWR);
    if (fd < 0) {
        LOGE("Open %s to set BT power fails: %s(%d)", BTHWCTL_DEV_NAME,
             strerror(errno), errno);
        goto out1;
    }

    ret = ioctl(fd, BTHWCTL_IOCTL_SET_POWER, &buf);
    if(ret < 0) {
        LOGE("Set BT power %d fails: %s(%d)\n", buf,
             strerror(errno), errno);
        goto out1;
    }

    bluetooth_power_status = on ? 1 : 0;

out1:
    if (fd >= 0) close(fd);
    return ret;
}

static int rda587x_set_bluetooth_power(int on) {
	int fd = -1;
	int bt_on_off = -1;

	fd = open(TCC_BT_DEVICE_PATH, O_RDWR);
	if( fd < 0 )
	{
	    printf("[###### TCC BT #######] [%s] open error[%d]\n", TCC_BT_DEVICE_PATH, fd);
	    return -1;
	}
	else
	{
	    bt_on_off = 0;
	    ioctl(fd, IOCTL_BT_DEV_POWER, &bt_on_off);//make sure bt is disabled
	    printf("[##### TCC BT #####] set_bluetooth_power [%d]\n", bt_on_off);
	    bt_on_off = 1;
	    ioctl(fd, IOCTL_BT_DEV_POWER, &bt_on_off);
	    printf("[##### TCC BT #####] set_bluetooth_power [%d]\n", bt_on_off);
	    close(fd);
	    return 0;
	}
}

static int rda5990_set_bluetooth_power(int on) {
	return 0;
}

static int set_bluetooth_power(int on) {
	if(chip_type == MT5931) {
		return mtk_set_bluetooth_power(on);
	} else if(chip_type == RDA587x) {
		return rda587x_set_bluetooth_power(on);
	} else if(chip_type == RDA5990) {
		return rda5990_set_bluetooth_power(on);
	} else {
		return broadcom_set_bluetooth_power(on);
	}
}

/**
@ret:
  >=0 , socket created ok;
  <0, socket created fail;
*/
static inline int bt_test_create_sock() {
    int sock = 0;
	sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (sock < 0) {
        printf("bluetooth_test Failed to create bluetooth hci socket: %s (%d)\n",
             strerror(errno), errno);
		return -1;
    }
    return sock;
}

#if 1
static int start_hciattach() {
	int ret;
	if(chip_type == MT5931) {
	  ret = __system("/system/bin/hciattach_mtk -n -t 10 -s 115200 /dev/ttyS0 mtk 1500000 noflow &");
	} else if(chip_type == RDA587x) {
	  ret = __system("/system/bin/hciattach_5876 -n -s 115200 /dev/ttyS0 rda 1500000 noflow &");
	} else if(chip_type == RDA5990) {
	  ret = __system("/system/bin/hciattach_5990 -n -s 115200 /dev/ttyS0 rda 921600 noflow &");
	} else if(chip_type == RTK8723AS) {
	  ret = __system("/system/bin/hciattach_8723 -n -s 115200  /dev/ttyS0 rtk_h5 &");
	} else if(chip_type == RTK8723AU) {
	  ret = __system("insmod /res/rtk_btusb.ko");
	} else {
		ret = __system("/system/bin/brcm_patchram_plus --patchram bychip --baudrate 1500000 --enable_lpm --enable_hci /dev/ttyS0 &");
	}
	return ret;
}
#else
static int start_hciattach() {
	int ret;
	char service_name[32];

	if(chip_type == MT5931) {
		strcpy(service_name, "hciattach_mtk");
	} else if(chip_type == RDA587x) {
	  strcpy(service_name, "hciattach_587x");
	} else if(chip_type == RDA5990) {
	  strcpy(service_name, "hciattach_5990");
	} else {
		strcpy(service_name, "hciattach_brm");
	}

	if (property_set("ctl.start", service_name) < 0) {
		printf("bluetooth_test Failed to start %s\n", service_name);
		return -1;
	}

	return ret;
}
#endif

static int bt_test_enable() {
    int ret = -1;
    int hci_sock = -1;
    int attempt;

    if (set_bluetooth_power(1) < 0) goto out;

    printf("Starting hciattach daemon\n");
    if (start_hciattach() != 0) {
        printf("Failed to start hciattach\n");
        set_bluetooth_power(0);
        goto out;
    }

    // Try for 10 seconds, this can only succeed once hciattach has sent the
    // firmware and then turned on hci device via HCIUARTSETPROTO ioctl
    printf("Waiting for HCI device present...\n");
    for (attempt = 50; attempt > 0;  attempt--) {
        printf("..%d..\n", attempt);
        hci_sock = bt_test_create_sock();
        if (hci_sock < 0) goto out;

        ret = ioctl(hci_sock, HCIDEVUP, HCI_DEV_ID);

        if (!ret) {
            break;
        } else if (errno == EALREADY) {
            printf("Bluetoothd already started, unexpectedly!\n");
            break;
        }

        close(hci_sock);
        usleep(200000);  // 200 ms retry delay
    }
    if (attempt == 0) {
        printf("%s: Timeout waiting for HCI device to come up, ret=%d\n",
            __FUNCTION__, ret);
        set_bluetooth_power(0);
        goto out;
    }

		printf("bt_enable success.\n");
    ret = 0;

out:
    if (hci_sock >= 0) close(hci_sock);
    return ret;
}

static int my_ba2str(const bdaddr_t *ba, char *str) {
    return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
                ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int bt_get_chipname(char* name, int len)
{
    int ret = -1;
    int fd = -1;
    int sz = 0;
    char* rfkill_name_path = NULL;

    if (rfkill_id == -1) {
        if (init_rfkill()) goto out;
    }

    asprintf(&rfkill_name_path, "/sys/class/rfkill/rfkill%d/name", rfkill_id);

    fd = open(rfkill_name_path, O_RDONLY);
    if (fd < 0) {
        printf("open(%s) failed: %s (%d)", rfkill_name_path, strerror(errno),
             errno);
        goto out;
    }

    sz = read(fd, name, len);
    if (sz < 0) {
        printf("read(%s) failed: %s (%d)", rfkill_name_path, strerror(errno),
             errno);
        goto out;
    }
    name[sz] = '\0';
    if (name[sz-1]=='\n')
        name[sz-1] = '\0';

    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}

int check_bluedroid_test()
{
	FILE *fp;

	fp = fopen("/data/bt_success.txt", "r");
	if(fp != NULL) {
		printf("check_bluedroid_test: success.\n");
		fclose(fp);
		fp = NULL;
		return 1;
	}

	//fp = fopen("/data/bt_fail.txt", "r");
	//if(fp != NULL) {
	//	printf("check_bluedroid_test: fail.\n");
	//	fclose(fp);
	//	fp = NULL;
	//	return -1;
	//}

	return 0; // wait
}

int bluedroid_test()
{
	int ret, counts = 10;

    if (chmod(BTHWCTL_DEV_NAME, 0660) < 0) {
        printf("Error changing permissions of %s to 0660: %s",
                BTHWCTL_DEV_NAME, strerror(errno));
        unlink(BTHWCTL_DEV_NAME);
    }

    if (chmod("/sys/class/rfkill/rfkill0/state", 0775) < 0) {
        printf("Error changing permissions of %s to 0660: %s",
                "/sys/class/rfkill/rfkill0/state", strerror(errno));
        unlink("/sys/class/rfkill/rfkill0/state");
    }

    if (chmod("/sys/class/rfkill/rfkill0/type", 0775) < 0) {
        printf("Error changing permissions of %s to 0660: %s",
                "/sys/class/rfkill/rfkill0/type", strerror(errno));
        unlink("/sys/class/rfkill/rfkill0/type");
    }

    if (chmod("/data", 0775) < 0) {
        printf("Error changing permissions of %s to 0660: %s",
                "/data", strerror(errno));
        unlink("/data");
    }

    if (chmod("/dev/ttyS0", 0775) < 0) {
        printf("Error changing permissions of %s to 0775 %s",
                "/dev/ttyS0", strerror(errno));
    }

    printf("bluedroid_test: start bdt test:\n");

    ret = __system("/system/bin/bdt &");
    if(ret != 0) {
    	printf("bluedroid_test: start bdt failed.\n");
    	return -1;
    }

    while(counts-- > 0) {
    	ret = check_bluedroid_test();
    	if(ret == 1) {
    		break;
    	}
    	usleep(1000000);
    }
    if (counts == 0) {
        printf("bluedroid_test: waitting for bt test ready timeout!\n");
        ret = -1;
    }

    return ret;
}

static char bt_chip[64] = "";

int bt_test_bluez()
{
    int dev_id = 0;
	int sock = 0;
    int i = 0;
	int ret = 0;
	char dt[32] = {0};
	chip_type = RK903;
	if(script_fetch("bluetooth", "chip_type", (int *)dt, 8) == 0) {
		printf("script_fetch chip_type = %s.\n", dt);
	}
	if(strcmp(dt, "rk903") == 0) {
		chip_type = RK903;
	} else if(strcmp(dt, "mt6622") == 0) {
		chip_type = MT5931;
	} else if(strcmp(dt, "rda587x") == 0) {
		chip_type = RDA587x;
	} else if(strcmp(dt, "rda5990") == 0) {
		chip_type = RDA5990;
	} else if(strcmp(dt, "rtk8723as") == 0) {
		chip_type = RTK8723AS;
	} else if(strcmp(dt, "rtk8723bs") == 0) {
		chip_type = RTK8723BS;
	} else if(strcmp(dt, "rtk8723au") == 0) {
		chip_type = RTK8723AU;
	} else if(strcmp(dt, "rtk8723bu") == 0) {
		chip_type = RTK8723BU;
    } else if(strcmp(dt, "bk3515") == 0) {
        chip_type = BK3515;
		sleep(5);
	} else {
		if (bt_get_chipname(bt_chip, 63) != 0) {

		    printf("Can't read BT chip name\n");
			goto fail;
		}

		if (!strcmp(bt_chip, "rk903_26M"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "rk903"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6210"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6330"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6476"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6493"))
		    chip_type = RK903;
		else {
		    printf("Not support BT chip, skip bt test.\n");
			goto fail;
		}
	}

	printf("bluetooth_test main function started: chip_type = %d\n", chip_type);

	if(chip_type == RTK8723BS || chip_type == BK3515) {
		ret = bluedroid_test();
		if(ret == 1) {
			printf("bluetooth_test success.\n");
			goto success;
		} else {
			printf("bluetooth_test fail.\n");
			goto fail;
		}
	}

	if(chip_type == RTK8723AU || chip_type == RTK8723BU) {
		int ret;
		ret = __system("busybox dmesg | busybox grep 'hci_register_dev success'");
		printf("a:ret = %d.\n", ret);
        if (ret != 0) {
		    ret = __system("insmod /res/rtk_btusb.ko");
		    ret = __system("busybox dmesg | busybox grep 'hci_register_dev success'");
        }
		printf("b:ret = %d.\n", ret);
		if(ret != 0) {
			printf("bluetooth_test fail.\n");
			goto fail;
		}
		printf("bluetooth_test success.\n");
		goto success;
	}

	ret = bt_test_enable();
	if(ret < 0){
		printf("bluetooth_test main function fail to enable \n");
		goto fail;
	}

	dev_id = hci_get_route(NULL);

	if(dev_id < 0){
		printf("bluetooth_test main function fail to get dev id\n");
		goto fail;
	}

	printf("bluetooth_test main function hci_get_route dev_id=%d\n",dev_id);

    sock = hci_open_dev( dev_id );
	if(sock < 0){
		printf("bluetooth_test main function fail to open bluetooth sock\n");
		goto fail;
	}

	printf("bluetooth_test main function hci_open_dev ok\n");

	if(sock >= 0){
		close( sock );
	}

	/*ret = bt_test_disable();
	if(ret < 0){
		printf("bluetooth_test main function fail to disable\n");
		ui_print_xy_rgba(0,tc_info->y,255,0,0,255,"bluetooth test error\n");
		return 0;
	}*/

success:
    printf("bluetooth_test main function end\n");
    return 0;

fail:
    printf("bluetooth_test main function end\n");
    return -1;
}

#define LOG(x...)   printf("[BT_TEST] "x)

static int get_chip_type()
{
	char dt[32] = {0};
	chip_type = RK903;
	if(script_fetch("bluetooth", "chip_type", (int *)dt, 8) == 0) {
		LOG("script_fetch chip_type = %s.\n", dt);
	}
	if(strcmp(dt, "rk903") == 0) {
		chip_type = RK903;
	} else if(strcmp(dt, "mt6622") == 0) {
		chip_type = MT5931;
	} else if(strcmp(dt, "rda587x") == 0) {
		chip_type = RDA587x;
	} else if(strcmp(dt, "rda5990") == 0) {
		chip_type = RDA5990;
	} else if(strcmp(dt, "rtk8723as") == 0) {
		chip_type = RTK8723AS;
	} else if(strcmp(dt, "rtk8723bs") == 0) {
		chip_type = RTK8723BS;
	} else if(strcmp(dt, "rtk8723au") == 0) {
		chip_type = RTK8723AU;
	} else if(strcmp(dt, "rtk8723bu") == 0) {
		chip_type = RTK8723BU;
    } else if(strcmp(dt, "bk3515") == 0) {
        chip_type = BK3515;
	} else if(strcmp(dt, "Sofia-3gr") == 0) {
        chip_type = SOFIA_3GR;
	} else {
		if (bt_get_chipname(bt_chip, 63) != 0) {
		    LOG("Can't read BT chip name\n");
			chip_type = BT_UNKNOWN;
		}

		if (!strcmp(bt_chip, "rk903_26M"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "rk903"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6210"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6330"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6476"))
		    chip_type = RK903;
		else if (!strcmp(bt_chip, "ap6493"))
		    chip_type = RK903;
		else {
		    LOG("Not support BT chip, skip bt test.\n");
			chip_type = BT_UNKNOWN;
		}
	}

	LOG("chip type is: %d\n", chip_type);

    return chip_type;
}

static void change_mode()
{
    if (chmod(BTHWCTL_DEV_NAME, 0660) < 0) {
        LOG("Error changing permissions of %s to 0660: %s\n",
                BTHWCTL_DEV_NAME, strerror(errno));
        unlink(BTHWCTL_DEV_NAME);
    }

    if (chmod("/sys/class/rfkill/rfkill0/state", 0775) < 0) {
        LOG("Error changing permissions of %s to 0660: %s\n",
                "/sys/class/rfkill/rfkill0/state", strerror(errno));
        unlink("/sys/class/rfkill/rfkill0/state");
    }

    if (chmod("/sys/class/rfkill/rfkill0/type", 0775) < 0) {
        LOG("Error changing permissions of %s to 0660: %s\n",
                "/sys/class/rfkill/rfkill0/type", strerror(errno));
        unlink("/sys/class/rfkill/rfkill0/type");
    }

    if (chmod("/dev/rtk_btusb", 0775) < 0) {
        LOG("Error changing permissions of %s to 0660: %s\n",
                "/dev/rtk_btusb", strerror(errno));
    }

    if (chmod("/dev/ttyS0", 0775) < 0) {
        LOG("Error changing permissions of %s to 0775 %s\n",
                "/dev/ttyS0", strerror(errno));
    }

    LOG("Change mode finish\n");

}

static int bt_test_bluedroid()
{
    int ret = -1;
    FILE *fp=NULL;
    int try = 20, bytes_for_second = 10*1024;
    char *result_buf, *buf;
    int result_size = try * bytes_for_second;

    result_buf = malloc(result_size);
    if (result_buf == NULL) {
        LOG("malloc result_buf fail\n");
        return ret;
    }
    buf = result_buf;

    change_mode();

    fp = popen("echo \"enable\" | bdt", "r");
    if (fp != NULL) {
        int fd = fileno(fp);
        int flags = fcntl(fd, F_GETFL, 0);
        int len = 0;
        fcntl(fd, F_SETFL, flags|O_NONBLOCK);

        LOG("running bdt for bluetooth test...\n");
        while (try-->0) {
            buf += len;
            len = fread(buf, sizeof(char), bytes_for_second, fp);
            if (len) {
                //LOG("read: %s\n", buf);
                if (strstr(result_buf, "ADAPTER STATE UPDATED : ON")) {
                    LOG("bt test success!\n");
                    ret = 0;
                    break;
                }
            }
            LOG("wait %d\n", try);
            sleep(1);
        }
        pclose(fp);
        if (try<=0)
            LOG("bt test timeout!\n");
    } else
        LOG("run bdt fail!\n");

    free(result_buf);
    return ret;
}

#elif defined(SOFIA3GR_PCBA)
int check_Sofia3gr_bluedroid_test()//add by wjh
{
	LOG("Sofia3gr_bluedroid_test: start check bdt test result.\n");

	int ret = -1;
    FILE *fp=NULL;
    int try = 10, bytes_for_second = 10*1024;
    char *result_buf, *buf;
    int result_size = try * bytes_for_second;

    result_buf = malloc(result_size);
    if (result_buf == NULL) {
        LOG("malloc result_buf fail\n");
        return ret;
    }
    buf = result_buf;

	fp = popen("echo \"enable\" | bdt", "r");
    if (fp != NULL) {
        int fd = fileno(fp);
        int flags = fcntl(fd, F_GETFL, 0);
        int len = 0;
        fcntl(fd, F_SETFL, flags|O_NONBLOCK);

        LOG("running bdt for bluetooth test...\n");
        while (try-->0) {
            buf += len;
            len = fread(buf, sizeof(char), bytes_for_second, fp);
            if (len) {
                //LOG("read: %s\n", buf);
                if (strstr(result_buf, "ADAPTER STATE UPDATED : ON")) {
                    LOG("bt test success!\n");
                    ret = 0;
                    break;
                }
            }
            LOG("wait %d\n", try);
            sleep(1);
        }
        pclose(fp);
        if (try<=0)
            LOG("bt test timeout!\n");
    } else
        LOG("run bdt fail!\n");

    free(result_buf);
	popen("echo \"quite\" | bdt", "r");
    return ret;
}


int Sofia3gr_bluedroid_test()//add by wjh
{
	int ret, counts = 3;

	LOG("Sofia3gr_bluedroid_test: start bdt.\n");
	while(counts-- > 0)
	{
		ret = check_Sofia3gr_bluedroid_test();
		if(ret == 0)
		{
			break;
		}
	}
	return ret;
}

#else
int uart_fd = -1;
struct termios termios;
unsigned char  buffer[1024];
int ttytestResult= -1;
unsigned char hci_reset[] = { 0x01, 0x03, 0x0c, 0x00 };
unsigned char hci_rtksyc[] = { 0xc0, 0x00, 0x2f, 0x00,0xd0, 0x01,0x7e,0xc0};


void
init_uart_brcm()
{
	tcflush(uart_fd, TCIOFLUSH);
	int n = tcgetattr(uart_fd, &termios);
	printf("tcgetattr %d\n",n);

#ifndef __CYGWIN__
	cfmakeraw(&termios);
#else
	termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                | INLCR | IGNCR | ICRNL | IXON);
	termios.c_oflag &= ~OPOST;
	termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios.c_cflag &= ~(CSIZE | PARENB);
	termios.c_cflag |= CS8;
#endif

	//termios.c_cflag |= CRTSCTS;
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcflush(uart_fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(uart_fd, TCSANOW, &termios);
}


void
init_uart_rtk()
{
	tcflush(uart_fd, TCIOFLUSH);
	int n = tcgetattr(uart_fd, &termios);
	printf("tcgetattr %d\n",n);

#ifndef __CYGWIN__
	cfmakeraw(&termios);
#else
	termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                | INLCR | IGNCR | ICRNL | IXON);
	termios.c_oflag &= ~OPOST;
	termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios.c_cflag &= ~(CSIZE | PARENB);
	termios.c_cflag |= CS8;
#endif
		termios.c_cflag &= ~CRTSCTS;

	termios.c_cflag |= PARENB;

	//termios.c_cflag |= CRTSCTS;
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcflush(uart_fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(uart_fd, TCSANOW, &termios);
}

void
dump(unsigned char *out, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i && !(i % 16)) {
			printf("\n");
		}

		printf("%02x ", out[i]);
	}

	printf("\n");
}


#define READTTY_TIMEOUT  30//3s
int readttyLen(int fd,unsigned char *buffer,int len)
{
	int count;
	int i= 0;
	int timeout=0;
	while(len){
		count = read(fd,&buffer[i],1);
		if(count == 1){
			i += count;
			len -= count;
		}
		else{
			usleep(100000);//100ms
			timeout ++;
			//printf("timeout %d\n", timeout);
			if(timeout >= READTTY_TIMEOUT)
				return -1;
		}
	}
	return i;
}
void readBrcmTty(int fd, unsigned char  *buffer)
{
	int i=0;
	int count;
	int len;
	count = readttyLen(fd,buffer,3);
	printf("readBrcmTty count11 %d\n", count);
	if(count < 3)
		return;
	i += count;
	len = buffer[2];

	count = readttyLen(fd,&buffer[i],len);
	if(count<len)
		return;
	i += count;

	//if (debug)
	{

		printf("readBrcmTty received %d\n", i);
		dump(buffer, i);
	}

	ttytestResult = 0;
	printf("bt ttytest read_event succ\n");
}

void readRtkTty(int fd, unsigned char  *buffer)
{
	int i=0;
	int count;
	int len;

	count = readttyLen(fd,buffer,16);
	if(count < 16)
		return;
	i += count;

	//if (debug)
	{

		printf("received %d\n", i);
		dump(buffer, i);
	}

	ttytestResult = 0;
	printf("bt ttytest read_event succ\n");
}


void
hci_send_cmd(unsigned char *buf, int len)
{
	//if (debug)
	{
		printf("writing\n");
		dump(buf, len);
	}

	int writelen=write(uart_fd, buf, len);
	printf("writelen %d\n",writelen);
}


void
expired(int sig)
{
    ttytestResult = -1;
	printf("bt ttytest expired\n");
}

void
proc_reset()
{
	signal(SIGALRM, expired);

	printf( "proc_reset");
	alarm(8);

	hci_send_buf(hci_reset, sizeof(hci_reset));

	read_event(uart_fd, buffer);

	alarm(0);
}

#define CONF_COMMENT '#'
#define CONF_DELIMITERS " =\n\r\t"
#define CONF_VALUES_DELIMITERS "=\n\r\t"
#define CONF_MAX_LINE_LEN 255
void get_tty_conf(const char *p_path,char *ttyPort)
{
    FILE    *p_file;
    char    *p_name;
    char    *p_value;
    char    line[CONF_MAX_LINE_LEN+1]; /* add 1 for \0 char */

    printf( "Attempt to load conf from %s", p_path);

    if ((p_file = fopen(p_path, "r")) != NULL)
    {
        /* read line by line */
        while (fgets(line, CONF_MAX_LINE_LEN+1, p_file) != NULL)
        {
            if (line[0] == CONF_COMMENT)
                continue;

            p_name = strtok(line, CONF_DELIMITERS);

            if (NULL == p_name)
            {
                continue;
            }

            p_value = strtok(NULL, CONF_DELIMITERS);

            if (NULL == p_value)
            {
                printf( "vnd_load_conf: missing value for name: %s", p_name);
                continue;
            }

            if (strcmp("UartPort", (const char *)p_name) == 0){
				printf("get ttyPort %s", p_value);
				strcpy(ttyPort,p_value);
				fclose(p_file);
				return;
            }

        }

        fclose(p_file);
    }
    else
    {
        printf( "vnd_load_conf file >%s< not found", p_path);
    }
	strcpy(ttyPort,"/dev/ttyS0");
}

int test_rtktty()
{
	init_uart_rtk();
	hci_send_cmd(hci_rtksyc, sizeof(hci_rtksyc));
	readRtkTty(uart_fd, buffer);
	return ttytestResult;
}
int test_brcmtty()
{
	init_uart_brcm();
	hci_send_cmd(hci_reset, sizeof(hci_reset));
	readBrcmTty(uart_fd, buffer);
	return ttytestResult;
}

static void ttytestThread(void *param)
{
	char ttyPort[30]={0};
	__system("echo 1 > /sys/class/rfkill/rfkill0/state");
	sleep(1);
    get_tty_conf("/vendor/etc/bluetooth/bt_vendor.conf",ttyPort);
	if ((uart_fd = open(ttyPort, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1) {
		printf( "port could not be opened, error %d\n", errno);
	}

	int i;
	for(i=0;i<3;i++){
	 if(test_brcmtty()>=0)
	 	return;
	 if(test_rtktty()>=0)
	 	return;
	}

}

int bluetoothtty_test()
{
    int i;

	pthread_t thread_id;
	pthread_create(&thread_id, NULL,(void*)ttytestThread, NULL);
	for(i=10;i>0;i--){
		sleep(1);
		if(ttytestResult == 0)
			return 0;
	}
	return -1;

}
#endif

#endif

void *bt_test(void *argv)
{
	struct testcase_info *tc_info = (struct testcase_info *)argv;
    int ret = 0;

	/*remind ddr test*/
	if(tc_info->y <= 0)
		tc_info->y  = get_cur_print_y();

	ui_print_xy_rgba(0,tc_info->y,255,255,0,255,"%s:[%s..] \n",
	                PCBA_BLUETOOTH,PCBA_TESTING);

#if 0    //Android pcba test
#if defined(SOFIA3GR_PCBA)
       ret = Sofia3gr_bluedroid_test();
#elif defined (BLUETOOTH_TTY_TEST)
	ret = bluetoothtty_test();
#else
	switch (get_chip_type()) {
    case RK903:
    case RTK8723BS:
    case BK3515:
        ret = bt_test_bluedroid();
        break;
	/*case SOFIA_3GR:
		ret = Sofia3gr_bluedroid_test();
        break;*/
    default:
        ret = bt_test_bluedroid();//bt_test_bluez();
        break;
    }
#endif
#else  //Linux pcba test
    int rst;
    char result_filename[100] = {0};

    printf("======================start bt test=============================\n");
    rst = run_test_item_cmd("echo_auto_test echo_bt_test");

    if(rst == 0) {
        snprintf(result_filename, sizeof(result_filename),
                "%s/echo_bt_test_result", "/tmp");
        ret = parse_test_result(result_filename, "bt_test", NULL);

    }else  {
        return NULL;
    }

#endif

    if (ret==0) {
        ui_print_xy_rgba(0,tc_info->y,0,255,0,255,"%s:[%s]\n",
                        PCBA_BLUETOOTH,PCBA_SECCESS);
		tc_info->result = 0;
    }
    else {
        ui_print_xy_rgba(0,tc_info->y,255,0,0,255,"%s:[%s]\n",
                        PCBA_BLUETOOTH,PCBA_FAILED);
		tc_info->result = -1;
    }
	return 0;
}


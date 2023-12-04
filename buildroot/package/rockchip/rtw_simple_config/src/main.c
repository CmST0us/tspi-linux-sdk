#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#include "sc_common.h"
#ifdef PLATFORM_MSTAR
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

//#define DEFAULT_WLAN_BAND "bgn" //abgn
#define _FW_UNDER_SURVEY	0x0800
#define BUF_SIZ 10240

#define DEST_MAC0 0x01
#define DEST_MAC1 0x00
#define DEST_MAC2 0x5e

static void ProcessPacket(unsigned char* , int, int);
int collect_scanres();
void doRecvfrom();


u8 g_abort = FALSE, g_reinit;
char ifName[IFNAMSIZ];
const unsigned char bc_mac[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const char *flag_str[] = {	"SC_SCAN", "SC_SAVE_PROFILE", "SC_DEL_PROFILE",
				"SC_RENAME", "SC_SUCCESS_ACK" };
char g_wpafile_path[512];
char wifi_proc_path[64];

void wifi_do_scan(void);
void wifi_disable(void);
void wifi_enable(void);
void RTK_SYSTEM(const char *cmd);

//   =====================	Custimize part	====================================

#define MAX_LINE_LEN		1024
#define MAX_SCAN_TIMES		10


struct _chplan g_chPlan[] = {
		{ .ch = 1,   	.bw_os = '+', },
		{ .ch = 6,   	.bw_os = '-', },
		{ .ch = 11,   	.bw_os = '-', },
		{ .ch = 2,   	.bw_os = '+', },
		{ .ch = 3,   	.bw_os = '+', },
		{ .ch = 4,   	.bw_os = '+', },
		{ .ch = 5,   	.bw_os = '-', },
		{ .ch = 7,   	.bw_os = '-', },
		{ .ch = 8,   	.bw_os = '-', },
		{ .ch = 9,   	.bw_os = '-', },
		{ .ch = 10,   	.bw_os = '-', },
		{ .ch = 12,   	.bw_os = '-', },
		{ .ch = 13,   	.bw_os = '-', },

/*
		// 5G, Band1		
		{ .ch = 36,   	.bw_os = '+', },
		{ .ch = 40,   	.bw_os = '-', },
		{ .ch = 44,   	.bw_os = '+', },
		{ .ch = 48,   	.bw_os = '-', },
		
		// 5G, Band2
		{ .ch = 52,   	.bw_os = '+', },
		{ .ch = 56,   	.bw_os = '-', },
		{ .ch = 60,   	.bw_os = '+', },
		{ .ch = 64,   	.bw_os = '-', },
		
		// 5G, Band3
		{ .ch = 100,   	.bw_os = '+', },
		{ .ch = 104,   	.bw_os = '-', },
		{ .ch = 108,   	.bw_os = '+', },
		{ .ch = 112,   	.bw_os = '-', },
		{ .ch = 116,   	.bw_os = '+', },
		{ .ch = 132,   	.bw_os = '-', },
		{ .ch = 136,   	.bw_os = '+', },
		{ .ch = 140,   	.bw_os = '0', },

		// 5G, Band4
		{ .ch = 149,   	.bw_os = '+', },
		{ .ch = 153,   	.bw_os = '-', },
		{ .ch = 157,   	.bw_os = '+', },
		{ .ch = 161,   	.bw_os = '-', },
		{ .ch = 165,   	.bw_os = '0', },
*/
};


#define SIG_SCPBC		SIGUSR1


#ifdef PLATFORM_MSTAR
int MSTAR_SYSTEM(const char * cmdstring)
{
    pid_t pid;
    int status;
 
    if (cmdstring == NULL) {
        return (1);
    }
 
    if ((pid = fork())<0) {
        status = -1;
    } else if (pid == 0) {
        execl("/sbin/busybox", "sh", "-c", cmdstring, (char *)0);	//Aries , ms918
        //execl("/system/xbin/busybox", "sh", "-c", cmdstring, (char *)0);
        exit(127);
    } else {
        sleep(1);
        while (waitpid(pid, &status, 0) < 0){
            if (errno != EINTR) {
                status = -1;
                break;
            }
        }
    }
 
    return status;
}
#endif

void RTK_SYSTEM(const char *cmd)
{
	DEBUG_PRINT("shell:  %s\n", cmd);
#ifdef	PLATFORM_MSTAR
	MSTAR_SYSTEM(cmd);
#else
	system(cmd);
#endif
}

/*
description:	the way of system to connect to tartget AP.
*/
int connect_ap(void)
{
	char cmdstr[200];
	
#ifndef ANDROID_ENV
	//  reinit module status
	wifi_disable();
	wifi_enable();
#ifdef CONFIG_IOCTL_CFG80211
	sprintf(cmdstr, "wpa_supplicant -i %s -c %s -Dnl80211 &", ifName, g_wpafile_path);
#else
	sprintf(cmdstr, "wpa_supplicant -i %s -c %s -Dnl80211 &", ifName, g_wpafile_path);
#endif
	RTK_SYSTEM(cmdstr);
	sleep(5);
#else
	//sprintf(cmdstr, "rmmod %s", WIFI_MODULE_NAME);
	//RTK_SYSTEM(cmdstr);
	RTK_SYSTEM("svc wifi enable");
#endif
	return 0;
}

/*
description:	the way of system to get IP address.
*/
int request_ip(void)
{
	char cmdstr[250];
	
	sprintf(cmdstr, "dhcpcd %s", ifName);
	RTK_SYSTEM(cmdstr);
	sleep(3);
	return 0;
}

/*
input:	@fd		: socket
	@buf		: received control request.
output:	@ctrl_cmd		: AP's ssid
	@accept_control	: <"on"/"off"> , AP's security
description:	parse the received control request and send corresponding response, 
	return 1 if receive a SC_SUCCESS_ACK, otherwise return 0. 
*/
static int ctrlmsg_handler(int fd, char *buf, int *ctrl_cmd, int *accept_control)
{
	int status = rtk_sc_valid_ctrlframe(buf);
	int flag;
	char dev_name[64];

	flag = rtk_sc_get_ctrlmsg_value(buf, CTRLMSG_DATAID_FLAG);
	DEBUG_PRINT("sockfd_ctrl!!  flag=%s \n", flag_str[flag]);
	switch(flag){
	case SC_SAVE_PROFILE:
		if(! (*accept_control))
			break;
		*ctrl_cmd = SC_SAVE_PROFILE;
		*accept_control = 0;
		rtw_sc_send_response_msg(fd, buf, SC_RSP_SAVE, status);
		break;
	case SC_DEL_PROFILE:
		if(! (*accept_control))
			break;
		*accept_control = 0;
		*ctrl_cmd  = SC_DEL_PROFILE;
//		reinit = SC_REINIT_WLAN;
		rtw_sc_send_response_msg(fd, buf, SC_RSP_DEL, status);
		break;
	case SC_RENAME:
		if(! (*accept_control))
			break;
		*accept_control = 0;
		*ctrl_cmd = SC_RENAME;
		rtk_sc_get_ctrlmsg_string(buf, CTRLMSG_DATAID_DEVICE_NAME, dev_name);
		rtk_sc_set_string_value(SC_DEVICE_NAME, dev_name);
		rtw_sc_send_response_msg(fd, buf, SC_RSP_RENAME, status);
		break;
	case SC_SUCCESS_ACK:
		DEBUG_PRINT("SC_SUCCESS_ACK! \n");
		return 1;
		break;
	default:
		DEBUG_PRINT("invalid request\n");
		rtw_sc_send_response_msg(fd, buf, SC_RSP_INVALID, 0);
		break;
	}
	return 0;
}

static void do_ctrl_cmd(int ctrl_cmd)
{
	char cmdstr[200];
	
	switch(ctrl_cmd) {
	case SC_SAVE_PROFILE:
		collect_scanres();
		break;
	case SC_DEL_PROFILE:
//		if(reinit == SC_REINIT_SYSTEM)
//			RTK_SYSTEM("reboot");
//		else if(reinit == SC_REINIT_WLAN) {
		/*      discard reinit system, always reinit wlan	*/
		sprintf(cmdstr, "rm -rf %s&", g_wpafile_path);
		RTK_SYSTEM(cmdstr);		
		RTK_SYSTEM("killall wpa_supplicant");
		RTK_SYSTEM("killall ping");
		//sprintf(cmdstr, "rmmod %s", WIFI_MODULE_NAME);
		//RTK_SYSTEM(cmdstr);
		exit(EXIT_SUCCESS);
		break;
	case SC_RENAME:
		break;
	}
}
//=====================		Custimize part ending	====================================





char *survey_info_path()
{
	static char path[200];

	memset(path, 0, 200);
	sprintf(path, "/proc/net/%s/%s/survey_info", wifi_proc_path, ifName);
	return path;
}

/*
input:	@before		: string before unicode procedure.

description:	process unicode(ex chinese word)  SSID of AP,
*/

void wifi_do_scan(void)
{
	char cmdstr[250], fwstats[200], readline[MAX_LINE_LEN];
	FILE *fd = NULL;
	char *ch = NULL;
	int fwstatus = 0, i;
	
	sprintf(cmdstr,"echo 1 > /proc/net/%s/%s/survey_info\n",
			wifi_proc_path, ifName);
	RTK_SYSTEM(cmdstr);

	//  poll /proc/../fwstatus and wait for scan done. wait 10sec at most.
	sprintf(fwstats,"/proc/net/%s/%s/fwstate", wifi_proc_path, ifName);
	for (i = 0; i< 10; i++) {
		sleep(1);
		fd = fopen(fwstats, "r");
		if( fd == NULL ) {
			printf("file[%s] open can not create file !!! \n", fwstats);
			return;
		}
		memset(readline, 0, sizeof(char)*MAX_LINE_LEN );
		fgets(readline, MAX_LINE_LEN , fd);
		fclose(fd);	
		if (strlen(readline) == 0)
			continue;
		if ((ch = strstr(readline, "=")) != NULL)
			fwstatus = (int)strtol(ch+1, NULL, 0);
		if ((fwstatus & _FW_UNDER_SURVEY) == 0)
			break;
	}
}

void wifi_disable(void)
{
	//char cmdstr[250];
#ifdef ANDROID_ENV
	RTK_SYSTEM("svc wifi disable");
	RTK_SYSTEM("sleep 5");
#else
	RTK_SYSTEM("killall wpa_supplicant");
	//sprintf(cmdstr, "rmmod %s", WIFI_MODULE_NAME);
	//RTK_SYSTEM(cmdstr);
#endif
}

void wifi_enable(void)
{
	char cmdstr[250];
	
#ifdef 	CONFIG_IOCTL_CFG80211
	sprintf(cmdstr, "modprobe cfg80211");
	RTK_SYSTEM(cmdstr);
#endif
	//sprintf(cmdstr, "insmod %s", WIFI_MODULE_NAME);
	//RTK_SYSTEM(cmdstr);
	sleep(1);

	//interface UP and enter monitor mode
	sprintf(cmdstr,"ifconfig %s up\n",ifName);
	RTK_SYSTEM(cmdstr);
	sleep(2);
}
/*
   if SSID contain Chinese word, convert all string into Hex format.
*/
char *ssid_rework(const char *before)
{
        unsigned int i;
        char *ch1;
        int flag_hex = FALSE;
        static char after[32 * 4 + 1];

        memset(after, 0, 32 * 4 + 1);
        if (before == NULL)
                return after;
        // process unicode SSID, without '"'
        for (i = 0; i < strlen(before); i++) {
                if (before[i] < 32 || before[i] > 127){
                        flag_hex = TRUE;
                        break;
                }
        }
        ch1 = &after[0];
        if (flag_hex) {
                i = 0;
                for (;;) {
                        if (before[i] == 0)
                                break;
                        sprintf(ch1,"%02x", (unsigned char)before[i]);
                        ch1 += 2;
                        i++;
                }
        }else { // ASCII SSID, need '"'
                after[0] ='"';
                strcpy(after+1, before);
                after[strlen(before) + 1] = '"';
        }
        return after;
}

/*
input:	@filepath		: path of wpa_supplicant config file which need to be stored.
	@ssid		: AP's ssid
	@cfgfile_type	: the security type of AP
	
description:	store the result as  wpa_supplicant config file  in given file path,
*/
static int store_cfgfile(const char *filepath, char *ssid, int cfgfile_type)
{
	FILE *fd;
	char commset[]={"update_config=1\nctrl_interface=/data/%s\neapol_version=1\nfast_reauth=1\n"};	
	char WPAstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tscan_ssid=1\n\tpsk=\"%s\"\n}\n"};
	char OPENstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n"};
	char WEPstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\twep_key0=%s\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n"};
	char CmdStr[2048], passwd[65];

	memset(passwd, 0, 65);
	fd=fopen(filepath, "w+");
	if( fd == NULL ) {
		printf("file open can not create file !!! \n");
		return -ENOENT;
	}
	sprintf(CmdStr, commset, ifName);
	fprintf(fd,"%s", CmdStr);
	switch (cfgfile_type) {
	case CFGFILE_OPEN :
		sprintf(CmdStr, OPENstr, ssid_rework(ssid));
		break;
	case CFGFILE_WPA :
		rtk_sc_get_string_value(SC_PASSWORD, passwd);
		sprintf(CmdStr, WPAstr, ssid_rework(ssid), passwd);
		break;
	case CFGFILE_WEP :
		rtk_sc_get_string_value(SC_PASSWORD, passwd);
		sprintf(CmdStr, WEPstr, ssid_rework(ssid), passwd);
		break;
	default :
		return -EINVAL;
		break;
	}
	DEBUG_PRINT("%s\n",CmdStr);
	fprintf(fd,"%s", CmdStr);
	fclose(fd);
	return 0;
}

void printf_encode(char *txt, size_t maxlen, const u8 *data, size_t len)
{
	char *end = txt + maxlen;
	size_t i;

	for (i = 0; i < len; i++) {
		if (txt + 4 > end)
			break;

		switch (data[i]) {
		case '\"':
			*txt++ = '\\';
			*txt++ = '\"';
			break;
		case '\\':
			*txt++ = '\\';
			*txt++ = '\\';
			break;
		case '\e':
			*txt++ = '\\';
			*txt++ = 'e';
			break;
		case '\n':
			*txt++ = '\\';
			*txt++ = 'n';
			break;
		case '\r':
			*txt++ = '\\';
			*txt++ = 'r';
			break;
		case '\t':
			*txt++ = '\\';
			*txt++ = 't';
			break;
		default:
			if (data[i] >= 32 && data[i] <= 127) {
				*txt++ = data[i];
			} else {
				txt += snprintf(txt, end - txt, "\\x%02x",
						   data[i]);
			}
			break;
		}
	}

	*txt = '\0';
}
const char* wpa_ssid_txt(const u8 *ssid, size_t ssid_len)
{
	static char ssid_txt[32 * 4 + 1];

	if (ssid == NULL) {
		ssid_txt[0] = '\0';
		return ssid_txt;
	}

	printf_encode(ssid_txt, sizeof(ssid_txt), ssid, ssid_len);
	return ssid_txt;
}
/*
input:	@scan_res	: path of scan_result file
	@target_bssid	: bssid of target AP
output:	@ssid		: AP's ssid, do nothing if strlen() is not zero.
	@flag		: indicate property of WPA, WPA2, WPS, IBSS, ESS, P2P.
	@channel	: AP's channel.
	
description:	parse scan_result to get the information of corresponding AP by BSSID.
*/
static int parse_scanres(char *scan_res, char *target_bssid, char *channel, 
			 char *ssid, u16 *flag)
{
	char readline[MAX_LINE_LEN];
	char *ch, *idx, *bssid, *flag_s;
	FILE* fp = NULL;
	int found = -EAGAIN;

	fp = fopen(scan_res, "r");
	if (!fp) {
	    printf("%s:Unable to open file [%s] \n", __func__, scan_res);
	    return -ENOENT;
	}
	if (strlen(target_bssid) < 6) {
		printf("%s:Error !! invalid BSSID [%s] \n", __func__, target_bssid);
		return -EINVAL;
	}
	while (! feof(fp)) {
		*flag = 0;
		*channel = 0;
		if (!fgets(readline, MAX_LINE_LEN , fp))
			break;
		idx = strtok(readline, " \r\n");
		if (!idx)
			continue;
		bssid = strtok(NULL, " \r\n");
		if (!bssid || (strlen(bssid) < 17) 
			|| (strcmp(bssid, target_bssid) != 0))
			continue;
		// Channel
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		strcpy(channel, ch);
		// RSSI
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// SdBm
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// Noise
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// age
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// flag
		flag_s = strtok(NULL, " \r\n");
		if (!flag_s)
			continue;		
		if ((ch = strstr(flag_s, PATTERN_WPA)) != NULL)
			*flag |= FLAG_WPA;
		if ((ch = strstr(flag_s, PATTERN_WPA2)) != NULL)
			*flag |= FLAG_WPA2;
		if ((ch = strstr(flag_s, PATTERN_WEP)) != NULL)
			*flag |= FLAG_WEP;
		if ((ch = strstr(flag_s, PATTERN_WPS)) != NULL)
			*flag |= FLAG_WPS;
		if ((ch = strstr(flag_s, PATTERN_IBSS)) != NULL)
			*flag |= FLAG_IBSS;
		if ((ch = strstr(flag_s, PATTERN_ESS)) != NULL)
			*flag |= FLAG_ESS;
		if ((ch = strstr(flag_s, PATTERN_P2P)) != NULL)
			*flag |= FLAG_P2P;
		// SSID
		if (strlen(ssid) == 0) {
			ch = strtok(NULL, "\t\r\n");
			if (ch) {
				int idx;
				for (idx=0; idx<strlen(ch); idx++) {
					if ((ch[idx]==' ') || (ch[idx]=='\t'))
						continue;
					else {
						ch += idx;
						break;
					}
				}
				strcpy(ssid, ch);
			}else {
				printf("Warming!! No SSID, please consult"
					" with the vendor!!\n");
				return FALSE;
			}
		}else
			ch = ssid;
		found = TRUE;
		DEBUG_PRINT("bssid = [%s], ssid = [%s], :channel=[%s] flag=%04X \n", 
					bssid, wpa_ssid_txt((const u8*)ch,strlen(ch)), channel, *flag);
		break;
	}
	fclose(fp);
	return found;
}

int collect_scanres(void)
{
#ifdef ANDROID_ENV
	char cmdstr[250];
#endif
	char bssid_str[64], bssid[ETH_ALEN], ch[5], ssid[64];
	int ret = -100, i;
	u16 flag = 0;
	
	memset(ssid, 0, 64);
	memset(bssid, 0, ETH_ALEN);
	memset(bssid_str, 0, 64);
	memset(ch, 0, 5);
	
	rtk_sc_get_string_value(SC_BSSID, bssid);
	rtk_sc_get_string_value(SC_SSID, ssid);
	sprintf(bssid_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
			(u8)bssid[0], (u8)bssid[1], (u8)bssid[2],
			(u8)bssid[3], (u8)bssid[4], (u8)bssid[5]);
	printf("%s() target_bssid=[%s], ssid=[%s] \n",__func__, bssid_str, ssid);
	for (i = 0; i < MAX_SCAN_TIMES; i++) {
		wifi_do_scan();
		if ((parse_scanres(survey_info_path(), bssid_str, ch, ssid, &flag))!=TRUE)
			continue;
		if ((flag & FLAG_WPA) || (flag & FLAG_WPA2))
			ret = store_cfgfile(g_wpafile_path, ssid, CFGFILE_WPA);
		else if (flag & FLAG_WEP)
			ret = store_cfgfile(g_wpafile_path, ssid, CFGFILE_WEP);
		else
			ret = store_cfgfile(g_wpafile_path, ssid, CFGFILE_OPEN);
		break;
	}
#ifdef ANDROID_ENV
	sprintf(cmdstr, "cat %s > /data/misc/wifi/wpa_supplicant.conf", g_wpafile_path);
	RTK_SYSTEM(cmdstr);
	RTK_SYSTEM("cat /data/misc/wifi/wpa_supplicant.conf");
	RTK_SYSTEM("chmod 777 /data/misc/wifi/wpa_supplicant.conf");
#endif
	if (ret == -100)
		printf("Error!! bssid:%s not found \n", bssid_str);
	return ret;
}

#if defined(SIMPLE_CONFIG_PBC_SUPPORT)
//  monitor HW button (ex: gpio pin) to capture pbc event.
void* pbc_monitor()
{
	while (1) {
	//	if (hw SC_PBC event be triggered) {
	//		kill(getpid(), SIG_SCPBC);
	//		printf("pbc_monitor  trigger \n");	
	//	}
		sleep(1);
	}
	return NULL;
}
#endif

void init_chplan(int chnum)	
{
	int i;
	
	for (i=0; i<chnum; i++) {
		if (g_chPlan[i].bw_os == '+')
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_LOWER;
		else if (g_chPlan[i].bw_os == '-')
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_UPPER;
		else
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	}

	if(g_sc_debug == 2) {
		for (i=0; i<chnum; i++)
			printf("probe_chplan[%d]:\tch=%d,\t bw_offset=%d \n",
				i, g_chPlan[i].ch, g_chPlan[i].bw_os);
	}
}

void wifi_monitor_mode_onoff(u8 onoff, const char *ifName)
{
	char cmdstr[200];

	memset(cmdstr, 0, sizeof(char)*200);
	//iwconfig wlanX mode monitor
	//iw dev wlanX set type monitor
#ifdef	CONFIG_IOCTL_CFG80211
	if (onoff)
		sprintf(cmdstr,"iw dev %s set type monitor\n", ifName);
	else
		sprintf(cmdstr,"iw dev %s set type managed\n", ifName);
#else
	if (onoff)
		sprintf(cmdstr,"iwconfig %s mode monitor\n", ifName);
	else
		sprintf(cmdstr,"iwconfig %s mode managed\n", ifName);
#endif
	RTK_SYSTEM(cmdstr);
}

static int get_rtheader_len(u8 *buf, size_t len)
{
	struct ieee80211_radiotap_header *rt_header;
	u16 rt_header_size;

	rt_header = (struct ieee80211_radiotap_header *)buf;
	/* check the radiotap header can actually be present */
	if (len < sizeof(struct ieee80211_radiotap_header))
		return -EILSEQ;
	 /* Linux only supports version 0 radiotap format */
	 if (rt_header->it_version)
		return -EILSEQ;
	rt_header_size = le16toh(rt_header->it_len);
	 /* sanity check for allowed length and radiotap length field */
	if (len < rt_header_size)
		return -EILSEQ;
	return rt_header_size;
}


static int cal_cckrate(u8 *buf, u8 offset)
{
	return buf[offset]/2;
}


float rate_array[][2][2] = {
					{{6.5, 7.2},	{13.5, 15}},
					{{13,	14.4},	{27,	30}},
					{{19.5,	21.7},	{40.5,	45}},
					{{26,	28.9},	{54,	60}},
					{{39,	43.3},	{81,	90}},
					{{52,	57.8},	{108,	120}},
					{{58.5,	65},	{121.5,	135}},
					{{65,	72.2},	{135,	150}},
					{{13,	14.4},	{27,	30}},
					{{26,	28.9},	{54,	60}},
					{{39,	43.3},	{81,	90}},
					{{52,	57.8},	{108,	120}},
					{{78,	86.7},	{162,	180}},
					{{104,	115.6},	{216,	240}},
					{{117,	130},	{243,	270}},
					{{130,	144.4},	{270,	300}},
					{{19.5,	21.7},	{40.5,	45}},
					{{39,	43.3},	{81,	90}},
					{{58.5,	65},	{121.5,	135}},
					{{78,	86.7},	{162,	180}},
					{{117,	130},	{243,	270}},
					{{156,	173.3},	{324,	360}},
					{{175.5, 195},	{364.5,	405}},
					{{195,	216.7},	{405,	450}},
					{{26,	28.8},	{54,	60}},
					{{52,	57.6},	{108,	120}},
					{{78,	86.8},	{162,	180}},
					{{104,	115.6},	{216,	240}},
					{{156,	173.2},	{324,	360}},
					{{208,	231.2},	{432,	480}},
					{{234,	260},	{486,	540}},	
					{{260,	288.8},	{540,	600}}};
                    
               


static int cal_mcsrate(u8 *buf, u8 offset)
{	
	u8 known = buf[offset];
	u8 flag = buf[offset+1];	
	u8 bw = 0;	
	u8 gi = 0;	
	u8 mcs = 0;	
	
	if (known & 0x01)
		bw = flag & 0x03;
	if (known & 0x02)
		mcs = buf[offset+2];	
	if (known & 0x04)
		gi = flag & 0x04;

//	printf("known=%x  flag=%x , mcs=%x \n", known, flag, mcs);
	return  rate_array[mcs][bw][gi];
}

static int cal_vhtrate(u8 *buf, size_t len)
{
	return 600;
}


                    

#define IEEE80211_RADIOTAP_FLAGS					0x00000002
#define IEEE80211_RADIOTAP_RATE						0x00000004
#define IEEE80211_RADIOTAP_CHANNEL					0x00000008
#define IEEE80211_RADIOTAP_DBM_ANTSIGNAL			0x00000020
#define IEEE80211_RADIOTAP_ANTENNA					0x00000800
#define IEEE80211_RADIOTAP_RXFLAGS					0x00004000
#define IEEE80211_RADIOTAP_MCS_INFO					0x00080000

#define IEEE80211_RADIOTAP_VHT_INFO					0x00200000




static int get_rtheader_rate(u8 *buf)
{
/*
	PS : Skip packet length checking, because get_rtheader_len() has do it before.
*/

	struct ieee80211_radiotap_header *rt_header;
	u8 rt_header_rate_pos = 0;
	
	rt_header = (struct ieee80211_radiotap_header *)buf;
	rt_header->it_present = le32toh(rt_header->it_present);
	
	rt_header_rate_pos = 8;
	// printf( "==%08X== %d  \n" , rt_header->it_present  , rt_header->it_present & (IEEE80211_RADIOTAP_FLAGS)   );
	
	if (rt_header->it_present & (IEEE80211_RADIOTAP_FLAGS))
		rt_header_rate_pos += 1;

	if (rt_header->it_present & (IEEE80211_RADIOTAP_RATE)) {
		return cal_cckrate(buf, rt_header_rate_pos);
	}

	rt_header_rate_pos += 1;		// force append 1 Byte.
	if (rt_header->it_present & (IEEE80211_RADIOTAP_CHANNEL))
		rt_header_rate_pos += 2;

	rt_header_rate_pos += 2;		// Channel_flag
	if (rt_header->it_present & (IEEE80211_RADIOTAP_DBM_ANTSIGNAL))
		rt_header_rate_pos += 1;
	if (rt_header->it_present & (IEEE80211_RADIOTAP_ANTENNA))
		rt_header_rate_pos += 1;
	if (rt_header->it_present & (IEEE80211_RADIOTAP_RXFLAGS))
		rt_header_rate_pos += 2;
	if (rt_header->it_present & (IEEE80211_RADIOTAP_MCS_INFO)) {
		return cal_mcsrate(buf, rt_header_rate_pos);
	}
	if (rt_header->it_present & (IEEE80211_RADIOTAP_VHT_INFO)) {
		return cal_vhtrate(buf, rt_header_rate_pos);
	}
	return 1;
}


void doRecvfrom(void)
{
	int NumTotalPkts;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	u8 buf[BUF_SIZ];
	int rt_header_len = 0;

	//Create a raw socket that shall sniff
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		printf("listener: socket"); 
		return;
	}
	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);

	//memcpy(g_sc_ctx.dmac, ifopts.ifr_hwaddr.sa_data, ETH_ALEN);


	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		printf("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		printf("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	
	NumTotalPkts=0;
	while(1) {
		numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
		//printf("listener: got packet %d bytes , total packet Num :%d \n", numbytes,NumTotalPkts);
		if (numbytes < 0) {
			printf("Recvfrom error , failed to get packets\n");
			goto thr_exit;
		}
		if( SC_STATUS_DONE == rtk_sc_get_value(SC_STATUS) )
			goto thr_exit;
		if( SC_STATUS_STOP == rtk_sc_get_value(SC_STATUS) )
			goto thr_exit;
		// Get length of Radiotap header 
		if ((rt_header_len = get_rtheader_len(buf, (size_t)numbytes)) < 1)
			continue;

		//Now process the packet
#ifdef CONFIG_NOFCS
		ProcessPacket(buf, numbytes+4, rt_header_len);
#else
		ProcessPacket(buf, numbytes, rt_header_len);
#endif
		NumTotalPkts++;
	}
thr_exit:
	close(sockfd);
	return ;
}

static void sig_handler(int sig)
{
	DEBUG_PRINT("%s: catch signal -- %d \n", __func__, sig);
	switch(sig) {
	case SIGINT:
		g_abort = TRUE;
		RTK_SYSTEM("killall wpa_supplicant");
		RTK_SYSTEM("killall ping");
		wifi_monitor_mode_onoff(FALSE, ifName);
		exit(EXIT_FAILURE);
		break;
	case SIG_SCPBC:
		g_reinit = TRUE;
		printf("sig_handler()  SIG_SCPBC  \n");
		break;
	default:
		break;
	}
}

static void print_usage(void)
{
	printf("\n\nUsage: \n\trtw_simple_config \t-i<ifname> -c<wpa_cfgfile> -n<dev_name> -p <pincode> -t <timeout>\n\t\t\t\t-m <phone_mac> [-dD] [-v]");
	printf("\n\nOPTIONS:\n\t");
	printf("-i = interface name\n\t");
	printf("-c = the path of wpa_supplicant config file\n\t");
	printf("-p = pincode\n\t");
	printf("-d = enable debug message, -D = more message.\n\t");
	printf("-n = device name\n\t");
	printf("-t = timeout (seconds)\n\t");
	printf("-m = filtering MAC, only accept the configuration frame which's from this MAC address\n\t");
	printf("-v = version\t\n\n");
	printf("example:\n\t");
	printf("rtw_simple_config -i wlan0 -c ./wpa_conf -D\n\t");
	printf("rtw_simple_config -i wlan0 -c ./wpa_conf -p 14825903 -t 120 -n RTKSC_SAMPLE -d\n\n");
	return;
}

int parse_argv(int argc, char **argv)
{
	int opt;

	/*	initial variable by default valve	*/
	rtk_sc_set_string_value(SC_DEVICE_NAME, DEFAULT_DEVICE_NAME);
	memset(ifName, 0, IFNAMSIZ);
	strcpy(ifName, DEFAULT_IF);
	rtk_sc_set_string_value(SC_PIN, DEFAULT_PIN);
	rtk_sc_set_string_value(SC_DEFAULT_PIN, DEFAULT_PIN);
	rtk_sc_set_value(SC_DURATION_TIME, -1);
	strcpy(g_wpafile_path, DEFAULT_WPAFILE_PATH);
	rtk_sc_set_macaddr_filter("00:00:00:00:00:00");
	g_sc_debug = 0;

	while ((opt = getopt(argc, argv, "i:c:n:t:p:P:m:hdDv")) != -1) {
		switch (opt) {
		case 'n':	// device name
			rtk_sc_set_string_value(SC_DEVICE_NAME, optarg);
			break;
		case 'i':	// wlan interface
			strcpy(ifName, optarg);
			break;
		case 'c':	// wpa_supplicant config file path
			strcpy(g_wpafile_path, optarg);
			break;
		case 'p':	// pincode
			rtk_sc_set_string_value(SC_PIN, optarg);
			break;
		case 'm':	// mac address filter
			if (rtk_sc_set_macaddr_filter(optarg)) {
				printf("Invalid MAC address -- %s\n",optarg);
				return -EINVAL;
			}
			break;
		case 'd':	// enable debug message
			g_sc_debug = 1;
			break;
		case 'D':	// enable move debug message
			g_sc_debug = 2;
			break;
		case 't':	// timeout
			if (atoi(optarg) > 0)
				rtk_sc_set_value(SC_DURATION_TIME, atoi(optarg));
			break;
		case 'v':
			printf("%s -- %s\n", argv[0], PROGRAM_VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
		default: /* '?' */
			print_usage();
			return -EINVAL;
		}
	}

	if(g_sc_debug) {
		char dbg_str[256], dbg_int = 0;

		DEBUG_PRINT("========option parse========\n");
		rtk_sc_get_string_value(SC_PIN, dbg_str);
		DEBUG_PRINT("pincode = %s\n", dbg_str);
		rtk_sc_get_string_value(SC_DEVICE_NAME, dbg_str);
		DEBUG_PRINT("device name = %s\n",  dbg_str);
		DEBUG_PRINT("ifName = %s\n",  ifName);
		dbg_int = rtk_sc_get_value(SC_DURATION_TIME);
		if (dbg_int > 0)
			DEBUG_PRINT("timeout = %d\n", dbg_int);
		DEBUG_PRINT("g_wpafile_path = %s\n",  g_wpafile_path);
		DEBUG_PRINT("========================\n");
	}
	return 0;
}
/*
	description : check 
	return : ip address obtained. 0
			 otherwise, error code. 
*/
int check_ip_timeout(const int timeout)
{
	int sockfd_ack, ret = 0; 
	unsigned int self_ipaddr=0;
	char smac[6];
	int i;

	if ((sockfd_ack = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return errno;
	}
	
	for (i=0; i<timeout; i++) {
		ret = rtk_sc_get_addr(sockfd_ack, smac, &self_ipaddr);
		if (self_ipaddr != 0) {
			close(sockfd_ack);
			return 0;
		}
		DEBUG_PRINT("Getting IP address(%02ds) ... \n", i);
		sleep(1);
	}
	close(sockfd_ack);
	return ret;
}

int control_handler(void)
{
	int accept_control = 1, on = 1, flag, stop_comfirm = 0;
	int sockfd_scan, sockfd_ctrl;			// socket descriptors
	struct sockaddr_in device_addr;     		// my address information
	struct sockaddr sender; 			// connector’s address information
	socklen_t sendsize = sizeof(sender);
	int sc_run_time, ctrl_cmd, sc_config_success = 0;
	ssize_t numbytes;
	fd_set fds;
	int max_fd, selret;
	char buf[256];
	struct timeval timeout;

	if ((sockfd_scan = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("create scan socket fail\n");
		exit(EXIT_FAILURE);
	}

	setsockopt( sockfd_scan, SOL_SOCKET, SO_BROADCAST|SO_REUSEADDR, &on, sizeof(on) );

	bzero(&device_addr,sizeof(struct sockaddr_in)); 
	device_addr.sin_family = AF_INET;         		// host byte order
	device_addr.sin_port = htons(SCAN_PORT);     		// short, network byte order
	device_addr.sin_addr.s_addr = INADDR_ANY;		// automatically fill with my IP

	// bind the socket with the address
	if (bind(sockfd_scan, (struct sockaddr *)&device_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind scan socket fail\n");
		close(sockfd_scan);
		exit(EXIT_FAILURE);
	}

	if ((sockfd_ctrl = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("create control socket fail\n");
		exit(EXIT_FAILURE);
	}
	setsockopt( sockfd_scan, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	bzero(&device_addr,sizeof(struct sockaddr_in)); 
	device_addr.sin_family = AF_INET;         		// host byte order
	device_addr.sin_port = htons(ACK_DEST_PORT);     	// short, network byte order
	device_addr.sin_addr.s_addr = INADDR_ANY;		// automatically fill with my IP

	// bind the socket with the address
	if (bind(sockfd_ctrl, (struct sockaddr *)&device_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind control socket fail\n");
		close(sockfd_scan);
		exit(EXIT_FAILURE);
	}

	sc_run_time=0;
	ctrl_cmd = SC_SCAN;
	while (1) {
		sc_run_time++;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(sockfd_scan, &fds);
		FD_SET(sockfd_ctrl, &fds);
		max_fd = (sockfd_ctrl > sockfd_scan) ? sockfd_ctrl : sockfd_scan;
		selret = select(max_fd+1, &fds, NULL, NULL, &timeout);
#if defined(SIMPLE_CONFIG_PBC_SUPPORT)
		if (g_reinit)
			return SC_REINIT_WLAN;
#endif
		if (!stop_comfirm && rtk_sc_send_finishack())
			continue;

		if (selret && FD_ISSET(sockfd_scan, &fds)) {
			stop_comfirm = 1;
			memset(buf, 0, 256);
			if ((numbytes = recvfrom(sockfd_scan, buf, 256, 0,
						   &sender, &sendsize)) == -1) {
				fprintf(stderr,"Receive failed!!!\n");
				close(sockfd_scan);
				exit(EXIT_FAILURE);
			}
			flag = rtk_sc_get_scanmsg_value(buf, SCANMSG_DATAID_FLAG);
			DEBUG_PRINT("sockfd_scan !! pMsg->flag=%s\n", flag_str[flag]);
			switch(flag) {
			case SC_SUCCESS_ACK:
				DEBUG_PRINT("receive config success ack\n");
				ctrl_cmd = SC_SCAN;
				break;
			case SC_SCAN:
				rtw_sc_send_scan_ack(sockfd_scan, buf, SC_RSP_SCAN);
				break;
			default:
				printf("invalid request\n");
				break;
			}
		}
		
		/* Note: APK would continue sends several ctrl_message for one click on GUI, so we 
		    don't do action in receiving each message, we just keep it in "ctrl_cmd" then do_ctrl_cmd() 
		    after ctrl_message sending finish (receive a SC_SUCCESS_ACK message)			*/
		
		if (selret && FD_ISSET(sockfd_ctrl, &fds)) {
			stop_comfirm = 1;
			memset(buf, 0, 256);
			if ((numbytes = recvfrom(sockfd_ctrl, buf, 256, 0,
						   &sender, &sendsize)) == -1) {
				fprintf(stderr,"Receive failed!!!\n");
				close(sockfd_ctrl);
				exit(EXIT_FAILURE);
			}
			if(g_sc_debug == 2) {
				BYTEDUMP("received ctrl_msg: ", buf, (u32)numbytes);
				printf("the length is %d\n", 
					rtk_sc_get_ctrlmsg_value(buf, CTRLMSG_DATAID_LENGTH));
			}
			sc_config_success = ctrlmsg_handler(sockfd_ctrl, buf, &ctrl_cmd, &accept_control);
		}
		else {
			if(sc_config_success == 1) {
				do_ctrl_cmd(ctrl_cmd);
				ctrl_cmd = SC_SCAN;
				sc_config_success = 0;
			}
			accept_control = 1;
		}
	}
}

#define WIFI_CHIP_TYPE_PATH     "/sys/class/rkwifi/chip"
static int check_wifi_chip_type_string(char *type)
{
    int wififd, ret = 0;
    char buf[64];

    wififd = open(WIFI_CHIP_TYPE_PATH, O_RDONLY);
    if (wififd < 0 ) {
        printf("Can't open %s, errno = %d", WIFI_CHIP_TYPE_PATH, errno);
        ret = -1;
        goto fail_exit;
    }
    memset(buf, 0, 64);

    if (0 == read(wififd, buf, 32)) {
        printf("read %s failed", WIFI_CHIP_TYPE_PATH);
        close(wififd);
        ret = -1;
        goto fail_exit;
    }
    close(wififd);

    strcpy(type, buf);
    printf("%s: %s", __FUNCTION__, type);

fail_exit:
    return ret;
}


int main(int argc, char *argv[])
{
	int err, chidx = 0;
	char cmdstr[256];
	pthread_t tid[2];
	int chnum = sizeof(g_chPlan)/sizeof(g_chPlan[0]);
	char wifi_type[64];

	check_wifi_chip_type_string(wifi_type);
	if (!(strcmp("RTL8189FS", wifi_type)))
		strcpy(wifi_proc_path, "rtl8189fs");
	if (!(strcmp("RTL8723DS", wifi_type)))
		strcpy(wifi_proc_path, "rtl8723ds");

	//   Environment  init
	rtk_sc_init();
	if ( parse_argv(argc, argv))
		return -EINVAL;
	memset(cmdstr,'\0',sizeof(cmdstr));
	signal(SIGINT, sig_handler);
	sprintf(cmdstr, "rm -rf %s&", g_wpafile_path);
	RTK_SYSTEM(cmdstr);
#if defined(SIMPLE_CONFIG_PBC_SUPPORT)
	signal(SIG_SCPBC, sig_handler);
	err = pthread_create(&(tid[1]), NULL, &pbc_monitor, NULL);
#endif

sc_reinit:
	wifi_disable();
	wifi_enable();
	wifi_do_scan();
	init_chplan(chnum);
	wifi_monitor_mode_onoff(TRUE, ifName);
 	if (!g_reinit) {
		err = pthread_create(&(tid[0]), NULL,(void *)&doRecvfrom, NULL);
		printf("after pthread_create...\n");
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
		else
			printf("\n doRecvfrom Thread created successfully\n");
	}
	g_reinit = FALSE;
	rtk_restart_simple_config(__LINE__);

	while (1) {
#if defined(SIMPLE_CONFIG_PBC_SUPPORT)
		if (g_reinit)
			goto sc_reinit;
#endif

		// switch channel & bandwidth
		sprintf(cmdstr,"echo %d %d 1 > /proc/net/%s/%s/monitor \n", 
			g_chPlan[chidx].ch, g_chPlan[chidx].bw_os, wifi_proc_path, ifName);
		RTK_SYSTEM(cmdstr);
		usleep(TIME_CHSWITCH);
		if ((SC_STATUS_DECODE == rtk_sc_get_value(SC_STATUS))
			|| rtk_sc_get_fix_sa()) {
			char bssid_str[64], bssid[ETH_ALEN], ssid[64], ch[5];
			int i, sec = 100 + rtk_sc_get_profile_pkt_index() * 2;
			u16 flag = 0;

find_channel:
			memset(ssid, 0, 64);
			memset(bssid, 0, ETH_ALEN);
			memset(bssid_str, 0, 64);
			memset(ch, 0, 5);
			rtk_sc_get_string_value(SC_BSSID, bssid);
			sprintf(bssid_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
					(u8)bssid[0], (u8)bssid[1], (u8)bssid[2],
					(u8)bssid[3], (u8)bssid[4], (u8)bssid[5]);
			if ((parse_scanres(survey_info_path(), bssid_str, ch, ssid, &flag)==TRUE)
				&& (strlen(ch) != 0)) {

				for (chidx = 0; chidx < chnum; chidx++)
					if (atoi(ch) == g_chPlan[chidx].ch)
						break;
				if (chidx == chnum) {
					printf("\n Unable to find corresponding channel in channel plan, ch=[%s] \n", ch);
					goto find_channel;
				}
				sprintf(cmdstr,"echo %d %d 1 > /proc/net/%s/%s/monitor \n", 
					g_chPlan[chidx].ch, g_chPlan[chidx].bw_os, wifi_proc_path, ifName);
				RTK_SYSTEM(cmdstr);
				DEBUG_PRINT("decode phase!! staying [ch:%s] for"
						" maximum [%d] seconds (parse_scanres==TRUE)\n", ch, sec);
			} else {
				printf("\nUnable to parse scan_result -- [ch:%s], do scan again.... \n", ch);
				wifi_monitor_mode_onoff(FALSE, ifName);
				wifi_do_scan();
				wifi_monitor_mode_onoff(TRUE, ifName);
				goto find_channel;
			}
	
			for (i = 0; i < sec; i++) {
				sleep(1);
				if (SC_STATUS_DONE == rtk_sc_get_value(SC_STATUS))
					break;
			}
		}
		if (SC_STATUS_DONE == rtk_sc_get_value(SC_STATUS) ||
			SC_STATUS_STOP == rtk_sc_get_value(SC_STATUS))
			break;
		rtk_restart_simple_config(__LINE__);
		if ( ++chidx == chnum)
			chidx = 0;
	}

	if (SC_STATUS_DONE == rtk_sc_get_value(SC_STATUS)) {
		wifi_monitor_mode_onoff(FALSE, ifName);		
		if (!collect_scanres()) {	
			connect_ap();
#ifndef ANDROID_ENV
			request_ip();
#endif
			if (check_ip_timeout(TIME_CHECK_IP)) {
				printf("WiFi link ready, but unable to get IP address!! program exit!\n");
				exit(EXIT_FAILURE);
			}
			control_handler();
		}
	}
	return 0;
}
 
static void ProcessPacket(u8 *buffer, int size, int rt_header_len)
{
	struct rx_frinfo frinfo;
	u8 type;
	u8 subtype;

	/*	80211 header format
		ver:	2bit
		type:	2bit
		subtype:	4bit
		tods:	1bit
		frds:	1bit
		other:	6bit		*/
		
		
	frinfo.pframe = buffer + rt_header_len;
	type = *(frinfo.pframe) & TYPE_MASK;
	subtype = (*(frinfo.pframe) & SUBTYPE_MASK) >> 4;

	frinfo.pktlen = size - rt_header_len;
	if ((type != TYPE_DATA) || (frinfo.pktlen < 50))
		return ;
	frinfo.to_fr_ds = *(frinfo.pframe + 1) & FRTODS_MASK;
	if (frinfo.to_fr_ds == 1) {
		frinfo.sa = GetAddr2Ptr(frinfo.pframe);
		frinfo.da = GetAddr3Ptr(frinfo.pframe);
	} else if (frinfo.to_fr_ds == 2) {
		frinfo.sa = GetAddr3Ptr(frinfo.pframe);
		frinfo.da = GetAddr1Ptr(frinfo.pframe);
	}

	if ((!frinfo.da) || (!frinfo.sa))
		return ;
	/*	SimpleConfigV1 -- Multicast packets	*/
	if (frinfo.da[0] == DEST_MAC0 &&
		frinfo.da[1] == DEST_MAC1 &&
		frinfo.da[2] == DEST_MAC2
	   )
	{
//		if(CHECKSUM_OK(frinfo.da, ETH_ALEN))
//		{
//			rtk_sc_start_parse_packet(&frinfo);
//		}
  	}
	else if (memcmp(frinfo.da, bc_mac, ETH_ALEN)==0) 	/* SimpleConfigV2 V3-- Broadcast packets	*/
	{
		frinfo.rx_rate = get_rtheader_rate(buffer);
		frinfo.seq = GetSequence(frinfo.pframe);
		if (!(subtype & 0x08))		// QOS data.
			frinfo.pktlen += 2;
		 
		rtk_sc_start_parse_bcpacket(&frinfo);
	}
}


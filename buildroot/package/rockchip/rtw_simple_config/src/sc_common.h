#include <stdio.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/if.h>


/********************		Customize Part	*******************/
#define PROGRAM_VERSION			"v3.1.0"
#define WIFI_MODULE_NAME		"8189fs.ko"
#define PROC_MODULE_PATH		"rtl8189fs"               /*need to modify to adpter diffrent wifi module, rtl8723bs,rtl8188fu,rtl8723ds*/
#define TIME_CHSWITCH			200000			/* 0.2 seconds of staying in each channel */
#define TIME_CHECK_IP			30			/* seconds of waiting ip address obtained */





// Bandwidth Offset
#define HAL_PRIME_CHNL_OFFSET_DONT_CARE	0
#define HAL_PRIME_CHNL_OFFSET_LOWER	1
#define HAL_PRIME_CHNL_OFFSET_UPPER	2


#define DEFAULT_IF				"wlan0"
#define DEFAULT_WPAFILE_PATH	"/data/wpa_conf"
#define DEFAULT_DEVICE_NAME		"RTK_SC_DEV"
#define DEFAULT_PIN				"57289961"

#define	PATTERN_WEP		"[WEP]"
#define	PATTERN_WPA		"[WPA]"
#define	PATTERN_WPA2	"[WPA2]"
#define	PATTERN_WPS		"[WPS]"
#define	PATTERN_IBSS	"[IBSS]"
#define	PATTERN_ESS		"[ESS]"
#define	PATTERN_P2P		"[P2P]"
#define FLAG_WPA		0x0001
#define FLAG_WPA2		0x0002
#define FLAG_WEP		0x0004
#define FLAG_WPS		0x0008
#define FLAG_IBSS		0x0010
#define FLAG_ESS		0x0020
#define FLAG_P2P		0x0040















/********************		APK related		*******************/
#define ACK_DEST_PORT			8864
#define SCAN_PORT				18864

#define SC_SCAN					0x00
#define SC_SAVE_PROFILE			0x01
#define SC_DEL_PROFILE			0x02
#define SC_RENAME				0x03
#define SC_SUCCESS_ACK			0x04
// #define SC_PRIVCMD01			0x05
// #define SC_PRIVCMD02			0x06

/******************************************************************************/




typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEBUG_PRINT(fmt, args...) 	do{				\
						if(g_sc_debug) 		\
						printf(""fmt,## args); 	\
					} while(0)

#define DEBUG_PRINT2(fmt, args...) \
				if(g_sc_debug == 2) \
				printf(""fmt,## args); 

#define MACCPY(src, dst)	do{ 	\
					src[0]=dst[0]; src[1]=dst[1]; \
					src[2]=dst[2]; src[3]=dst[3]; \
					src[4]=dst[4]; src[5]=dst[5]; \
				} while(0)

#define watchdog_kick()		{}

#define GetAddr1Ptr(pbuf)       ((unsigned char *)(pbuf) + 4)
#define GetAddr2Ptr(pbuf)       ((unsigned char *)(pbuf) + 10)
#define GetAddr3Ptr(pbuf)       ((unsigned char *)(pbuf) + 16)
#define GetAddr4Ptr(pbuf)       ((unsigned char *)(pbuf) + 24)
#define GetSequence(pbuf)		(le16toh(*(unsigned short *)((unsigned char*)(pbuf) + 22)) >> 4)

struct ieee80211_radiotap_header {
        u8        it_version;     /* set to 0 */
        u8        it_pad;
        u16       it_len;         /* entire length */
        u32       it_present;     /* fields present */
} __attribute__((__packed__));
#define FRTODS_MASK		0x03
#define TYPE_MASK		0x0C
#define SUBTYPE_MASK		0xF0
#define TYPE_DATA		0x08
#define GetToDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_TO_DS_)) != 0)
#define GetFrDs(pbuf)		(((*(unsigned short *)(pbuf)) & (_FROM_DS_)) != 0)

#define SC_ENABLED			1
#define SC_DURATION_TIME		2
#define SC_GET_SYNC_TIME		3
#define SC_GET_PROFILE_TIME		4
#define SC_VXD_RESCAN_TIME		5
#define SC_PIN_ENABLED			6
#define SC_STATUS			7
#define SC_DEBUG			8
#define SC_CHECK_LINK_TIME		9
#define SC_SYNC_VXD_TO_ROOT 		10
#define SC_ACK_ROUND			11
#define SC_CONTROL_IP			12
//#define SC_PRIV_STATUS			13
#define SC_CONFIG_TIME			14
				
#define SC_PIN				20
#define SC_DEFAULT_PIN			21
#define SC_PASSWORD 			22
#define SC_DEVICE_NAME			23
#define SC_DEVICE_TYPE			24
#define SC_SSID 			25
#define SC_BSSID			26

#define SC_FROM_TO_DS			30


#define SC_STATUS_STOP			-1
#define SC_STATUS_INIT			0
#define SC_STATUS_SYNC			1
#define SC_STATUS_DECODE		2
#define SC_STATUS_DONE			3

#define SC_SUCCESS_IP			0x10
#define SC_DHCP_GETTING_IP		0x00
#define SC_DHCP_STATIC_IP		0x01
#define SC_DHCP_GOT_IP			0x02

#define SC_RSP_ACK			0x20
#define SC_RSP_SCAN			0x21
#define SC_RSP_SAVE			0x22
#define SC_RSP_DEL			0x23
#define SC_RSP_RENAME			0x24
#define SC_RSP_INVALID			0x3f

#define SCANMSG_DATAID_FLAG			1
#define SCANMSG_DATAID_LENGTH			2
#define SCANMSG_DATAID_SEC_LEVEL		3
#define SCANMSG_DATAID_NONCE			4
#define SCANMSG_DATAID_DIGEST			5
#define SCANMSG_DATAID_SMAC			6
#define SCANMSG_DATAID_DEVICE_TYPE		7

#define CTRLMSG_DATAID_FLAG			1
#define CTRLMSG_DATAID_LENGTH			2
#define CTRLMSG_DATAID_SEC_LEVEL		3
#define CTRLMSG_DATAID_NONCE			4
#define CTRLMSG_DATAID_DIGEST1			5
#define CTRLMSG_DATAID_DIGEST2			6
#define CTRLMSG_DATAID_DEVICE_NAME		7

#define SC_REINIT_SYSTEM			1
#define SC_REINIT_WLAN				2

struct rx_frinfo {
	//unsigned char		*pskb;
	unsigned int		pktlen;
	unsigned char		*da;
	unsigned char		*sa;
	unsigned char		*pframe;
	unsigned char		to_fr_ds;
	unsigned short		seq;
	unsigned short		tid;
	unsigned char		rx_rate;
	unsigned char		rx_bw;
};
int g_sc_debug;
int sockfd;

#define MAX_CHNUM		50
struct _chplan {
	u8 	ch;
	u8 	bw_os;
};

#define RS_DEBUG		0

#define BYTEDUMP(desc, buf, len)			\
			do {				\
				u32 i;			\
				unsigned char *p;	\
				p = (unsigned char*)(buf);		\
				printf("%s ", (desc));			\
				for(i = 0; i < (len); i++)		\
					printf("%02x", p[i]);		\
				printf("\n");				\
			} while(0)

int CHECKSUM_OK(unsigned char *data, int len);
unsigned short checksum(unsigned short* buffer, int size);
int bytecopy(unsigned char *src, unsigned char *dst, unsigned int len);
int rtk_sc_init();
int rtk_sc_get_value(unsigned int id);
int rtk_sc_get_string_value(unsigned int id, void *value);
int rtk_sc_set_value(unsigned int id, unsigned int value);
int rtk_sc_set_string_value(unsigned int id, char *value);
/*	Get the value of specific field(id) from scan message	*/
int rtk_sc_get_scanmsg_value(const char *buf, unsigned int id);
/*	Get the string value of specific field(id) from scan message	*/
int rtk_sc_get_scanmsg_string(const char *buf, unsigned int id, void *value);
/*	Get the value of specific field(id) from control message	*/
int rtk_sc_get_ctrlmsg_value(const char *buf, unsigned char id);
/*	Get the string value of specific field(id) from control message	*/
int rtk_sc_get_ctrlmsg_string(const char *buf, unsigned char id, void *value);
int rtk_sc_start_parse_packet(struct rx_frinfo *pfrinfo);
int rtk_sc_start_parse_bcpacket(struct rx_frinfo *pfrinfo);
int rtk_sc_get_fix_sa();
int rtk_sc_get_profile_pkt_index();
int rtk_sc_stop_simple_config();
#define CFGFILE_OPEN			0
#define CFGFILE_WPA				1
#define CFGFILE_WEP				2
int rtk_sc_set_macaddr_filter(char *macaddr);
int rtk_sc_get_addr(int sockfd, char *mac_addr, u32 *self_ipaddr);
void rtk_restart_simple_config(int line);
unsigned int rtk_sc_get_phoneip();
int rtk_sc_valid_ctrlframe(const char *buf);

int rtk_sc_send_finishack();
int rtw_sc_send_response_msg(int sockfd, const char *buf, unsigned char flag, unsigned char status);
int rtw_sc_send_scan_ack(int sockfd_scan, const char *buf, char flag);




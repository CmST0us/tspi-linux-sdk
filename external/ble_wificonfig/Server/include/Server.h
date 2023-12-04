#ifndef __SOFTAP_SERVER_H__
#define __SOFTAP_SERVER_H__

#include <stdio.h>

#define LOG_TAG "Server"

/* socket port */
#define SOCKET_PORT 8443
/* connection number in one time */
#define TCP_QUEUE_LINE 1
#define RECV_BUFFER_SIZE 2000
#define MSG_BUFF_LEN 8888

/* request type string from http. */
#define REQUEST_WIFI_LIST           "/provision/wifiListInfo"
#define REQUEST_WIFI_SET_UP  	    "/provision/wifiSetup"
#define REQUEST_COMPANION_INFO      "/provision/companionInfo"
#define REQUEST_DEVICE_CONTEXT      "/provision/deviceContext"



#define LOG_DEBUG_LEVEL (1)
#define LOG_ERROR_FLAG (4)
#define LOG_WARING_FLAG (3)
#define LOG_INFO_FLAG (2)
#define LOG_DEBUG_FLAG (1)

#define LOG_PRINTF(level, format, ...) \
	do { \
		if (level > LOG_DEBUG_LEVEL) { \
			printf("[%s]: " format, LOG_TAG, ##__VA_ARGS__); \
		} \
	} while(0)
#define log_info(format, ...) LOG_PRINTF(LOG_INFO_FLAG, format, ##__VA_ARGS__)
#define log_dbg(format, ...) LOG_PRINTF(LOG_DEBUG_FLAG, format, ##__VA_ARGS__)
#define log_warn(format, ...) LOG_PRINTF(LOG_WARING_FLAG, format, ##__VA_ARGS__)
#define log_err(format, ...) LOG_PRINTF(LOG_ERROR_FLAG, format, ##__VA_ARGS__)


#endif // __SOFTAP_SERVER_H__

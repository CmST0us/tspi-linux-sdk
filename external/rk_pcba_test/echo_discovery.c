

/**
 * Copyright (C) 2017 Fuzhou Rockchip Electronics Co., Ltd
 * author: WuQiang <xianlee.wu@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "fcntl.h"
#include "sys/file.h"
#include "sys/ioctl.h"
#include "time.h"
#include "malloc.h"
#include "unistd.h"
#include "net/if.h"
#include "arpa/inet.h"
#include "sys/socket.h"

// vendor parameter
#define VENDOR_REQ_TAG 0x56524551
#define VENDOR_READ_IO _IOW('v', 0x01, unsigned int)
#define VENDOR_WRITE_IO _IOW('v', 0x02, unsigned int)

#define VENDOR_SN_ID 1
#define VENDOR_WIFI_MAC_ID 2
#define VENDOR_LAN_MAC_ID 3
#define VENDOR_BLUETOOTH_ID 4
#define VENDOR_STREAM_ID 14
#define VENDOR_PROTOCOL_ID  15

#define UDPPORT 18888
#define FWK_MSG_UID_LEN 20
#define FWK_MSG_CMD_LEN 128
#define FWK_MSG_IP_MAX_LEN 16
#define FWK_MSG_MAC_MAX_LEN 18

/* Storage parameter */
#define VENDOR_PARAMETER_ID 5
/* Change the id when flash firmware */
#define VENDOR_FW_UPDATE_ID 6

#define VENDOR_DATA_SIZE (3 * 1024)  // 3k
#define VENDOR_DATA_PROTOCOL_SIZE (384)  /* 64*6=384 byte */

#define VERDOR_DEVICE "/dev/vendor_storage"

typedef struct _RK_VERDOR_REQ {
  uint32_t tag;
  uint16_t id;
  uint16_t len;
  uint8_t data[VENDOR_DATA_SIZE];
} RK_VERDOR_REQ;

typedef struct _RK_VERDOR_PROTOCOL_REQ {
  uint32_t tag;
  uint16_t id;
  uint16_t len;
  uint8_t data[VENDOR_DATA_PROTOCOL_SIZE];
} RK_VERDOR_PROTOCOL_REQ;

static int rk_parameter_read_uid(int sys_fd, char *iotc_uid, int len);
static int parameter_read_protocol_uid(char iotc_uid[FWK_MSG_UID_LEN+1]);
static void broadcast_service();

int main()
{
    printf("starting device discovery service!\n");

    broadcast_service();

    printf("discovery service crashed!\n");

    return 0;
}

int rk_parameter_read_uid(int sys_fd, char *iotc_uid, int len)
{
    RK_VERDOR_PROTOCOL_REQ req;

    if (sys_fd < 0) {
        printf("_read_uid: error with sys_fd < 0\n");
        return -1;
    }
    req.tag = VENDOR_REQ_TAG;
    req.id = VENDOR_SN_ID;
    req.len = 512;
    if (ioctl(sys_fd, VENDOR_READ_IO, &req)) {
        printf("_read_uid:VENDOR_SN_ID fail\n");
        return -1;
    }
    /* rknand_print_hex_data("vendor read:", (uint32_t*)&req, req.len + 8); **/
    if (req.len > len) {
        printf("_read_uid:%d force to %d\n", req.len, len);
        req.len = len;
    }
    memcpy(iotc_uid, req.data, req.len);

    return req.len;
}

int parameter_read_protocol_uid(char iotc_uid[FWK_MSG_UID_LEN+1])
{
    int sys_fd = -1;
    int ret = -1;

    printf("read_uid: enter...\n");
    sys_fd = open(VERDOR_DEVICE, O_RDWR, 0);
    if (sys_fd < 0) {
        printf("read_uid: error with sys_fd < 0\n");
        return ret;
    }
    memset(iotc_uid, 0, FWK_MSG_UID_LEN + 1);
    ret = rk_parameter_read_uid(sys_fd, &iotc_uid[0], FWK_MSG_UID_LEN);
    close(sys_fd);
    printf("read_uid: success with uid=%s\n", iotc_uid);

    return ret;
}


static void broadcast_service()
{
    socklen_t addr_len = 0;
    struct sockaddr_in server_addr;
    struct ifreq ifr;
    char buf[64];
    char local_addr[FWK_MSG_IP_MAX_LEN];
    char local_mac[FWK_MSG_MAC_MAX_LEN];
    char cmd_info[FWK_MSG_CMD_LEN];
    char uid[FWK_MSG_UID_LEN + 1];
    int broadcast_fd = -1;
    int opt = -1;

    broadcast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcast_fd < 0) {
        perror("broadcast_thread : socket");
        goto stop_broadcast;
    }
    printf("broadcast_thread socketfd = %d\n", broadcast_fd);

    strcpy(ifr.ifr_name, "rndis0");

    memset(local_addr, 0, FWK_MSG_IP_MAX_LEN);
    memset(local_mac, 0, FWK_MSG_MAC_MAX_LEN);

    // get rndis ethernet interface ip address
    while (ioctl(broadcast_fd, SIOCGIFADDR, &ifr) < 0) {
        sleep(1);
    }
    snprintf(local_addr, FWK_MSG_IP_MAX_LEN, "%s",
            inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));

    // get rndis ethernet interface mac address
    while (ioctl(broadcast_fd, SIOCGIFHWADDR, &ifr) < 0) {
        sleep(1);
    }
    snprintf(local_mac, FWK_MSG_MAC_MAX_LEN, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    printf("Discovery:got device net info!\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UDPPORT);
    addr_len = sizeof(server_addr);

    if (setsockopt(broadcast_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt,
               sizeof(opt)) < 0) {
        perror("broadcast setsockopt SO_BROADCAST");
        goto stop_broadcast;
    }
    if (setsockopt(broadcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
               sizeof(opt)) < 0) {
        perror("broadcast setsockopt SO_REUSEADDR");
        goto stop_broadcast;
    }

    if (bind(broadcast_fd, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("broadcast bind");
        goto stop_broadcast;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        memset(uid, 0, sizeof(uid));
        memset(cmd_info, 0, sizeof(cmd_info));


        fprintf(stderr,"=========function is: %s line is : %d=============\n",__func__,__LINE__);
        if (recvfrom(broadcast_fd, buf, 64, 0,
            (struct sockaddr *)&server_addr, &addr_len) < 0) {
            perror("broadcast recvfrom");
            continue;
        }

        printf("broadcast: from: %s port: %d > %s\n",
               inet_ntoa(server_addr.sin_addr),
               ntohs(server_addr.sin_port), buf);

        if (strcmp("CMD_DISCOVER", buf) == 0) {

            if ((parameter_read_protocol_uid(uid)) < 0) {
                printf("Discover:read uid failed!\n");
            }

            uid[FWK_MSG_UID_LEN] = '\0';

            snprintf(cmd_info, FWK_MSG_CMD_LEN,
                    "{\"UID\":\"%s\","
                    "\"IP\":\"%s\","
                    "\"MAC\":\"%s\","
                    "\"DEVICENAME\":\"%s\"}",
                    uid,
                    local_addr,
                    local_mac,
                    "ECHO");

            printf("sendto: %s port: %d > %s\n",
                   inet_ntoa(server_addr.sin_addr),
                   ntohs(server_addr.sin_port), cmd_info);
            if (sendto(broadcast_fd, cmd_info, strlen(cmd_info),
                0, (struct sockaddr *)&server_addr, addr_len) < 0) {
                perror("broadcast_thread recvfrom");
                goto stop_broadcast;
            }
        }
    }

stop_broadcast:
    printf("stop broadcast !\n");
    if (broadcast_fd >= 0) {
        shutdown(broadcast_fd, 2);
        close(broadcast_fd);
        broadcast_fd = -1;
    }
}

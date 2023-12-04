/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Instituto Nokia de Tecnologia - INdT
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <pthread.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus/gdbus.h"

#include "src/error.h"
#include "wifi_util_wrapper.h"


#define GATT_MGR_IFACE			"org.bluez.GattManager1"
#define GATT_SERVICE_IFACE		"org.bluez.GattService1"
#define GATT_CHR_IFACE			"org.bluez.GattCharacteristic1"
#define GATT_DESCRIPTOR_IFACE		"org.bluez.GattDescriptor1"

/* Immediate wifi Service UUID */
#define WIFI_SERVICES_UUID       "1B7E8251-2877-41C3-B46E-CF057C562023"
#define SECURITY_CHAR_UUID       "CAC2ABA4-EDBB-4C4A-BBAF-0A84A5CD93A1"
#define HIDE_CHAR_UUID           "CAC2ABA4-EDBB-4C4A-BBAF-0A84A5CD26C7"
#define SSID_CHAR_UUID           "ACA0EF7C-EEAA-48AD-9508-19A6CEF6B356"
#define PASSWORD_CHAR_UUID       "40B7DE33-93E4-4C8B-A876-D833B415A6CE"
#define CHECKDATA_CHAR_UUID      "40B7DE33-93E4-4C8B-A876-D833B415C759"
#define NOTIFY_CHAR_UUID         "8AC32D3f-5CB9-4D44-BEC2-EE689169F626"
#define NOTIFY_DESC_UUID         "00002902-0000-1000-8000-00805f9b34fb"
#define WIFILIST_CHAR_UUID       "8AC32D3f-5CB9-4D44-BEC2-EE689169F627"
#define DEVICECONTEXT_CHAR_UUID  "8AC32D3f-5CB9-4D44-BEC2-EE689169F628"

#define BT_NAME                  "Yami"
static char reconnect_path[66];

#ifdef DUEROS
#define DUEROS_WIFI_SERVICES_UUID       "00001111-0000-1000-8000-00805f9b34fb"
#define DUEROS_CHARACTERISTIC_UUID      "00002222-0000-1000-8000-00805f9b34fb"

#define DUEROS_SOCKET_RECV_LEN 20

static pthread_t dueros_tid = 0;
static int dueros_socket_done = 0;
static int dueros_socket_fd = -1;
static char dueros_socket_path[] = "/data/bluez5_utils/socket_dueros";

struct characteristic *dueros_chr;

typedef struct{
    int server_sockfd;
    int client_sockfd;
    int server_len;
    int client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;
    char sock_path[512];
} tAPP_SOCKET;

#define CMD_ADV                  "hcitool -i hci0 cmd 0x08 0x0008 15 02 01 06 11 07 fb 34 9b 5f 80 00 00 80 00 10 00 00 11 11 00 00"
#else
#define SERVICES_UUID            "23 20 56 7c 05 cf 6e b4 c3 41 77 28 51 82 7e 1b"
#endif

#define CMD_PARA                 "hcitool -i hci0 cmd 0x08 0x0006 A0 00 A0 00 00 02 00 00 00 00 00 00 00 07 00"
#define CMD_EN                   "hcitool -i hci0 cmd 0x08 0x000a 1"

#define ADV_IRK		"\x69\x30\xde\xc3\x8f\x84\x74\x14" \

const char *WIFI_CONFIG_FORMAT = "ctrl_interface=/var/run/wpa_supplicant\nap_scan=1\nupdate_config=1\n\n";

const char *WIFI_CONFIG_FORMAT_PSK =  "network={\n    ssid=\"%s\"\n"
                                      "    key_mgmt=%s\n    psk=\"%s\"\n    scan_ssid=%s\n"
                                      "    priority=%d\n}\n\n";
const char *WIFI_CONFIG_FORMAT_NONE = "network={\n    ssid=\"%s\"\n"
                                      "    key_mgmt=%s\n    scan_ssid=%s\n"
                                      "    priority=%d\n}\n\n";
const char *WIFI_CONFIG_FORMAT_WEP =  "network={\n    ssid=\"%s\"\n"
                                      "    key_mgmt=%s\n    wep_key0=\"%s\"\n    scan_ssid=%s\n"
                                      "    priority=%d\n}\n\n";                                

static GMainLoop *main_loop;
static GSList *services;
static DBusConnection *connection;

static pthread_t wificonfig_tid = 0;

char wifi_ssid[256];
char wifi_ssid_bk[256];
char wifi_password[256];
char wifi_password_bk[256];
char wifi_security[256];
char wifi_hide[256];
char check_data[256];
int priority = 0;

struct characteristic *temp_chr;

struct characteristic {
	char *service;
	char *uuid;
	char *path;
	uint8_t *value;
	int vlen;
	const char **props;
};

struct descriptor {
	struct characteristic *chr;
	char *uuid;
	char *path;
	uint8_t *value;
	int vlen;
	const char **props;
};

/*
 * Supported properties are defined at doc/gatt-api.txt. See "Flags"
 * property of the GattCharacteristic1.
 */
static const char *ias_alert_level_props[] = { "read", "write", NULL };
static const char *chr_props[] = { "read", "write", "notify", NULL };
static const char *desc_props[] = { "read", "write", NULL };

static void chr_write(struct characteristic *chr, const uint8_t *value, int len);
static void chr_iface_destroy(gpointer user_data);

#ifdef DUEROS
/********************SERVER API***************************/
static int setup_socket_server(tAPP_SOCKET *app_socket)
{
    unlink (app_socket->sock_path);
    if ((app_socket->server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("fail to create socket\n");
        perror("socket");
        return -1;
    }
    app_socket->server_address.sun_family = AF_UNIX;
    strcpy (app_socket->server_address.sun_path, app_socket->sock_path);
    app_socket->server_len = sizeof (app_socket->server_address);
    app_socket->client_len = sizeof (app_socket->client_address);
    if ((bind (app_socket->server_sockfd, (struct sockaddr *)&app_socket->server_address, app_socket->server_len)) < 0) {
        perror("bind");
        return -1;

    }
    if (listen (app_socket->server_sockfd, 10) < 0) {
        perror("listen");
        return -1;
    }
    printf ("Server is ready for client connect...\n");

    return 0;
}

static int accpet_client(tAPP_SOCKET *app_socket)
{
    app_socket->client_sockfd = accept (app_socket->server_sockfd, (struct sockaddr *)&app_socket->server_address, (socklen_t *)&app_socket->client_len);
    if (app_socket->client_sockfd == -1) {
        perror ("accept");
        return -1;
    }
    return 0;
}

static void teardown_socket_server(tAPP_SOCKET *app_socket)
{
    unlink (app_socket->sock_path);
    app_socket->server_sockfd = 0;
    app_socket->client_sockfd = 0;
}

/********************CLIENT API***************************/
static int setup_socket_client(char *socket_path)
{
    struct sockaddr_un address;
    int sockfd,  len;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            printf("%s: can not creat socket\n", __func__);
            return -1;
    }

    address.sun_family = AF_UNIX;
    strcpy (address.sun_path, socket_path);
    len = sizeof (address);

    if (connect (sockfd, (struct sockaddr *)&address, len) == -1) {
        printf("%s: can not connect to socket\n", __func__);;
        return -1;
    }

    return sockfd;

}

static void teardown_socket_client(int sockfd)
{
    close(sockfd);
}

/********************COMMON API***************************/
static int socket_send(int sockfd, char *msg, int len)
{
    int bytes;
    if (sockfd < 0) {
        printf("%s: invalid sockfd\n",__func__);
        return -1;
    }
    if ((bytes = send(sockfd, msg, len, 0)) < 0) {
        perror ("send");
    }
    return bytes;
}

static int socket_recieve(int sockfd, char *msg, int len)
{
    int bytes;

    if (sockfd < 0) {
        printf("%s: invalid sockfd\n",__func__);
        return -1;
    }

    if ((bytes = recv(sockfd, msg, len, 0)) < 0)
    {
        perror ("recv");
    }
    return bytes;

}

static int dueros_socket_send(char *msg, int len) {
    return socket_send(dueros_socket_fd, msg, len);
}

static void *dueros_socket_recieve(void *arg) {
    int bytes = 0;
    char data[DUEROS_SOCKET_RECV_LEN];

    dueros_socket_fd = setup_socket_client(dueros_socket_path);
    if (dueros_socket_fd < 0) {
        printf("Fail to connect server socket\n");
        goto exit;
    }

    while (dueros_socket_done) {
        memset(data, 0, sizeof(data));
        bytes = socket_recieve(dueros_socket_fd, data, sizeof(data));
        if (bytes <= 0) {
            printf("Server leaved, break\n");
            break;
        }

        printf("dueros_socket_recieve, len: %d\n", bytes);
        for(int i = 0; i < bytes; i++)
            printf("%02x ", data[i]);
        printf("\n\n");

        //send to apk
        chr_write(dueros_chr, (uint8_t *) data, bytes);
        usleep(1000000); //sleep 1s
    }

exit:
    printf("Exit dueros socket thread\n");
    pthread_exit(0);
}

static int dueros_socket_thread_create(void) {
    dueros_socket_done = 1;
    if (pthread_create(&dueros_tid, NULL, dueros_socket_recieve, NULL)) {
        printf("Create dueros socket thread failed\n");
        return -1;
    }

    return 0;
}

static void dueros_socket_thread_delete(void) {
    dueros_socket_done = 0;

    teardown_socket_client(dueros_socket_fd);

    if (dueros_tid) {
        pthread_join(dueros_tid, NULL);
        dueros_tid = 0;
    }
}

#endif

static bool saveCheckdata(int flag, char * info)
{
    FILE *fp;
    int fd;
    
    if (flag == 0)
        fp = fopen("/data/cfg/check_data", "w");
    else if (flag == 1)
        fp = fopen("/data/cfg/bt_status", "w");
    else if (flag == 2)
        fp = fopen("/data/cfg/wifi_ssid", "w");


    if (fp == NULL)
    {
        return -1;
    }

    fputs(info, fp);
    fflush(fp);
    fd = fileno(fp);
    if (fd >= 0) {
        fsync(fd);
        printf("save check_data sucecees.\n");
    }
    fclose(fp);

    return 0;
}

static gboolean desc_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &desc->uuid);

	return TRUE;
}

static gboolean desc_get_characteristic(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH,
						&desc->chr->path);

	return TRUE;
}

static bool desc_read(struct descriptor *desc, DBusMessageIter *iter)
{
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &array);

	if (desc->vlen && desc->value)
		dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
						&desc->value, desc->vlen);

	dbus_message_iter_close_container(iter, &array);

	return true;
}

static gboolean desc_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct descriptor *desc = user_data;

	printf("Descriptor(%s): Get(\"Value\")\n", desc->uuid);

	return desc_read(desc, iter);
}

static void desc_write(struct descriptor *desc, const uint8_t *value, int len)
{
	g_free(desc->value);
	desc->value = g_memdup(value, len);
	desc->vlen = len;

	g_dbus_emit_property_changed(connection, desc->path,
					GATT_DESCRIPTOR_IFACE, "Value");
}

static int parse_value(DBusMessageIter *iter, const uint8_t **value, int *len)
{
	DBusMessageIter array;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &array);
	dbus_message_iter_get_fixed_array(&array, value, len);

	return 0;
}

static void desc_set_value(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct descriptor *desc = user_data;
	const uint8_t *value;
	int len;

	printf("Descriptor(%s): Set(\"Value\", ...)\n", desc->uuid);

	if (parse_value(iter, &value, &len)) {
		printf("Invalid value for Set('Value'...)\n");
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	desc_write(desc, value, len);

	g_dbus_pending_property_success(id);
}

static gboolean desc_get_props(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct descriptor *desc = data;
	DBusMessageIter array;
	int i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &array);

	for (i = 0; desc->props[i]; i++)
		dbus_message_iter_append_basic(&array,
					DBUS_TYPE_STRING, &desc->props[i]);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static const GDBusPropertyTable desc_properties[] = {
	{ "UUID",		"s", desc_get_uuid },
	{ "Characteristic",	"o", desc_get_characteristic },
	{ "Value",		"ay", desc_get_value, desc_set_value, NULL },
	{ "Flags",		"as", desc_get_props, NULL, NULL },
	{ }
};

static gboolean chr_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct characteristic *chr = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &chr->uuid);

	return TRUE;
}

static gboolean chr_get_service(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct characteristic *chr = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH,
							&chr->service);

	return TRUE;
}

static bool chr_read(struct characteristic *chr, DBusMessageIter *iter)
{
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &array);

	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
						&chr->value, chr->vlen);

	dbus_message_iter_close_container(iter, &array);

	return true;
}

static gboolean chr_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct characteristic *chr = user_data;

	printf("Characteristic(%s): Get(\"Value\")\n", chr->uuid);

	return chr_read(chr, iter);
}

static gboolean chr_get_props(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct characteristic *chr = data;
	DBusMessageIter array;
	int i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &array);

	for (i = 0; chr->props[i]; i++)
		dbus_message_iter_append_basic(&array,
					DBUS_TYPE_STRING, &chr->props[i]);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static void chr_write(struct characteristic *chr, const uint8_t *value, int len)
{
	g_free(chr->value);
	chr->value = g_memdup(value, len);
	chr->vlen = len;

	g_dbus_emit_property_changed(connection, chr->path, GATT_CHR_IFACE,
								"Value");
}

static void chr_set_value(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct characteristic *chr = user_data;
	const uint8_t *value;
	int len;

	printf("Characteristic(%s): Set('Value', ...)\n", chr->uuid);

	if (!parse_value(iter, &value, &len)) {
		printf("Invalid value for Set('Value'...)\n");
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	chr_write(chr, value, len);

	g_dbus_pending_property_success(id);
}

static const GDBusPropertyTable chr_properties[] = {
	{ "UUID",	"s", chr_get_uuid },
	{ "Service",	"o", chr_get_service },
	{ "Value",	"ay", chr_get_value, chr_set_value, NULL },
	{ "Flags",	"as", chr_get_props, NULL, NULL },
	{ }
};

static gboolean service_get_primary(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	dbus_bool_t primary = TRUE;

	printf("Get Primary: %s\n", primary ? "True" : "False");

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &primary);

	return TRUE;
}

static gboolean service_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	const char *uuid = user_data;

	printf("Get UUID: %s\n", uuid);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);

	return TRUE;
}

static gboolean service_get_includes(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	const char *uuid = user_data;
	char service_path[100] = {0,};
	DBusMessageIter array;
	char *p = NULL;

	snprintf(service_path, 100, "/service3");
	printf("Get Includes: %s\n", uuid);

	p = service_path;

	printf("Includes path: %s\n", p);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
			DBUS_TYPE_OBJECT_PATH_AS_STRING, &array);

	dbus_message_iter_append_basic(&array, DBUS_TYPE_OBJECT_PATH,
							&p);

	snprintf(service_path, 100, "/service2");
	p = service_path;
	printf("Get Includes: %s\n", p);

	dbus_message_iter_append_basic(&array, DBUS_TYPE_OBJECT_PATH,
							&p);
	dbus_message_iter_close_container(iter, &array);


	return TRUE;

}


static gboolean service_exist_includes(const GDBusPropertyTable *property,
							void *user_data)
{
	const char *uuid = user_data;

	printf("Exist Includes: %s\n", uuid);

#ifdef DUEROS
	if (strncmp(uuid, "00001111", 8) == 0)
		return TRUE;
#else
	if (strncmp(uuid, "1B7E8251", 8) == 0)
		return TRUE;
#endif

	return FALSE;
}

static const GDBusPropertyTable service_properties[] = {
	{ "Primary", "b", service_get_primary },
	{ "UUID", "s", service_get_uuid },
	{ "Includes", "ao", service_get_includes, NULL,
					service_exist_includes },
	{ }
};

static void chr_iface_destroy(gpointer user_data)
{
	struct characteristic *chr = user_data;

	g_free(chr->uuid);
	g_free(chr->service);
	g_free(chr->value);
	g_free(chr->path);
	g_free(chr);
}

static void desc_iface_destroy(gpointer user_data)
{
	struct descriptor *desc = user_data;

	g_free(desc->uuid);
	g_free(desc->value);
	g_free(desc->path);
	g_free(desc);
}

static int parse_options(DBusMessageIter *iter, const char **device)
{
	DBusMessageIter dict;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;
		int var;

		dbus_message_iter_recurse(&dict, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		var = dbus_message_iter_get_arg_type(&value);
		if (strcasecmp(key, "device") == 0) {
			if (var != DBUS_TYPE_OBJECT_PATH)
				return -EINVAL;
			dbus_message_iter_get_basic(&value, device);
			printf("Device: %s\n", *device);                        
		}

		dbus_message_iter_next(&dict);
	}

	return 0;
}

void check_wifiinfo(int flag, char * info) {
        char temp[1024];
        char buff[256] = {0};
        char cmdline[256] = {0};
        int j = 0;
        
        if (flag == 0) {
               for (int i = 0; i < strlen(info); i++) {
                        sprintf(temp+2*i, "%02x", info[i]);
               }
               temp[strlen(info)*2] = '\0';
               strcpy(info, temp);
               printf("check_wifiinfo ssid: %s\n", info);
        } else if (flag == 1){
                memset(cmdline, 0, sizeof(cmdline));
                sprintf(cmdline,"wpa_cli -iwlan0 scan_result | grep %s", wifi_ssid_bk);
                execute("wpa_cli -iwlan0 scan", buff);
                execute(cmdline, buff);
                if (strstr(buff, "WPA") != NULL) 
                        strcpy(wifi_security, "WPA-PSK");
                else if (strstr(buff, "ESS") != NULL && strstr(buff, "WPA") == NULL)
                        strcpy(wifi_security, "NONE");
                printf("check_wifiinfo security: %s\n", wifi_security);                       
        } else if (flag == 2) {
                for(int i = 0; i < strlen(info); i++) {
                        temp[j++] = '\\';
                        temp[j++] = info[i];
                }
                temp[j] = '\0';
                strcpy(info, temp);

                printf("check_wifiinfo password: %s\n", info);
        }                
}

void execute(const char cmdline[],char recv_buff[]){
    printf("consule_run: %s\n",cmdline);

    FILE *stream = NULL;
    char buff[1024];

    memset(recv_buff, 0, sizeof(recv_buff));

    if((stream = popen(cmdline,"r"))!=NULL){
        while(fgets(buff,1024,stream)){
            strcat(recv_buff,buff);
        }
    }

    pclose(stream);
}

//wpa_supplicant
static int wpa_supplicant_config_wifi() {
    FILE *fp = NULL;

    if ((fp = fopen("/data/cfg/wpa_supplicant.conf", "w+")) == NULL)
    {
        printf("open cfg wpa_supplicant.conf failed");
        return -1;
    }

    fprintf(fp, "%s\n", "ctrl_interface=/var/run/wpa_supplicant");
    fprintf(fp, "%s\n", "ap_scan=1");
    fprintf(fp, "%s\n", "network={");
    fprintf(fp, "%s%s%s\n", "ssid=\"", wifi_ssid, "\"");
    fprintf(fp, "%s%s%s\n", "psk=\"", wifi_password, "\"");
    fprintf(fp, "%s\n", "key_mgmt=WPA-PSK");
    fprintf(fp, "%s\n", "}");

    fclose(fp);

    if (-1 == system("killall wpa_supplicant; dhcpcd -k wlan0; killall dhcpcd;"
                   "ifconfig wlan0 0.0.0.0")) {
        printf("killall wpa_supplicant dhcpcd failed");
        return -1;
    }

    if (-1 == system("wpa_supplicant -Dnl80211 -i wlan0 "
                   "-c /data/cfg/wpa_supplicant.conf &")) {
        printf("start wpa_supplicant failed");
        return -1;
    }

    if (-1 == system("dhcpcd wlan0 -t 0 &")) {
        printf("dhcpcd failed");
        return -1;
    }

    return 0;
}

//wpa_cli
static gboolean config_wifi() {
    printf("start config_wifi\n");
    char buff[256] = {0};
    char cmdline[256] = {0};
    int id = -1;
    bool execute_result = false;

    execute("wpa_cli -iwlan0 disconnect",buff);
    usleep(500000);
    // 1. add network
    execute("wpa_cli -iwlan0 add_network",buff);
    id = atoi(buff);
    if(id < 0){
        perror("add_network failed.\n");
        return FALSE;
    }
    priority = id + 1;
    
    // 2. setNetWorkSSID
    if (wifi_ssid != NULL && strlen(wifi_ssid) != 0)
        check_wifiinfo(0, wifi_ssid);
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d ssid %s",id, wifi_ssid);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);   
    if(!execute_result){
        perror("setNetWorkSSID failed.\n");
        return FALSE;
    }
    
    // 3. setNetWorkSECURe
    check_wifiinfo(1, wifi_security);
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d key_mgmt %s", id,wifi_security);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);   
    if(!execute_result){
        perror("setNetWorkSECURe failed.\n");
        return FALSE;
    }
    
    // 4. setNetWorkPWD
    if (wifi_password != NULL && strlen(wifi_password) != 0)
            check_wifiinfo(2, wifi_password);
    if (strcmp(wifi_security, "NONE")) {
    	memset(cmdline, 0, sizeof(cmdline));
    	sprintf(cmdline,"wpa_cli -iwlan0 set_network %d psk \\\"%s\\\"", id,wifi_password);
    	printf("%s\n", cmdline);
    	execute(cmdline,buff);
    	execute_result = !strncmp(buff,"OK",2);
   	if(!execute_result){
            perror("setNetWorkPWD failed.\n");
            return FALSE;
    	}
    } else if (strcmp(wifi_security, "NONE") == 0) {
        printf("wifi_security is NONE! ignore the password\n");
    } else {
            if (wifi_password_bk != NULL && strlen(wifi_password_bk) != 0) {
                    memset(cmdline, 0, sizeof(cmdline));
                    if(strlen(wifi_password_bk) == 10 || strlen(wifi_password_bk) == 26)
                         sprintf(cmdline,"wpa_cli -iwlan0 set_network %d wep_key0 %s", id,wifi_password);
                    else
                         sprintf(cmdline,"wpa_cli -iwlan0 set_network %d wep_key0 \\\"%s\\\"", id,wifi_password);
                    printf("%s\n", cmdline);
                    execute(cmdline,buff);
                    execute_result = !strncmp(buff,"OK",2);   
                    if(!execute_result) {
                        perror("setNetWorkPWD failed.\n");
                        return FALSE;
                    }
            }
    }
	
    // 5. setNetWorkHIDE
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d scan_ssid %s", id,wifi_hide);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);    
    if(!execute_result){
        perror("setNetWorkHIDE failed.\n");
        return FALSE;
    }
    
    // 6. setNetWorkPriority
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d priority %d", id, id + 1);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);    
    if(!execute_result){
        perror("setNetWorkPriority failed.\n");
        return FALSE;
    }
    
    // 7. enableNetWork
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 enable_network %d", id);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);        
    if(!execute_result){
        perror("enableNetWork failed.\n");
        return FALSE;
    }

    //execute("wpa_cli save_config", buff);
    //execute("wpa_cli reconfig", buff);
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -i wlan0 select_network %d", id);
    memset(reconnect_path, 0, 66);
    strcpy(reconnect_path, cmdline);
    printf("repath: %s.\n", reconnect_path);
    printf("%s\n", cmdline);
    execute(cmdline,buff);
    execute_result = !strncmp(buff,"OK",2);        
    if(!execute_result){
        perror("select_network failed.\n");
        return FALSE;
    }

    return TRUE;
}

static bool save_wifi_config(int mode)
{
    char buff[256] = {0};
    char cmdline[256] = {0};
    
   //7. save config
    if (mode == 1) {
        execute("wpa_cli save_config", buff);
        execute("wpa_cli reconfigure", buff);
    } else { 
        execute("wpa_cli flush", buff);
	usleep(600000);
        execute("wpa_cli reconfigure", buff);
        execute("wpa_cli reconnect", buff);
    }
    return 0;
 /*
    if (mode == 1){
        FILE *fp;
        char body[256];
        int fd;
        int isFileExit = 0;
    
        if((access("/data/cfg/wpa_supplicant.conf",F_OK))!=-1) 
            isFileExit = 1;   
        else  
            isFileExit = 0;   

        fp = fopen("/data/cfg/wpa_supplicant.conf", "a+");

        if (fp == NULL)
            return -1;

        if (isFileExit == 0){
            snprintf(body, sizeof(body), WIFI_CONFIG_FORMAT);
            fputs(body, fp);
        }

        if (strcmp(wifi_security, "NONE"))
            snprintf(body, sizeof(body), WIFI_CONFIG_FORMAT_PSK, wifi_ssid_bk, wifi_security, wifi_password_bk, wifi_hide, priority);
        if (!strcmp(wifi_security, "NONE") && strlen(wifi_password_bk) > 0)
            snprintf(body, sizeof(body), WIFI_CONFIG_FORMAT_WEP, wifi_ssid_bk, wifi_security, wifi_password_bk, wifi_hide, priority);
        if (strlen(wifi_password_bk) == 0)
            snprintf(body, sizeof(body), WIFI_CONFIG_FORMAT_NONE, wifi_ssid_bk, wifi_security, wifi_hide, priority);    
        fputs(body, fp);
        fflush(fp);
        fd = fileno(fp);
        if (fd >= 0) {
            fsync(fd);
            printf("save wpa_supplicant.conf sucecees.\n");
        }
        fclose(fp);
        execute("wpa_cli reconfig", buff);
    } else {
         execute("wpa_cli flush", buff);   
    }

    return 0;
*/
}

static gboolean check_wifi_isconnected()
{
        printf("check_wifi_isconnected\n");
        char ret_buff[256] = {0};
        char temp_buff[256] = {0};
        gboolean isWifiConnected = FALSE;
	int retry = 1;
	char *str = NULL;
	char *str1 = NULL;

reconnect:
        for(int i = 0; i < 20; i++) {
	    printf("check wifi ...\n");
            usleep(500000);
            execute("wpa_cli -iwlan0 status | grep wpa_state", ret_buff);
            strncpy(temp_buff, ret_buff+10, strlen(ret_buff) - 11);
            printf("wpa_cli status %s\n", temp_buff);

            if (strncmp(temp_buff, "COMPLETED", 9) == 0) {
                usleep(500000);
                execute("wpa_cli -iwlan0 status | grep ip_address", ret_buff);
                if (strlen(ret_buff) > 0) {
                    strncpy(temp_buff, ret_buff+11, strlen(ret_buff) - 12);
                    printf("wpa_cli ip_address %s\n", temp_buff);
                
                    if ((strncmp(temp_buff, "127.0.0.1", 9) != 0) && (strncmp(temp_buff, "169", 3) != 0)) {
                    //if (strncmp(temp_buff, "127.0.0.1", 9) != 0) {
			memset(ret_buff, 0, 256);
			memset(temp_buff, 0, 256);
			execute("wpa_cli -iwlan0 status | grep ssid", ret_buff);
			if ((str = strstr(ret_buff, "bssid"))) {
			    str1 = strstr(str+5, "ssid");
			    strncpy(temp_buff, str1+5, strlen(wifi_ssid_bk));
			    printf("wpa_cli ssid %s %d\n", temp_buff, strlen(wifi_ssid_bk));
			    if (strncmp(temp_buff, wifi_ssid_bk, strlen(wifi_ssid_bk)) != 0)
				return FALSE;
			}
                        isWifiConnected = TRUE;
                        printf("wifi is connected.\n");
                        break;
                    }
                }
            }
            if (strncmp(temp_buff, "DISCONNECTED", 12) == 0)
                execute(reconnect_path, temp_buff);

        }

        if ((!isWifiConnected) && (retry-- > 0)) {
		execute(reconnect_path, ret_buff);
		goto reconnect;
	}

        if (!isWifiConnected) 
            printf("wifi is not connected.\n");            
    
        return isWifiConnected;
            
}
static void *config_wifi_thread(void)
{
        printf("config_wifi_thread\n");
        uint8_t level = 1;
        
	saveCheckdata(0, check_data);        
	if (!config_wifi()) {
	    printf("### config_wifi failed  sleep !!! ###\n");
            save_wifi_config(0);
            level = 2;
	    printf("### confs to notify level 2 ###\n");
            chr_write(temp_chr, &level, sizeof(level));    
	    //return;
            goto clean;
	}
        
        if (check_wifi_isconnected()) {
            save_wifi_config(1);
            level = 1;
	    printf("### cons to notify level 1 ###\n");
            chr_write(temp_chr, &level, sizeof(level));
            g_main_loop_quit(main_loop); 
        } else {
            save_wifi_config(0);
            level = 2;
	    printf("### conf to notify level 2 ###\n");
            chr_write(temp_chr, &level, sizeof(level));    
        }

clean:
	memset(wifi_ssid, 0, 256);
	memset(wifi_ssid_bk, 0, 256);
	memset(wifi_password, 0, 256);
	memset(wifi_password_bk, 0, 256);
	memset(wifi_security, 0, 256);
	memset(wifi_hide, 0, 256);
}
static int wifi_config_thread_create(void)
{

    if (pthread_create(&wificonfig_tid, NULL, config_wifi_thread, NULL)) {
        printf("Create wifi config thread failed\n");
        return -1;
    }

    return 0;
}
static int wifi_config_thread_cancel(void)
{

    if (pthread_cancel(&wificonfig_tid)) {
        printf("Cancle wifi config thread failed\n");
        return -1;
    }

    return 0;
}

#define BLE_SEND_MAX_LEN (134) //(20) //(512)
static DBusMessage *chr_read_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct characteristic *chr = user_data;
	DBusMessage *reply;
	DBusMessageIter iter;
	const char *device;
        static char *slist = NULL;
        static char *devicesn = NULL;
	char str[512];

	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return g_dbus_create_error(msg, DBUS_ERROR_NO_MEMORY,
							"No Memory");

	dbus_message_iter_init_append(reply, &iter);

        if (!strcmp(WIFILIST_CHAR_UUID, chr->uuid)) {
		if (!slist) {
                	slist = get_wifi_list();
			printf("%s: wifi total list is  %s, len=%d\n", __func__, slist, strlen(slist));
		}
                int len;
		char *tmp = slist;

                len = strlen(slist);

                chr_write(chr, slist, (len > BLE_SEND_MAX_LEN) ? BLE_SEND_MAX_LEN : len);

		if (len > BLE_SEND_MAX_LEN) {
			slist = strdup(slist + BLE_SEND_MAX_LEN);
			free(tmp);
		} else {
			free(slist);
			slist = NULL;
		}
        }
	if (!strcmp(DEVICECONTEXT_CHAR_UUID, chr->uuid)) {
                if (!devicesn) {
			devicesn = get_device_context();
			printf("%s device context: %s\n", __func__, devicesn);
                }
                int len;
                char *tmp = devicesn;

                len = strlen(devicesn);

                chr_write(chr, devicesn, (len > BLE_SEND_MAX_LEN) ? BLE_SEND_MAX_LEN : len);

                if (len > BLE_SEND_MAX_LEN) {
                        devicesn = strdup(devicesn + BLE_SEND_MAX_LEN);
                        free(tmp);
                } else {
                        free(devicesn);
                        devicesn = NULL;
                }
	}

	chr_read(chr, &iter);
	strncpy(str, chr->value, chr->vlen);
        str[chr->vlen] = '\0';
        printf("chr_read_value[%d]: %s\n", chr->vlen, str);

	return reply;
}

static DBusMessage *chr_write_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	printf("chr_write_value v0.3\n");
	struct characteristic *chr = user_data;
	DBusMessageIter iter;
	const uint8_t *value;
	int len;
	const char *device;
	char str[120];
	memset(str, 0, 120);

	dbus_message_iter_init(msg, &iter);

	if (parse_value(&iter, &value, &len))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	//chr_write(chr, value, len);

#ifdef DUEROS
    printf("chr_write_value, len: %d\n", len);
    for(int i = 0; i < len; i++)
        printf("%02x ", value[i]);
    printf("\n\n");
        
    if (!strcmp(DUEROS_CHARACTERISTIC_UUID, chr->uuid)) {
        dueros_socket_send((char *) value, len);
    }

    //chr_write(chr, "1111111111", 10);
#else
	if (((int)(chr->value) & 0x3) || chr->value == NULL) {
		printf("######chr_write chr->value: %p, value: %s, len: %d\n", chr->value, value, len);
		//return dbus_message_new_method_return(msg);
	}
	chr_write(chr, value, len);
	if(len == 0 || chr->value == NULL) {
		printf("chr_write_value is null\n");
                strcpy(wifi_password, "");
		return dbus_message_new_method_return(msg);
	}               
	//chr->value[len] = '\0';
	strncpy(str, chr->value, len);
	str[len] = '\0';
	printf("chr_write_value  %p, %d\n", chr->value, len);
        if (!strcmp(SSID_CHAR_UUID, chr->uuid)){                
                strcpy(wifi_ssid, str);
                strcpy(wifi_ssid_bk, str);
                saveCheckdata(2, wifi_ssid_bk);
                printf("wifi ssid is  %s, bk: %s\n", wifi_ssid, wifi_ssid_bk);
        }
        if (!strcmp(PASSWORD_CHAR_UUID, chr->uuid)){
                strcpy(wifi_password, str);
                strcpy(wifi_password_bk, str);
        	printf("wifi pwd is  %s\n", wifi_password);
	}
	if (!strcmp(HIDE_CHAR_UUID, chr->uuid)){
                strcpy(wifi_hide, str);
                printf("wifi hide is  %s\n", wifi_hide);                            
        }
		
	if (!strcmp(SECURITY_CHAR_UUID, chr->uuid)){
                strcpy(wifi_security, str);
                printf("wifi sec is  %s\n", wifi_security);                            
        }
		
	if (!strcmp(CHECKDATA_CHAR_UUID, chr->uuid)){
                strncpy(check_data, str, len);
                printf("check_data is  %s\n", check_data);
        	#if 0
        	wpa_cli_config_wifi();
        	save_wifi_config(wifi_ssid, wifi_password);
        	#else
        	wifi_config_thread_create(); 
        	#endif
    	}
#endif

	return dbus_message_new_method_return(msg);
}

static DBusMessage *chr_start_notify(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	return g_dbus_create_error(msg, DBUS_ERROR_NOT_SUPPORTED,
							"Not Supported");
}

static DBusMessage *chr_stop_notify(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	return g_dbus_create_error(msg, DBUS_ERROR_NOT_SUPPORTED,
							"Not Supported");
}

static const GDBusMethodTable chr_methods[] = {
	{ GDBUS_ASYNC_METHOD("ReadValue", GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					chr_read_value) },
	{ GDBUS_ASYNC_METHOD("WriteValue", GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL, chr_write_value) },
	{ GDBUS_ASYNC_METHOD("StartNotify", NULL, NULL, chr_start_notify) },
	{ GDBUS_METHOD("StopNotify", NULL, NULL, chr_stop_notify) },
	{ }
};

static DBusMessage *desc_read_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct descriptor *desc = user_data;
	DBusMessage *reply;
	DBusMessageIter iter;
	const char *device;

	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return g_dbus_create_error(msg, DBUS_ERROR_NO_MEMORY,
							"No Memory");

	dbus_message_iter_init_append(reply, &iter);

	desc_read(desc, &iter);

	return reply;
}

static DBusMessage *desc_write_value(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct descriptor *desc = user_data;
	DBusMessageIter iter;
	const char *device;
	const uint8_t *value;
	int len;

	if (!dbus_message_iter_init(msg, &iter))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_value(&iter, &value, &len))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	if (parse_options(&iter, &device))
		return g_dbus_create_error(msg, DBUS_ERROR_INVALID_ARGS,
							"Invalid arguments");

	desc_write(desc, value, len);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable desc_methods[] = {
	{ GDBUS_ASYNC_METHOD("ReadValue", GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					desc_read_value) },
	{ GDBUS_ASYNC_METHOD("WriteValue", GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL, desc_write_value) },
	{ }
};

static gboolean register_characteristic(const char *chr_uuid,
						const uint8_t *value, int vlen,
						const char **props,
						const char *desc_uuid,
						const char **desc_props,
						const char *service_path)
{
	struct characteristic *chr;
	struct descriptor *desc;
	static int id = 1;

	chr = g_new0(struct characteristic, 1);
	chr->uuid = g_strdup(chr_uuid);
	chr->value = g_memdup(value, vlen);
	chr->vlen = vlen;
	chr->props = props;
	chr->service = g_strdup(service_path);
	chr->path = g_strdup_printf("%s/characteristic%d", service_path, id++);

	if (!g_dbus_register_interface(connection, chr->path, GATT_CHR_IFACE,
					chr_methods, NULL, chr_properties,
					chr, chr_iface_destroy)) {
		printf("Couldn't register characteristic interface\n");
		chr_iface_destroy(chr);
		return FALSE;
	}
 
       if (strcmp(chr_uuid, NOTIFY_CHAR_UUID) == 0) {
           printf("save temp characteristic\n");
           temp_chr = chr;
       }

	if (!desc_uuid)
		return TRUE;

	desc = g_new0(struct descriptor, 1);
	desc->uuid = g_strdup(desc_uuid);
	desc->chr = chr;
	desc->props = desc_props;
	desc->path = g_strdup_printf("%s/descriptor%d", chr->path, id++);

	if (!g_dbus_register_interface(connection, desc->path,
					GATT_DESCRIPTOR_IFACE,
					desc_methods, NULL, desc_properties,
					desc, desc_iface_destroy)) {
		printf("Couldn't register descriptor interface\n");
		g_dbus_unregister_interface(connection, chr->path,
							GATT_CHR_IFACE);

		desc_iface_destroy(desc);
		return FALSE;
	}

#ifdef DUEROS
    if (strcmp(chr_uuid, DUEROS_CHARACTERISTIC_UUID) == 0) {
        printf("save dueros characteristic\n");
        dueros_chr = chr;
    }
#endif

	return TRUE;
}

static char *register_service(const char *uuid)
{
	static int id = 1;
	char *path;

	path = g_strdup_printf("/service%d", id++);
	if (!g_dbus_register_interface(connection, path, GATT_SERVICE_IFACE,
				NULL, NULL, service_properties,
				g_strdup(uuid), g_free)) {
		printf("Couldn't register service interface\n");
		g_free(path);
		return NULL;
	}

	return path;
}

#ifdef DUEROS
static void create_wifi_services(void)
{
	char *service_path;
	uint8_t level = ' ';

	service_path = register_service(DUEROS_WIFI_SERVICES_UUID);
	if (!service_path)
		return;
	
	gboolean mSsidPassword = register_characteristic(DUEROS_CHARACTERISTIC_UUID,
						&level, sizeof(level),
						chr_props,
						CONFIG_UUID, /*NULL*/
						desc_props,
						service_path);

	/* Add Wifi Config Characteristic to Immediate Wifi Config Service */
	if (!mSsidPassword) {
		printf("Couldn't register wifi config characteristic (IAS)\n");
		g_dbus_unregister_interface(connection, service_path,
							GATT_SERVICE_IFACE);
		g_free(service_path);
		return;
	}

	services = g_slist_prepend(services, service_path);
	printf("Registered service: %s\n", service_path);
}
#else
static void create_wifi_services(void)
{
	char *service_path;
	uint8_t level = ' ';

	service_path = register_service(WIFI_SERVICES_UUID);
	if (!service_path)
		return;
		
	gboolean mSecure = register_characteristic(SECURITY_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);
	gboolean mHide = register_characteristic(HIDE_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);
	gboolean mSsid = register_characteristic(SSID_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);
	gboolean mPassword = register_characteristic(PASSWORD_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);
    	gboolean mCheckdata = register_characteristic(CHECKDATA_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);
	gboolean mConfigCharNotify = register_characteristic(NOTIFY_CHAR_UUID,
						&level, sizeof(level),
						chr_props,
						NULL,
						desc_props,
						service_path);

	gboolean mWifiListNotify = register_characteristic(WIFILIST_CHAR_UUID,
						&level, sizeof(level),
						chr_props,
						NULL,
						desc_props,
						service_path);

	gboolean mDeviceContextNotify = register_characteristic(DEVICECONTEXT_CHAR_UUID,
						&level, sizeof(level),
						ias_alert_level_props,
						NULL,
						desc_props,
						service_path);

	/* Add Alert Level Characteristic to Immediate Alert Service */
	if (!mSecure || !mHide || !mSsid || !mPassword || !mCheckdata ||
	    !mConfigCharNotify || !mWifiListNotify || !mDeviceContextNotify) {
		printf("Couldn't register Alert Level characteristic (IAS)\n");
		g_dbus_unregister_interface(connection, service_path,
							GATT_SERVICE_IFACE);
		g_free(service_path);
		return;
	}

	services = g_slist_prepend(services, service_path);
	printf("Registered service: %s\n", service_path);
}

#endif

static void send_advertise(){
        printf("send_advertise\n");
        char buff[256] = {0};
        char addr[6];
        char CMD_RA[256] = "hcitool -i hci0 cmd 0x08 0x0005";
        char CMD_ADV_DATA[256] = "hcitool -i hci0 cmd 0x08 0x0008 ";
        char temp[256];
        int len = 21;               //3 + 18            flag+uuid
        
        //creat random address
        int i;
        srand(time(NULL));
        for(i = 0; i < 6;i++)
               addr[i]=rand()&0xFF;
        addr[0] &= 0x3f;	/* Clear two most significant bits */
	addr[0] |= 0x40;	/* Set second most significant bit */
        for(i = 0; i < 6;i++) {
              sprintf(temp,"%02x", addr[i]);
              strcat(CMD_RA, " "); 
              strcat(CMD_RA, temp);        
        }
         printf ("%s\n", CMD_RA);       

        //LE Set Random Address Command
        execute(CMD_RA, buff);
        sleep(1);
        
        //LE SET PARAMETERS
		//execute(CMD_PARA, buff);
        
        // LE Set Advertising Data Command
        //len = len + strlen(BT_NAME) +2;
        sprintf(temp,"%02x", len);
        strcat(CMD_ADV_DATA, temp);                                          //add len
        strcat(CMD_ADV_DATA, " 02 01 06 11 07 ");                            //add flag
        strcat(CMD_ADV_DATA, SERVICES_UUID);                                 //add uuid
/*
        sprintf(temp,"%02x", strlen(BT_NAME) + 1);
        strcat(CMD_ADV_DATA, " ");
        strcat(CMD_ADV_DATA, temp);
        strcat(CMD_ADV_DATA, " 09");
        for (i = 0; i < strlen(BT_NAME); i++) {
                strcat(CMD_ADV_DATA, " ");
                sprintf(temp,"%02x", BT_NAME[i]);
                strcat(CMD_ADV_DATA, temp);
        }
*/    
        execute(CMD_ADV_DATA, buff);
        sleep(1);

        // LE Set Advertise Enable Command
        execute(CMD_EN, buff);
}

static void register_app_reply(DBusMessage *reply, void *user_data)
{
    printf("register_app_reply\n");
	DBusError derr;

	dbus_error_init(&derr);
	dbus_set_error_from_message(&derr, reply);

	if (dbus_error_is_set(&derr))
		printf("RegisterApplication: %s\n", derr.message);
	else
		printf("RegisterApplication: OK\n");

	send_advertise();

	dbus_error_free(&derr);
}

static void register_app_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = "/";
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

	/* TODO: Add options dictionary */

	dbus_message_iter_close_container(iter, &dict);
}

static void register_app(GDBusProxy *proxy)
{
	if (!g_dbus_proxy_method_call(proxy, "RegisterApplication",
					register_app_setup, register_app_reply,
					NULL, NULL)) {
		printf("Unable to call RegisterApplication\n");
		return;
	}
}

struct adapter {
	GDBusProxy *proxy;
	GDBusProxy *ad_proxy;
	GList *devices;
};

static struct adapter *default_ctrl;
static GList *ctrl_list;
static GDBusProxy *default_dev;
static GDBusProxy *default_attr;

#undef bt_shell_printf
#define bt_shell_printf printf


static gboolean device_is_child(GDBusProxy *device, GDBusProxy *master)
{
	DBusMessageIter iter;
	const char *adapter, *path;

	if (!master)
		return FALSE;

	if (g_dbus_proxy_get_property(device, "Adapter", &iter) == FALSE)
		return FALSE;

	dbus_message_iter_get_basic(&iter, &adapter);
	path = g_dbus_proxy_get_path(master);

	if (!strcmp(path, adapter))
		return TRUE;

	return FALSE;
}


static struct adapter *find_ctrl(GList *source, const char *path)
{
        GList *list;

        for (list = g_list_first(source); list; list = g_list_next(list)) {
                struct adapter *adapter = list->data;

                if (!strcasecmp(g_dbus_proxy_get_path(adapter->proxy), path))
                        return adapter;
        }

        return NULL;
}

static void set_default_device(GDBusProxy *proxy, const char *attribute)
{
	char *desc = NULL;
	DBusMessageIter iter;
	const char *path;

	default_dev = proxy;

	if (proxy == NULL) {
		default_attr = NULL;
		goto done;
	}

	if (!g_dbus_proxy_get_property(proxy, "Alias", &iter)) {
		if (!g_dbus_proxy_get_property(proxy, "Address", &iter))
			goto done;
	}

	path = g_dbus_proxy_get_path(proxy);

	dbus_message_iter_get_basic(&iter, &desc);
	desc = g_strdup_printf("[%s%s%s]# ", desc,
				attribute ? ":" : "",
				attribute ? attribute + strlen(path) : "");

done:
	g_free(desc);
}
static struct adapter *adapter_new(GDBusProxy *proxy)
{
	struct adapter *adapter = g_malloc0(sizeof(struct adapter));

	ctrl_list = g_list_append(ctrl_list, adapter);

	if (!default_ctrl)
		default_ctrl = adapter;

	return adapter;
}
static void proxy_added_cb(GDBusProxy *proxy, void *user_data)
{
	const char *iface;

	iface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(iface, "org.bluez.Adapter1")) {
			printf("	new org.bluez.Adapter1\n");
			{
				struct adapter *adapter;
				adapter = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
				if (!adapter)
					adapter = adapter_new(proxy);
			
				adapter->proxy = proxy;
			
				//print_adapter(proxy, COLORED_NEW);
				//bt_shell_set_env(g_dbus_proxy_get_path(proxy), proxy);
			}
	}

	if (g_strcmp0(iface, GATT_MGR_IFACE))
		return;

	register_app(proxy);
}

static void print_fixed_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t *valbool;
	dbus_uint32_t *valu32;
	dbus_uint16_t *valu16;
	dbus_int16_t *vals16;
	unsigned char *byte;
	int len;

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_fixed_array(iter, &valbool, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		//bt_shell_hexdump((void *)valbool, len * sizeof(*valbool));

		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_fixed_array(iter, &valu32, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		//bt_shell_hexdump((void *)valu32, len * sizeof(*valu32));

		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_fixed_array(iter, &valu16, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		//bt_shell_hexdump((void *)valu16, len * sizeof(*valu16));

		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_fixed_array(iter, &vals16, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		//bt_shell_hexdump((void *)vals16, len * sizeof(*vals16));

		break;
	case DBUS_TYPE_BYTE:
		dbus_message_iter_get_fixed_array(iter, &byte, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		//bt_shell_hexdump((void *)byte, len * sizeof(*byte));

		break;
	default:
		return;
	};
}

static void print_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t valbool;
	dbus_uint32_t valu32;
	dbus_uint16_t valu16;
	dbus_int16_t vals16;
	unsigned char byte;
	const char *valstr;
	DBusMessageIter subiter;
	char *entry;

	if (iter == NULL) {
		bt_shell_printf("%s%s is nil\n", label, name);
		return;
	}

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_INVALID:
		bt_shell_printf("%s%s is invalid\n", label, name);
		break;
	case DBUS_TYPE_STRING:
	case DBUS_TYPE_OBJECT_PATH:
		dbus_message_iter_get_basic(iter, &valstr);
		bt_shell_printf("%s%s: %s\n", label, name, valstr);
		break;
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_basic(iter, &valbool);
		bt_shell_printf("%s%s: %s\n", label, name,
					valbool == TRUE ? "yes" : "no");
                if(!strcmp(name, "ServicesResolved")){
                        if (valbool != TRUE) {
                                send_advertise();
                                saveCheckdata(1, "0");
                        } else {
                                saveCheckdata(1, "1");
                        }
                }                 
		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_basic(iter, &valu32);
		bt_shell_printf("%s%s: 0x%08x\n", label, name, valu32);
		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_basic(iter, &valu16);
		bt_shell_printf("%s%s: 0x%04x\n", label, name, valu16);
		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_basic(iter, &vals16);
		bt_shell_printf("%s%s: %d\n", label, name, vals16);
		break;
	case DBUS_TYPE_BYTE:
		dbus_message_iter_get_basic(iter, &byte);
		bt_shell_printf("%s%s: 0x%02x\n", label, name, byte);
		break;
	case DBUS_TYPE_VARIANT:
		dbus_message_iter_recurse(iter, &subiter);
		print_iter(label, name, &subiter);
		break;
	case DBUS_TYPE_ARRAY:
		dbus_message_iter_recurse(iter, &subiter);

		if (dbus_type_is_fixed(
				dbus_message_iter_get_arg_type(&subiter))) {
			print_fixed_iter(label, name, &subiter);
			break;
		}

		while (dbus_message_iter_get_arg_type(&subiter) !=
							DBUS_TYPE_INVALID) {
			print_iter(label, name, &subiter);
			dbus_message_iter_next(&subiter);
		}
		break;
	case DBUS_TYPE_DICT_ENTRY:
		dbus_message_iter_recurse(iter, &subiter);
		entry = g_strconcat(name, " Key", NULL);
		print_iter(label, entry, &subiter);
		g_free(entry);

		entry = g_strconcat(name, " Value", NULL);
		dbus_message_iter_next(&subiter);
		print_iter(label, entry, &subiter);
		g_free(entry);
		break;
	default:
		bt_shell_printf("%s%s has unsupported type\n", label, name);
		break;
	}
}



static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	const char *interface;
	struct adapter *ctrl;

	interface = g_dbus_proxy_get_interface(proxy);
	printf("xxxh : property_changed: %s\n", interface);

	if (!strcmp(interface, "org.bluez.Device1")) {
            if (default_ctrl != NULL)
                printf("xxxh : default_ctrl is not null\n");
		if (default_ctrl && device_is_child(proxy,
					default_ctrl->proxy) == TRUE) {
			DBusMessageIter addr_iter;
			char *str;

			if (g_dbus_proxy_get_property(proxy, "Address",
							&addr_iter) == TRUE) {
				const char *address;
				dbus_message_iter_get_basic(&addr_iter,&address);
				str = g_strdup_printf("[CHG] Device: %s ", address);                     
			} else
				str = g_strdup("");
                        
			if (strcmp(name, "Connected") == 0) {
				dbus_bool_t connected;

				dbus_message_iter_get_basic(iter, &connected);

				if (connected && default_dev == NULL) {
					set_default_device(proxy, NULL);
				} else if (!connected && default_dev == proxy) {
					set_default_device(NULL, NULL);
				}
			}

			print_iter(str, name, iter);
			g_free(str);
		}
	} else if (!strcmp(interface, "org.bluez.Adapter1")) {
		DBusMessageIter addr_iter;
		char *str;

		if (g_dbus_proxy_get_property(proxy, "Address",
						&addr_iter) == TRUE) {
			const char *address;

			dbus_message_iter_get_basic(&addr_iter, &address);
			str = g_strdup_printf("[CHG] Controller %s ", address);
		} else
			str = g_strdup("");

		print_iter(str, name, iter);
		g_free(str);
	} else if (!strcmp(interface, "org.bluez.LEAdvertisingManager1")) {
		DBusMessageIter addr_iter;
		char *str;

		ctrl = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
		if (!ctrl)
			return;

		if (g_dbus_proxy_get_property(ctrl->proxy, "Address",
						&addr_iter) == TRUE) {
			const char *address;

			dbus_message_iter_get_basic(&addr_iter, &address);
			str = g_strdup_printf("[CHG] Controller %s ", address);
		} else
			str = g_strdup("");

		print_iter(str, name, iter);
		g_free(str);
	} else if (proxy == default_attr) {
		char *str;

		str = g_strdup_printf("[CHG] Attribute %s ",
						g_dbus_proxy_get_path(proxy));

		print_iter(str, name, iter);
		g_free(str);
	}
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static bool __terminated = false;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (!__terminated) {
			printf("Terminating\n");
			g_main_loop_quit(main_loop);
		}

		__terminated = true;
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

int main(int argc, char *argv[])
{
	GDBusClient *client;
	guint signal;

	signal = setup_signalfd();
	if (signal == 0)
		return -errno;

	connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);

	g_dbus_attach_object_manager(connection);

	printf("gatt-service unique name: %s\n",
				dbus_bus_get_unique_name(connection));

#ifdef DUEROS
    dueros_socket_thread_create();
#endif

	create_wifi_services();

	client = g_dbus_client_new(connection, "org.bluez", "/");

	g_dbus_client_set_proxy_handlers(client, proxy_added_cb, NULL, property_changed,
									NULL);

	g_main_loop_run(main_loop);

#ifdef DUEROS
    dueros_socket_thread_delete();
#endif

	g_dbus_client_unref(client);

	g_source_remove(signal);

	g_slist_free_full(services, g_free);
	dbus_connection_unref(connection);

	return 0;
}

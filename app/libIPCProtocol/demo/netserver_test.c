#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>

#include "json-c/json.h"
#include "netserver.h"

#define TYPE_ALL    ""
#define TYPE_WIFI    "wifi"
#define TYPE_ETH    "ethernet"

void netserver_test(void)
{
    char *json_str;
    netserver_scan_wifi();

    json_str = netserver_get_service(TYPE_ETH);
    printf("%s, netserver_get_service, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = netserver_get_config("ethernet_84c2e41b56b1_cable");
    printf("%s, netserver_get_config, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = netserver_get_networkip("eth0");
    printf("%s, netserver_get_networkip, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = netserver_get_networkip("wlan0");
    printf("%s, netserver_get_networkip, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

int main( int argc , char ** argv)
{
    GMainLoop *main_loop;

    netserver_test();

    main_loop = g_main_loop_new(NULL, FALSE);

    g_main_loop_run(main_loop);
    if (main_loop)
        g_main_loop_unref(main_loop);

    return 0;
}

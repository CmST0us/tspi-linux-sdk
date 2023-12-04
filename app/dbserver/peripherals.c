#include <stdio.h>
#include <assert.h>
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
#include <gdbus.h>

#include "json-c/json.h"
#include "rkdb.h"
#include "common.h"

#define TABLE_PERIPHERALS_RELAY               "PeripheralsRelay"
#define TABLE_PERIPHERALS_WEIGEN              "PeripheralsWeigen"
#define TABLE_PERIPHERALS_FILL_LIGHT          "PeripheralsFillLight"
#define TABLE_PERIPHERALS_VERSION             "PeripheralsVersion"

#define PERIPHERALS_VERSION             "1.0.0"

int peripherals_dbus_register(DBusConnection *dbus_conn)
{
    g_dbus_register_interface(dbus_conn, "/",
                              DB_PERIPHERALS_INTERFACE,
                              methods,
                              signals, NULL, DB_PERIPHERALS_INTERFACE, NULL);

    return 0;
}

void peripherals_init(void)
{
    char *col_para;

    if (equal_version(TABLE_PERIPHERALS_VERSION, PERIPHERALS_VERSION))
        return;

    g_free(rkdb_drop(TABLE_PERIPHERALS_RELAY));
    g_free(rkdb_drop(TABLE_PERIPHERALS_WEIGEN));
    g_free(rkdb_drop(TABLE_PERIPHERALS_FILL_LIGHT));
    g_free(rkdb_drop(TABLE_PERIPHERALS_VERSION));

    creat_version_table(TABLE_PERIPHERALS_VERSION, PERIPHERALS_VERSION);

    col_para = "id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "iIOIndex INT," \
               "iEnable INT DEFAULT 0," \
               "iValidLevel INT DEFAULT 1," \
               "iDuration INT DEFAULT 500";
    g_free(rkdb_create(TABLE_PERIPHERALS_RELAY, col_para));
    g_free(rkdb_insert(TABLE_PERIPHERALS_RELAY, "id, iIOIndex", "0, 0"));

    
    col_para = "id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "iEnable INT DEFAULT 0," \
               "iWiegandBit INT DEFAULT 26," \
               "iDuration INT DEFAULT 0";
    g_free(rkdb_create(TABLE_PERIPHERALS_WEIGEN, col_para));
    g_free(rkdb_insert(TABLE_PERIPHERALS_WEIGEN, "id", "0"));

    col_para = "id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "iSaveEnergyEnable INT DEFAULT 0," \
               "iSaveEnergyBrightness INT DEFAULT 50," \
               "iNormalBrightness INT DEFAULT 50";
    g_free(rkdb_create(TABLE_PERIPHERALS_FILL_LIGHT, col_para));
    g_free(rkdb_insert(TABLE_PERIPHERALS_FILL_LIGHT, "id", "0"));
}


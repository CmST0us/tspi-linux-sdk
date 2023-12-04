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
#include <libgen.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "rkdb.h"
#include "network.h"
#include "storage.h"
#include "media.h"
#include "common.h"
#include "system.h"
#include "event.h"
#include "log.h"
#include "peripherals.h"

enum {
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

int enable_minilog = 0;
int dbserver_log_level = LOG_INFO;

int main(int argc , char ** argv)
{
    GMainLoop *main_loop;
    DBusError dbus_err;
    DBusConnection *dbus_conn;
    char *db_file = "/userdata/sysconfig.db";
    char *db_path;
#ifdef ENABLE_MINILOGGER
    enable_minilog = 1;
    __minilog_log_init(argv[0], NULL, false, false, "dbserver","1.0");
#endif
    if (argc == 2)
        db_file = argv[1];

    db_path = g_strdup(db_file);
    db_path = dirname(db_path);

    if (access(db_path, 0)) {
        LOG_INFO("dbserver: %s folder does not exist\n", db_path);
        return 0;
    }

    if (db_path)
        g_free(db_path);

    rkdb_init(db_file);

    network_init();
    storage_init();
    media_init();
    system_init();
    event_init();
    peripherals_init();

    LOG_INFO("dbserver init finish\n");

    dbus_error_init(&dbus_err);
    dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, DB_SERVER, &dbus_err);

    main_loop = g_main_loop_new(NULL, FALSE);

    network_dbus_register(dbus_conn);
    storage_dbus_register(dbus_conn);
    media_dbus_register(dbus_conn);
    system_dbus_register(dbus_conn);
    event_dbus_register(dbus_conn);
    peripherals_dbus_register(dbus_conn);

    LOG_INFO("dbserver dbus register finish\n");

    g_main_loop_run(main_loop);
    rkdb_deinit();
    if (main_loop)
        g_main_loop_unref(main_loop);

    return 0;
}

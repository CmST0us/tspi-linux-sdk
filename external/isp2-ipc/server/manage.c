#if CONFIG_DBSERVER
#include "../utils/log.h"
#include "json-c/json.h"
#include "dbus_signal.h"
#include "isp_func.h"
#include <gdbus.h>

#define ISPSERVER "rockchip.ispserver"
#define ISPSERVER_PATH "/"
#define ISPSERVER_INTERFACE ISPSERVER ".server"

#define ISP_STATUS_CMD                                            \
  "{ \"status\": \"%d\" }"                                                     \

static DBusConnection *g_isp_conn = NULL;
static int manage_init_flag = 0;
extern rk_aiq_working_mode_t gc_hdr_mode;

static gboolean signal_ispchanged(DBusConnection *conn, char *interface, char *json_str)
{
    DBusMessage *signal;
    DBusMessageIter iter;

    if (interface == NULL)
        return FALSE;

    signal = dbus_message_new_signal(ISPSERVER_PATH,
                                     interface, "IspStatusChanged");
    if (!signal)
        return FALSE;

    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(conn, signal, NULL);
    dbus_message_unref(signal);

    return TRUE;
}

static int isp_signal_send(int status) {
  char j_str[128] = {0};
  sprintf(j_str, ISP_STATUS_CMD, status);
  LOG_INFO("isp status: %d\n", status);
  signal_ispchanged(g_isp_conn, ISPSERVER_INTERFACE, j_str);
  return 0;
}

static DBusMessage *get_dump_exposure_info(DBusConnection *conn,
                                           DBusMessage *msg, void *data) {
  const char *sender;
  char *interface;
  char str[128] = {'\0'};
  DBusMessage *reply;
  DBusMessageIter array;
  dbus_bool_t onoff;
  LOG_DEBUG("get_dump_exposure_info\n");

  sender = dbus_message_get_sender(msg);
  dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &interface,
                        DBUS_TYPE_INVALID);

  Uapi_ExpQueryInfo_t stExpInfo;
  rk_aiq_wb_cct_t stCCT;
  exposure_info_get(&stExpInfo, &stCCT);
  if (gc_hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
    sprintf(str, "M:%.0f-%.1f LM:%.1f CT:%.1f",
            stExpInfo.CurExpInfo.LinearExp.exp_real_params.integration_time *
                1000 * 1000,
            stExpInfo.CurExpInfo.LinearExp.exp_real_params.analog_gain,
            stExpInfo.MeanLuma, stCCT.CCT);
  } else {
    sprintf(str, "S:%.0f-%.1f M:%.0f-%.1f L:%.0f-%.1f SLM:%.1f MLM:%.1f "
                 "LLM:%.1f CT:%.1f",
            stExpInfo.CurExpInfo.HdrExp[0].exp_real_params.integration_time *
                1000 * 1000,
            stExpInfo.CurExpInfo.HdrExp[0].exp_real_params.analog_gain,
            stExpInfo.CurExpInfo.HdrExp[1].exp_real_params.integration_time *
                1000 * 1000,
            stExpInfo.CurExpInfo.HdrExp[1].exp_real_params.analog_gain,
            stExpInfo.CurExpInfo.HdrExp[2].exp_real_params.integration_time *
                1000 * 1000,
            stExpInfo.CurExpInfo.HdrExp[2].exp_real_params.analog_gain,
            stExpInfo.HdrMeanLuma[0], stExpInfo.HdrMeanLuma[1],
            stExpInfo.HdrMeanLuma[2], stCCT.CCT);
  }
  LOG_DEBUG("isp exposure dump: %s\n", str);

  reply = dbus_message_new_method_return(msg);
  if (!reply)
    return NULL;
  dbus_message_iter_init_append(reply, &array);
  const char *str_const = (const char *)&str;
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING,
                                 &str_const); // must const

  return reply;
}

static DBusMessage *sendTurnoffSignal(DBusConnection *conn,
                             DBusMessage *msg, void *data)
{
    isp_signal_send(2);
    LOG_INFO("sendTurnoffSignal\n");
    return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static const GDBusMethodTable server_methods[] = {
    {GDBUS_METHOD("GetDumpExposureInfo", NULL, GDBUS_ARGS({"str", "s"}),
                  get_dump_exposure_info)},
    {
      GDBUS_ASYNC_METHOD("SendTurnoffIspSignal",
      NULL, NULL, sendTurnoffSignal)
    },
    {},
};

static const GDBusSignalTable server_signals[] = {
    {
        GDBUS_SIGNAL("IspStatusChanged",
        GDBUS_ARGS({ "json", "s" }))
    },
    { },
};

void manage_init(void) {
  if (manage_init_flag)
    return;
  manage_init_flag = 1;
  LOG_INFO("manage_init\n");
  DBusError err;
  DBusConnection *connection;

  dbus_error_init(&err);
#if CONFIG_CALLFUNC
  LOG_INFO("register NULL dbus conn\n");
  connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, &err);
#else
  LOG_INFO("register %s dbus conn\n", ISPSERVER);
  connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, ISPSERVER, &err);
#endif
  if (!connection) {
    LOG_ERROR("connect fail\n");
    return;
  }
  g_isp_conn = connection;
  g_dbus_register_interface(g_isp_conn, "/", ISPSERVER_INTERFACE,
                            server_methods, server_signals, NULL, NULL, NULL);
  if (dbus_error_is_set(&err)) {
    LOG_ERROR("Error: %s\n", err.message);
  }
  int ret = isp_status_sender_register(&isp_signal_send);
  if (ret) {
    LOG_ERROR("isp_status_sender_register Error: %d\n", ret);
  }
}

#endif
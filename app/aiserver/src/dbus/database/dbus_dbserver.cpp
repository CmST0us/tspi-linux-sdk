// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_dbserver.h"
#include "logger/log.h"

#ifdef USE_RKMEDIA
#include <nlohmann/json.hpp>
#include "flow_db_protocol.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_dbserver"

namespace rockchip {
namespace aiserver {

DBusDbServer::DBusDbServer(DBus::Connection &connection,
                           const char *adaptor_path, const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusDbServer::~DBusDbServer() {}

void DBusDbServer::DataChanged(const std::string &param) {
    // NO NEED TODO
}

std::string DBusDbServer::SelectVideoDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_VIDEO_CMD, id);
  return Cmd(tmp);
}

std::string DBusDbServer::SelectAudioDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_AUDIO_CMD, id);
  return Cmd(tmp);
}

std::string DBusDbServer::SelectOsdDb(int region_id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_OSD_REGION_CMD, region_id);
  return Cmd(tmp);
}

std::string DBusDbServer::SelectRoiDb(int region_id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_ROI_REGION_CMD, region_id);
  return Cmd(tmp);
}

DBusDbListener::DBusDbListener(DBus::Connection &connection)
    : DBus::InterfaceProxy(DBSERVER_DBCHANGE_INTERFACE),
      DBus::ObjectProxy(connection, DBSERVER_PATH, DBSERVER_BUS_NAME) {
  connect_signal(DBusDbListener, DataChanged, DataChangedCb);
}

DBusDbListener::~DBusDbListener() {}

void DBusDbListener::DataChangedCb(const DBus::SignalMessage &sig) {
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  printf("DBus data changed [%s]\n", db_data.c_str());

#ifdef USE_RKMEDIA
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  db_protocol->DbDataDispatch(db_data);
#endif
}

DBusDbEvent::DBusDbEvent(DBus::Connection &connection, const char *adaptor_path,
                         const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusDbEvent::~DBusDbEvent() {}

void DBusDbEvent::DataChanged(const std::string &param) {
  // NO NEED TODO
}

std::string DBusDbEvent::SelectRegionInvade(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_REGION_INVADE_CMD, id);
  return Cmd(tmp);
}

std::string DBusDbEvent::SelectMoveDetectDb(int id) {
  char tmp[128];
  sprintf(tmp, DB_SELECT_MOVE_DETECTION_CMD, id);
  return Cmd(tmp);
}

DBusDbEventListener::DBusDbEventListener(DBus::Connection &connection)
    : DBus::InterfaceProxy(DBSERVER_DBEVENT_CHANGE_INTERFACE),
      DBus::ObjectProxy(connection, DBSERVER_PATH, DBSERVER_BUS_NAME) {
  connect_signal(DBusDbEventListener, DataChanged, DataChangedCb);
}

DBusDbEventListener::~DBusDbEventListener() {}

void DBusDbEventListener::DataChangedCb(const DBus::SignalMessage &sig) {
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  printf("DBus envent: [%s]\n", db_data.c_str());

#ifdef USE_RKMEDIA
  std::unique_ptr<FlowDbProtocol> db_protocol;
  db_protocol.reset(new FlowDbProtocol);
  db_protocol->DbDataDispatch(db_data);
#endif
}

} // namespace aiserver
} // namespace rockchip

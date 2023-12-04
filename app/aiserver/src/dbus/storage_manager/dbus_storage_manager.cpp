// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_storage_manager.h"
#include "logger/log.h"

#ifdef USE_RKMEDIA
#include <nlohmann/json.hpp>
#include "flow_export.h"
#include "flow_sm_protocol.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_storage_manager.cpp"

namespace rockchip {
namespace aiserver {

DBusStorageManager::DBusStorageManager(DBus::Connection &connection,
                                       const char *adaptor_path,
                                       const char *adaptor_name)
    : DBus::ObjectProxy(connection, adaptor_path, adaptor_name) {}

DBusStorageManager::~DBusStorageManager() {}

std::string DBusStorageManager::GetStorageStatus() {
  char tmp[128];
  sprintf(tmp, SM_GET_DISK_STATUS);
  return GetDisksStatus(tmp);
}

std::string DBusStorageManager::GetStoragePath() {
  char tmp[128];
  sprintf(tmp, SM_GET_DISK_STATUS);
  return GetMediaPath(tmp);
}

void DBusStorageManager::UpdateDisksStatus(const std::string &param) {
  // TODO
}

void DBusStorageManager::UpdateMediaPath(const std::string &param) {
  // TODO
}

void DBusStorageManager::FreeSizeNotice(const std::string &param) {
  // TODO
}

DBusStorageManagerListen::DBusStorageManagerListen(DBus::Connection &connection)
    : DBus::InterfaceProxy(STORAGE_MANAGER_DBCHANGE_INTERFACE),
      DBus::ObjectProxy(connection, STORAGE_MANAGER_PATH,
                        STORAGE_MANAGER_BUS_NAME) {
  connect_signal(DBusStorageManagerListen, UpdateDisksStatus,
                 UpdateDisksStatusChanged);
  connect_signal(DBusStorageManagerListen, UpdateMediaPath,
                 UpdateMediaPathChanged);
  connect_signal(DBusStorageManagerListen, FreeSizeNotice,
                 FreeSizeNoticeChanged);
}

DBusStorageManagerListen::~DBusStorageManagerListen() {}

void DBusStorageManagerListen::UpdateDisksStatusChanged(
    const DBus::SignalMessage &sig) {
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  LOG_INFO("DBusStorageListen UpdateDisksStatusChanged :\n[%s]\n",
           db_data.c_str());
  if (db_data.empty())
    return;

#ifdef USE_RKMEDIA
  std::unique_ptr<FlowSMProtocol> sm_protocol;
  sm_protocol.reset(new FlowSMProtocol);
  sm_protocol->DisksStatusChanged(db_data);
#endif
}

void DBusStorageManagerListen::UpdateMediaPathChanged(
    const DBus::SignalMessage &sig) {
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  LOG_INFO("DBusStorageListen UpdateMediaPathChanged :\n[%s]\n",
           db_data.c_str());
  if (db_data.empty())
    return;

#ifdef USE_RKMEDIA
  std::unique_ptr<FlowSMProtocol> sm_protocol;
  sm_protocol.reset(new FlowSMProtocol);
  sm_protocol->MediaPathChanged(db_data);
#endif
}

void DBusStorageManagerListen::FreeSizeNoticeChanged(
    const DBus::SignalMessage &sig) {
  DBus::MessageIter it = sig.reader();
  std::string db_data;
  it >> db_data;
  LOG_INFO("DBusStorageListen FreeSizeNoticeChanged :\n[%s]\n",
           db_data.c_str());
  if (db_data.empty())
    return;

#ifdef USE_RKMEDIA
  std::unique_ptr<FlowSMProtocol> sm_protocol;
  sm_protocol.reset(new FlowSMProtocol);
  sm_protocol->FreeSizeNotice(db_data);
#endif
}

} // namespace aiserver
} // namespace rockchip

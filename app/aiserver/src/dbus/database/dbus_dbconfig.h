// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_DBCONFIG_H_
#define _RK_DBUS_DBCONFIG_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

namespace rockchip {
namespace aiserver {

class DbAudioConfig {
public:
  DbAudioConfig() = default;
  ~DbAudioConfig() {}

  int volume;
  int bit_rate;
  int sample_rate;
  std::string audio_source;
  std::string encode_type;
  std::string ans;
};

class DbVideoConfig {
public:
  DbVideoConfig() = default;
  ~DbVideoConfig() {}

  int gop;
  int max_rate;
  int stream_smooth;
  int frame_rate_num;
  int frame_rate_den;
  std::string resolution;
  std::string image_quality;
  std::string output_data_type;
  std::string rc_mode;
  std::string rc_quality;
  std::string smart264;
  std::string svc;
  std::string video_type;
};

class DbMediaConfig {
public:
  DbMediaConfig() = default;
  ~DbMediaConfig() {}
  std::shared_ptr<DbVideoConfig> VideoConfigParse(std::string db_config);
  std::shared_ptr<DbAudioConfig> AudioConfigParse(std::string db_config);

private:
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_DBCONFIG_H_
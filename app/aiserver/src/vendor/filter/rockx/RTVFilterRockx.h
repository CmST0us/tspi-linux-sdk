/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * author: hh@rock-chips.com
 *   date: 2020-5-16
 * module: video filter with ROCKX
 */

#ifndef SRC_RT_MEDIA_AV_FILER_ROCKX_RTVFILTERROCKX_H_
#define SRC_RT_MEDIA_AV_FILER_ROCKX_RTVFILTERROCKX_H_

#include "rockx/rockx_type.h"     // NOLINT

#include "rt_header.h"

class RTMediaBuffer;
class RtMetaData;
class RTVFilterRockx {
 public:
    RTVFilterRockx();
    virtual ~RTVFilterRockx();

 public:
    virtual RT_RET create(RtMetaData *config);
    virtual RT_RET destroy();
    virtual RT_RET invoke(void *data);
    virtual RT_RET doFilter(RTMediaBuffer *src, RtMetaData *extraInfo, RTMediaBuffer *dst);

 protected:
    virtual RT_RET openLib(RtMetaData *meta);
    virtual RT_RET parseLibPath(RtMetaData *meta);
    virtual RT_RET parseModelName(RtMetaData *meta);
    virtual RT_RET parseInputFormat(RtMetaData *meta);
    virtual RT_RET parseAIAlgorithmEnable(RtMetaData *meta);
    //  parser config from  metadata
    virtual RT_RET parseConfig(RtMetaData *meta);

    virtual RT_RET faceDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image);
    virtual RT_RET headDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image);
    virtual RT_RET personDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image);
    virtual void   freeConfig();
    virtual void   dumpRockxObject(void *object);
    virtual RT_RET fillAIResultToMeta(RtMetaData *meta, void *data);

 private:
    void   *mCtx;
    UINT32  mCounter;
};

#endif  // SRC_RT_MEDIA_AV_FILER_ROCKX_RTVFILTERROCKX_H_

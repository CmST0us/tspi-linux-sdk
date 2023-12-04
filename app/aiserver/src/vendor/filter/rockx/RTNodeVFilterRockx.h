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
 */

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNodeVFILTERROCKX_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNodeVFILTERROCKX_H_


#include "rt_type.h"            // NOLINT
#include "rt_header.h"          // NOLINT
#include "RTTaskNode.h"         // NOLINT
#include "RTMediaBuffer.h"      // NOLINT
#include "RTMediaRockx.h"       // NOLINT

class RTNodeVFilterRockx : public RTTaskNode {
 public:
    RTNodeVFilterRockx();
    virtual ~RTNodeVFilterRockx();

    virtual RT_RET open(RTTaskNodeContext *context);
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context);

 protected:
    virtual RT_RET invokeInternal(RtMetaData *meta);
    virtual RT_RET initSupportOptions();

 protected:
    void    *mCtx;
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNodeVFILTERROCKX_H_

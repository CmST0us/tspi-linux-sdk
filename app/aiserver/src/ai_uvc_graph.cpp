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
 *     author: <rimon.xu@rock-chips.com> and <martin.cheng@rock-chips.com>
 *       date: 2020-04-03
 *     module: ai_uvc_graph
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "ai_uvc_graph"

#ifdef DEBUG_FLAG
#undef DEBUG_FLAG
#endif

#define DEBUG_FLAG              0x0

#include <math.h>
#include <thread>
#include <sys/prctl.h>

#include "ai_uvc_graph.h"
#include "RTTaskGraph.h"
#include "RTMediaMetaKeys.h"
#include "rt_string_utils.h"

#define UVC_GRAPH_CONFIG_FILE               "/oem/usr/share/aiserver/aicamera.json"
#define SUBGRAPH_STASTERIA_CONFIG_FILE      "/oem/usr/share/aiserver/subgraph_stasteria.json"
#define SUBGRAPH_AI_MATTING_CONFIG_FILE     "/oem/usr/share/aiserver/subgraph_aimatting.json"
#define GRAPH_STASTERIA_CONFIG_FILE         "/oem/usr/share/aiserver/camera_nv12_rkrga_720_stasteria.json"

#define ISP_SCALE0_NODE_ID              0
#define ISP_BYPASS_NODE_ID              1
#define ISP_SCALE1_NODE_ID              2
#define ZOOM_RGA_NODE_ID                3
#define ST_ASTERIA_FACE_NODE_ID         4
#define EPTZ_NODE_ID                    5
#define EPTZ_RGA_NODE_ID                6
#define UVC_LINK_OUTPUT_NODE_ID         7
#define FACE_LINE_NODE_ID               9
#define ST_ASTERIA_RGA_NODE_ID          10
#define ST_NN_NODE0_ID                  11
#define ST_NN_NODE1_ID                  12
#define ST_NN_NODE2_ID                  13

#define NN_LINK_OUTPUT_NODE_ID          1000
#define AI_MATTING_NODE_ID              64
#define MATTING_LINK_OUTPUT_NODE_ID     1001
#define UVC_SMALL_RGA_NODE_ID           21

#define RT_DETECT_FACE              0x00000001
#define RT_DETECT_FACE_LANDMARK     0x00000002
#define RT_DETECT_FACE_ATTRIBUTE    0x00000004
#define RT_DETECT_FACE_FEATURE      0x00000008
#define RT_DETECT_HAND              0x00000100
#define RT_DETECT_HAND_LANDMARK     0x00000200
#define RT_DETECT_BODY              0x00001000
#define RT_DETECT_VENDOR            0x10000000

#define RT_FEATURE_UVC_MASK         0x0000000f
#define RT_FEATURE_NN_MASK          0x000000f0
#define RT_FEATURE_AIMATTING_MASK   0x00000f00
#define RT_FEATURE_FACEAE_MASK      0x0000f000

#define RT_FEATURE_UVC              0x00000001
#define RT_FEATURE_EPTZ             0x00000002
#define RT_FEATURE_UVC_ZOOM         0x00000003
#define RT_FEATURE_EPTZ_ZOOM        0x00000004
#define RT_FEATURE_UVC_RGA          0x00000005
#define RT_FEATURE_UVC_RGA_ZOOM     0x00000007
#define RT_FEATURE_FACE_LINE        0x00000008
#define RT_FEATURE_UVC_BYPASS       0x00000009

#define RT_FEATURE_NN               0x00000010
#define RT_FEATURE_AIMATTING        0x00000100
#define RT_FEATURE_FACEAE           0x00001000

#define ST_ASTERIA_WIDTH            1280
#define ST_ASTERIA_HEIGHT           720
#define ISP_BYPASS_WIDTH            2560
#define ISP_BYPASS_HEIGHT           1440

#define RT_EPTZ_MANUAL_MAX          3
#define RT_EPTZ_PAN_MAX             10.0
#define RT_EPTZ_PAN_COUNT           20.0
#define RT_EPTZ_TILT_MAX            10.0
#define RT_EPTZ_TILT_COUNT          20.0

#define RT_FORCE_USE_RGA_MIN_HEIGHT 480
#define RT_FORCE_USE_RGA_MIN_WIDTH  640

#define ST_ASTERIA_SCENE_MAIN       "scene_nn"
#define ST_ASTERIA_SCENE_PIC        "scene_pic"
#define ST_ASTERIA_SCENE_EPTZ       "scene_eptz"

typedef struct __AI_UVC_GRAPH_CTX {
    float       mZoom;
    INT32       mWidth;
    INT32       mHeight;
    INT32       mVirWidth;
    INT32       mVirHeight;
    INT32       mQuant;
    RTTaskGraph *mTaskGraph;
    std::thread *mThread;
    RT_BOOL      mRunning;
    RtMutex     *mStateMutex;
    RtCondition *mStateCondition;
    INT32        mFeature;
    RT_BOOL      mRequest;
    INT32        mDetection;
    AI_UVC_EPTZ_MODE  mEptzMode;
    INT32         mEptzVal[RT_EPTZ_MANUAL_MAX];
} AIUVCGraphCtx;

static INT32 gCameraWidth  = 1280;
static INT32 gCameraHeight = 720;

AIUVCGraphCtx* getUVCGraphCtx(void* ctx) {
    return reinterpret_cast<AIUVCGraphCtx *>(ctx);
}

AIUVCGraph::AIUVCGraph(const char* tagName)
        : mCtx(RT_NULL) {
    AIUVCGraphCtx* ctx = rt_malloc(AIUVCGraphCtx);
    ctx->mZoom = 1.0f;
    ctx->mWidth = 1280;
    ctx->mHeight = 720;
    ctx->mVirWidth = 1280;
    ctx->mVirHeight = 720;
    ctx->mTaskGraph = RT_NULL;
    ctx->mRunning = RT_TRUE;
    ctx->mRequest = RT_FALSE;
    ctx->mDetection = 0;
    ctx->mFeature = 0;
    ctx->mStateMutex = new RtMutex();
    ctx->mStateCondition = new RtCondition();
    mMattingCallback = nullptr;
    mNNCallback = nullptr;
    mUVCCallback = nullptr;
    rt_memset(ctx->mEptzVal, 0, sizeof(ctx->mEptzVal));

    mCtx = reinterpret_cast<void *>(ctx);
    initialize();
}

AIUVCGraph::~AIUVCGraph() {
    deinitialize();
}

RT_RET AIUVCGraph::initialize() {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    ctx->mThread = new std::thread(threadLoop, reinterpret_cast<void*>(this));

    return RT_OK;
}

RT_RET AIUVCGraph::deinitialize() {
    stop();
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    ctx->mRunning = RT_FALSE;
    {
        RtMutex::RtAutolock autoLock(ctx->mStateMutex);
        if (ctx->mTaskGraph != RT_NULL) {
            ctx->mTaskGraph->invoke(GRAPH_CMD_STOP, RT_NULL);
        }
        ctx->mRequest = RT_TRUE;
        ctx->mStateCondition->broadcast();
    }
    if (ctx->mThread)
    {
        ctx->mThread->join();
        delete ctx->mThread;
        ctx->mThread = nullptr;
    }

    rt_safe_delete(ctx->mStateMutex);
    rt_safe_delete(ctx->mStateCondition);
    rt_safe_free(ctx);
    return RT_OK;
}

RT_RET AIUVCGraph::prepare() {
    RT_LOGD("prepare in");
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    {
        RtMutex::RtAutolock autoLock(ctx->mStateMutex);
        if (ctx->mTaskGraph != RT_NULL) {
            return RT_OK;
        }
        ctx->mRequest = RT_TRUE;
        ctx->mStateCondition->broadcast();
        ctx->mStateCondition->wait(ctx->mStateMutex);
    }
    return RT_OK;
}

RT_RET AIUVCGraph::start() {
    return RT_OK;
}

RT_RET AIUVCGraph::stop() {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    ctx->mFeature = 0;
    selectLinkMode();

    return ret;
}

void* AIUVCGraph::threadLoop(void* arg) {
    RT_RET ret = RT_OK;
    prctl(PR_SET_NAME, "uvcGraphThread");
    AIUVCGraph *uvcGraph = reinterpret_cast<AIUVCGraph *>(arg);
    AIUVCGraphCtx * ctx = getUVCGraphCtx(uvcGraph->getCtx());

    while (ctx->mRunning) {
        {
            RtMutex::RtAutolock autoLock(ctx->mStateMutex);
            while (!ctx->mRequest) {
                ctx->mStateCondition->wait(ctx->mStateMutex);
            }
        }
        ctx->mStateMutex->lock();
        ctx->mRequest = RT_FALSE;
        if (ctx->mTaskGraph != RT_NULL) {
            ctx->mTaskGraph->release();
            rt_safe_delete(ctx->mTaskGraph);
        }
        RT_LOGD("new request feature 0x%x", ctx->mFeature);
        ret = uvcGraph->setupGraphAndWaitDone();
        if (ret != RT_OK) {
            RT_LOGE("failed to setup uvc graph!");
            continue;
        }
    }
    RT_LOGD("thread out");

    if (ctx->mTaskGraph != RT_NULL) {
        ctx->mTaskGraph->release();
        rt_safe_delete(ctx->mTaskGraph);
    }

    return NULL;
}

RT_RET AIUVCGraph::setupGraphAndWaitDone() {
    RT_RET ret = RT_OK;
    RT_BOOL enableEPTZ = RT_FALSE;
    std::vector<std::string> linkModes;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);

    RT_ASSERT(ctx->mTaskGraph == RT_NULL);
    ctx->mTaskGraph = new RTTaskGraph("uvc");
    ret = ctx->mTaskGraph->autoBuild(UVC_GRAPH_CONFIG_FILE);
    CHECK_EQ(ret, RT_OK);
    ret = setCameraParams();
    CHECK_EQ(ret, RT_OK);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_PREPARE, RT_NULL);
    CHECK_EQ(ret, RT_OK);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_START, RT_NULL);
    CHECK_EQ(ret, RT_OK);

    if (mUVCCallback != nullptr) {
        ctx->mTaskGraph->observeOutputStream("link output",
                                             UVC_LINK_OUTPUT_NODE_ID << 16,
                                             mUVCCallback);
    }
    if (mNNCallback != nullptr) {
        ctx->mTaskGraph->observeOutputStream("nn link output",
                                             NN_LINK_OUTPUT_NODE_ID << 16,
                                             mNNCallback);
    }
    if (mMattingCallback != nullptr) {
        ctx->mTaskGraph->observeOutputStream("matting link output",
                                             MATTING_LINK_OUTPUT_NODE_ID << 16,
                                             mMattingCallback);
    }
    updateAIAlgorithm();
    ctx->mStateMutex->unlock();
    {
        RtMutex::RtAutolock autoLock(ctx->mStateMutex);
        ctx->mStateCondition->broadcast();
    }

    ctx->mTaskGraph->waitUntilDone();
    return RT_OK;
__FAILED:
    rt_safe_delete(ctx->mTaskGraph);
    return ret;
}

RT_RET AIUVCGraph::openUVC() {
    RT_LOGD("in");
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    if ((ctx->mFeature & RT_FEATURE_UVC_MASK) != 0) {
        RT_LOGE("uvc is already running!");
        return ret;
    }

    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
    if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
        (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
        ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
    } else {
        ctx->mFeature |= RT_FEATURE_UVC;
    }
    selectLinkMode();

    return ret;
}

RT_RET AIUVCGraph::closeUVC() {
    RT_LOGD("in");
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    if ((ctx->mFeature & RT_FEATURE_UVC_MASK) == 0) {
        RT_LOGE("uvc is already close!");
        return ret;
    }

    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
    selectLinkMode();

    return ret;
}

RT_RET AIUVCGraph::selectLinkMode() {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RT_ASSERT(ctx->mTaskGraph != RT_NULL);
    ctx->mTaskGraph->clearLinkShips();
    RT_LOGD("ctx->mFeature = 0x%x &uvc= 0x%x,faceae=0x%x", ctx->mFeature, ctx->mFeature & RT_FEATURE_UVC_MASK
                                                                       ,(ctx->mFeature & RT_FEATURE_FACEAE_MASK));
    if ((ctx->mFeature & RT_FEATURE_UVC_MASK) == 0) {
        ctx->mTaskGraph->selectLinkMode((ctx->mFeature & RT_FEATURE_NN_MASK) != 0
                                            ? "nn_isp" : "none",
                                         (ctx->mFeature & RT_FEATURE_AIMATTING_MASK) != 0
                                            ? "aimatting" : "none",
                                         (ctx->mFeature & RT_FEATURE_FACEAE_MASK) != 0
                                            ? "uvc_faceae" : "none");
    } else {
        INT32 uvcMask = ctx->mFeature & RT_FEATURE_UVC_MASK;
        switch (uvcMask) {
          case RT_FEATURE_UVC:
            ctx->mTaskGraph->selectLinkMode("uvc");
            break;
          case RT_FEATURE_EPTZ:
            ctx->mTaskGraph->selectLinkMode("eptz");
            break;
          case RT_FEATURE_UVC_BYPASS:
            ctx->mTaskGraph->selectLinkMode("uvc_bypass");
            break;
          case RT_FEATURE_UVC_ZOOM:
            ctx->mTaskGraph->selectLinkMode("uvc_zoom");
            break;
          case RT_FEATURE_FACE_LINE:
            ctx->mTaskGraph->selectLinkMode("face_line");
            break;
          default:
            RT_LOGE("unsupport uvc mask 0x%x", uvcMask);
            break;
        }

        const char* nnlink = "none";
        if ((ctx->mFeature & RT_FEATURE_NN_MASK) != 0) {
            if (uvcMask == RT_FEATURE_UVC) {
                nnlink = "nn_isp";
            } else {
                nnlink = "nn_linkout";
            }
        }

        ctx->mTaskGraph->selectLinkMode(nnlink,
                                         (ctx->mFeature & RT_FEATURE_AIMATTING_MASK) != 0
                                            ? "aimatting" : "none",
                                         (ctx->mFeature & RT_FEATURE_FACEAE_MASK) != 0
                                            ? "uvc_faceae" : "none");
    }
    return ret;
}

RT_RET AIUVCGraph::observeUVCOutputStream(
        std::function<RT_RET(RTMediaBuffer *)> streamCallback) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    mUVCCallback = std::move(streamCallback);
    if (mUVCCallback != nullptr && ctx->mTaskGraph != RT_NULL) {
        ctx->mTaskGraph->observeOutputStream("link output",
                                             UVC_LINK_OUTPUT_NODE_ID << 16,
                                             mUVCCallback);
    }
    return RT_OK;
}

RT_RET AIUVCGraph::observeNNOutputStream(
        std::function<RT_RET(RTMediaBuffer *)> streamCallback) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    mNNCallback = std::move(streamCallback);
    if (mNNCallback != nullptr && ctx->mTaskGraph != RT_NULL) {
        ctx->mTaskGraph->observeOutputStream("nn link output",
                                             NN_LINK_OUTPUT_NODE_ID << 16,
                                             mNNCallback);
    }
    return RT_OK;
}

RT_RET AIUVCGraph::observeMattingOutputStream(
        std::function<RT_RET(RTMediaBuffer *)> streamCallback) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    mMattingCallback = std::move(streamCallback);
    if (mMattingCallback != nullptr && ctx->mTaskGraph != RT_NULL) {
        ctx->mTaskGraph->observeOutputStream("matting link output",
                                             MATTING_LINK_OUTPUT_NODE_ID << 16,
                                             mMattingCallback);
    }
    return RT_OK;
}

RT_RET AIUVCGraph::updateCameraParams(RtMetaData *params) {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);

    params->findInt32("opt_width",        &(ctx->mWidth));
    params->findInt32("opt_height",       &(ctx->mHeight));
    params->findInt32("opt_vir_width",    &(ctx->mVirWidth));
    params->findInt32("opt_vir_height",   &(ctx->mVirHeight));
    params->findInt32("opt_quantization", &(ctx->mQuant));
    if (gCameraWidth != ctx->mWidth || gCameraHeight != ctx->mHeight) {
        {
            RtMutex::RtAutolock autoLock(ctx->mStateMutex);
            if (ctx->mTaskGraph != RT_NULL) {
                ctx->mTaskGraph->invoke(GRAPH_CMD_STOP, RT_NULL);
            }
            ctx->mRequest = RT_TRUE;
            ctx->mStateCondition->broadcast();
            ctx->mStateCondition->wait(ctx->mStateMutex);
        }
        gCameraWidth = ctx->mWidth;
        gCameraHeight = ctx->mHeight;
    }
__FAILED:
    return ret;
}

RT_RET AIUVCGraph::setCameraParams() {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMetaData params;
    INT32 bypassWidth = ISP_BYPASS_WIDTH;
    INT32 bypassHeight = ISP_BYPASS_HEIGHT;
    char* enableBypassWidth = getenv("CAMERA_MAX_WIDTH");
    char* enableBypassHeight = getenv("CAMERA_MAX_HEIGHT");
    if (enableBypassWidth && strlen(enableBypassWidth) > 0) {
        RT_LOGD_IF(DEBUG_FLAG, "bypass width %s setting", enableBypassWidth);
        bypassWidth = atoi(enableBypassWidth);
    } else {
        RT_LOGE("Cannot get max width from getenv, use default(%d)", bypassWidth);
    }
    if (enableBypassHeight && strlen(enableBypassHeight) > 0) {
        RT_LOGD_IF(DEBUG_FLAG, "bypass height %s setting", enableBypassHeight);
        bypassHeight = atoi(enableBypassHeight);
    } else {
        RT_LOGE("Cannot get max height from getenv, use default(%d)", bypassHeight);
    }

    // stride align to 16 for all node
    ctx->mVirWidth = RT_ALIGN(ctx->mVirWidth, 16);
    ctx->mVirHeight = RT_ALIGN(ctx->mVirHeight, 16);

    // set isp params
    if (ctx->mWidth > RT_FORCE_USE_RGA_MIN_WIDTH){
        params.setInt32("opt_width",         ctx->mWidth);
        params.setInt32("opt_height",        ctx->mHeight);
        params.setInt32("opt_vir_width",     ctx->mVirWidth);
        params.setInt32("opt_vir_height",    ctx->mVirHeight);
        params.setInt32("node_buff_size",    RT_ALIGN(ctx->mWidth, 16) * RT_ALIGN(ctx->mHeight, 16) * 3 / 2);
        params.setInt32("opt_quantization",  ctx->mQuant);
        params.setInt32(kKeyTaskNodeId,      ISP_SCALE0_NODE_ID);
        params.setCString(kKeyPipeInvokeCmd, "update-params");
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
        CHECK_EQ(ret, RT_OK);
    } else {
#ifdef RK356X
        params.setInt32(kKeyTaskNodeId,      ISP_SCALE0_NODE_ID);
        params.setCString(kKeyPipeInvokeCmd, "update-params");
        params.setInt32("opt_quantization",   ctx->mQuant);
        params.setInt32("opt_width",          bypassWidth);
        params.setInt32("opt_height",         bypassHeight);
        params.setInt32("opt_vir_width",      bypassWidth);
        params.setInt32("opt_vir_height",     bypassHeight);
        params.setInt32("node_buff_size",     RT_ALIGN(bypassWidth, 16) * RT_ALIGN(bypassHeight, 16) * 3 / 2);
        params.setInt32("opt_quantization",   ctx->mQuant);
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
        CHECK_EQ(ret, RT_OK);
#endif
    }

    params.clear();
    params.setInt32(kKeyTaskNodeId,      ISP_BYPASS_NODE_ID);
    params.setCString(kKeyPipeInvokeCmd, "update-params");
    params.setInt32("opt_quantization",   ctx->mQuant);
    params.setInt32("opt_width",          bypassWidth);
    params.setInt32("opt_height",         bypassHeight);
    params.setInt32("opt_vir_width",      bypassWidth);
    params.setInt32("opt_vir_height",     bypassHeight);
    params.setInt32("node_buff_size",     RT_ALIGN(bypassWidth, 16) * RT_ALIGN(bypassHeight, 16) * 3 / 2);
    params.setInt32("opt_quantization",   ctx->mQuant);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    CHECK_EQ(ret, RT_OK);

    params.clear();
    params.setInt32("node_buff_size",    ctx->mVirWidth * ctx->mVirHeight * 3 / 2);
    params.setInt32(kKeyTaskNodeId,      EPTZ_RGA_NODE_ID);
    params.setCString(kKeyPipeInvokeCmd, "update-params");
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    CHECK_EQ(ret, RT_OK);
    params.setCString(kKeyPipeInvokeCmd, "set_config");
    params.setInt32("role",              RT_RGA_ROLE_DST);
    params.setInt32("x offset",          0);
    params.setInt32("y offset",          0);
    params.setInt32("width",             ctx->mWidth);
    params.setInt32("height",            ctx->mHeight);
    params.setInt32("horizontal stride", ctx->mWidth);
    params.setInt32("vertical stride",   ctx->mHeight);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    CHECK_EQ(ret, RT_OK);

    params.clear();
    params.setCString(kKeyPipeInvokeCmd, "update-params");
    params.setInt32(kKeyTaskNodeId,      EPTZ_NODE_ID);
    params.setInt32("opt_width",         bypassWidth);
    params.setInt32("opt_height",        bypassHeight);
    params.setInt32("opt_clip_width",    ctx->mWidth);
    params.setInt32("opt_clip_height",   ctx->mHeight);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

    params.clear();
    params.setCString(kKeyPipeInvokeCmd, "update-params");
    params.setInt32(kKeyTaskNodeId,      FACE_LINE_NODE_ID);
    params.setInt32("opt_width",         bypassWidth);
    params.setInt32("opt_height",        bypassHeight);
    params.setInt32("opt_clip_width",    ctx->mWidth);
    params.setInt32("opt_clip_height",   ctx->mHeight);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

    params.clear();
    params.setCString(kKeyPipeInvokeCmd, "update-params");
    params.setInt32(kKeyTaskNodeId,      ZOOM_RGA_NODE_ID);
    params.setInt32("opt_width",         bypassWidth);
    params.setInt32("opt_height",        bypassHeight);
#ifdef RK356X
    if (ctx->mWidth > RT_FORCE_USE_RGA_MIN_WIDTH){
        params.setInt32("opt_width",         ctx->mWidth);
        params.setInt32("opt_height",        ctx->mHeight);
    }
#endif
    params.setInt32("opt_clip_width",    ctx->mWidth);
    params.setInt32("opt_clip_height",   ctx->mHeight);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

    params.clear();
    params.setCString(kKeyPipeInvokeCmd, "set_config");
    params.setInt32(kKeyTaskNodeId,      ST_ASTERIA_RGA_NODE_ID);
    params.setInt32("role",              RT_RGA_ROLE_SRC);
    params.setInt32("x offset",          0);
    params.setInt32("y offset",          0);
    params.setInt32("width",             ctx->mWidth);
    params.setInt32("height",            ctx->mHeight);
    params.setInt32("horizontal stride", ctx->mVirWidth);
    params.setInt32("vertical stride",   ctx->mVirHeight);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    params.setInt32("role",              RT_RGA_ROLE_DST);
    params.setInt32("x offset",          0);
    params.setInt32("y offset",          0);
    params.setInt32("width",             ST_ASTERIA_WIDTH);
    params.setInt32("height",            ST_ASTERIA_HEIGHT);
    params.setInt32("horizontal stride", ST_ASTERIA_WIDTH);
    params.setInt32("vertical stride",   ST_ASTERIA_HEIGHT);
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

    CHECK_EQ(ret, RT_OK);

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::updateEncoderParams(RtMetaData *params) {
    // TODO(@team): encoder process implement in uvc_app
    return RT_OK;
}

RT_RET AIUVCGraph::updateNNParams(RtMetaData *params) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);

    // for sentimes and rockx
    params->setInt32(kKeyTaskNodeId, ST_NN_NODE0_ID);
    RT_RET ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, params);
    CHECK_EQ(ret, RT_OK);

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::preload(RtMetaData *meta) {
    RT_ASSERT(meta != RT_NULL);
    RT_RET ret = RT_OK;
    std::string stScene;
    const char *sceneName = RT_NULL;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    if (!meta->findCString("preload_handle", &sceneName)) {
        RT_LOGE("scene name is null!");
        ret = RT_ERR_UNKNOWN;
        return ret;
    }

    stScene = sceneName;
    if (stScene == "scene_nn") {
        meta->setInt32(kKeyTaskNodeId,         ST_NN_NODE0_ID);
        meta->setCString(kKeyPipeInvokeCmd,    "preload_resource");
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, meta);
        CHECK_EQ(ret, RT_OK);
    } else if (stScene == "scene_eptz") {
        meta->setInt32(kKeyTaskNodeId,         ST_ASTERIA_FACE_NODE_ID);
        meta->setCString(kKeyPipeInvokeCmd,    "preload_resource");
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, meta);
        CHECK_EQ(ret, RT_OK);
    } else {
        RT_LOGE("unsupport stasteria scene %s", stScene.c_str());
    }

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::openAI() {
    RT_LOGD("in");
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    if (!ctx->mTaskGraph->hasLinkMode("nn_isp")) {
        RT_LOGE("link mode(nn) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    if ((ctx->mFeature & RT_FEATURE_NN_MASK) != 0) {
        RT_LOGE("ai is already running!");
        return ret;
    }

    ctx->mFeature &= ~RT_FEATURE_NN_MASK;
    ctx->mFeature |= RT_FEATURE_NN;
    selectLinkMode();

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::closeAI() {
    RT_LOGD("in");
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    if ((ctx->mFeature & RT_FEATURE_NN_MASK) == 0) {
        RT_LOGE("ai is already close!");
        return ret;
    }

    ctx->mFeature &= ~RT_FEATURE_NN_MASK;
    selectLinkMode();

__FAILED:
    return ret;
}

std::string AIUVCGraph::getAIAlgorithmType(std::string type) {
    std::string name = "none";
    RTSTRING_SWITCH(type.c_str()) {
      RTSTRING_CASE(RT_AI_TYPE_FACE_LANDMARK): {
        name = "face_landmark";
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_DETECT): {
        name = "face";
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_ATTRIBUTE): {
        name = "face_attribute";
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_FEATURE): {
        name = "face_feature";
      } break;
      RTSTRING_CASE(RT_AI_TYPE_BODY): {
        name = "body";
      } break;
      RTSTRING_CASE(RT_AI_TYPE_HAND):
      RTSTRING_CASE(RT_AI_TYPE_HAND_LANDMARK): {
        name = "hand";
      } break;
      default:
        RT_LOGE("unsupport algorithm %s", type.c_str());
        break;
    }
    return name;
}

INT32 AIUVCGraph::getDetectionByType(std::string type) {
    INT32 mask = 0;

    RTSTRING_SWITCH(type.c_str()) {
      RTSTRING_CASE(RT_AI_TYPE_FACE_LANDMARK): {
        mask |= RT_DETECT_FACE_LANDMARK;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_DETECT): {
        mask |= RT_DETECT_FACE;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_ATTRIBUTE): {
        mask |= RT_DETECT_FACE_ATTRIBUTE;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_FACE_FEATURE): {
        mask |= RT_DETECT_FACE_FEATURE;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_BODY): {
        mask |= RT_DETECT_BODY;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_HAND): {
        mask |= RT_DETECT_HAND;
      } break;
      RTSTRING_CASE(RT_AI_TYPE_HAND_LANDMARK): {
        mask |= RT_DETECT_HAND_LANDMARK;
      } break;
      default:
        mask |= RT_DETECT_VENDOR;
        break;
      }

    return mask;
}

RT_RET AIUVCGraph::updateAIAlgorithm() {
    RT_RET ret = RT_OK;
    std::string type;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);

    // face
    type = getAIAlgorithmType(RT_AI_TYPE_FACE_DETECT);
    if (ctx->mDetection & RT_DETECT_FACE) {
        setAIAlgorithm(RT_TRUE, type);
        setAIAlgorithm(RT_TRUE, "face_distance");
    } else {
        setAIAlgorithm(RT_FALSE, type);
        setAIAlgorithm(RT_FALSE, "face_distance");
    }

    // face landmark
    type = getAIAlgorithmType(RT_AI_TYPE_FACE_LANDMARK);
    if ((ctx->mDetection & RT_DETECT_FACE_LANDMARK)) {
        setAIAlgorithm(RT_TRUE, type);
    } else {
        setAIAlgorithm(RT_FALSE, type);
    }

    // face attribute
    type = getAIAlgorithmType(RT_AI_TYPE_FACE_ATTRIBUTE);
    if (ctx->mDetection & RT_DETECT_FACE_ATTRIBUTE) {
        setAIAlgorithm(RT_TRUE, type);
    } else {
        setAIAlgorithm(RT_FALSE, type);
    }

    // face feature
    type = getAIAlgorithmType(RT_AI_TYPE_FACE_FEATURE);
    if (ctx->mDetection & RT_DETECT_FACE_FEATURE) {
        setAIAlgorithm(RT_TRUE, type);
    } else {
        setAIAlgorithm(RT_FALSE, type);
    }

    // body feature
    type = getAIAlgorithmType(RT_AI_TYPE_BODY);
    if (ctx->mDetection & RT_DETECT_BODY) {
        setAIAlgorithm(RT_TRUE, type);
    } else {
        setAIAlgorithm(RT_FALSE, type);
    }

    // hand
    type = getAIAlgorithmType(RT_AI_TYPE_HAND);
    if ((ctx->mDetection & RT_DETECT_HAND)
            || (ctx->mDetection & RT_DETECT_HAND_LANDMARK)) {
        setAIAlgorithm(RT_TRUE, type);
    } else {
        setAIAlgorithm(RT_FALSE, type);
    }

    for (auto &detection : mVendorDetections) {
        setAIAlgorithm(detection.second, detection.first);
    }

    return ret;
}

RT_RET AIUVCGraph::setAIAlgorithm(RT_BOOL enable, std::string type) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMetaData params;

    // for sentimes and rockx
    params.setInt32(kKeyTaskNodeId, ST_NN_NODE0_ID);
    params.setCString(kKeyPipeInvokeCmd, "set_nn_config");
    params.setInt32("enable", enable);
    params.setCString("detection", type.c_str());
    RT_RET ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    CHECK_EQ(ret, RT_OK);

    {
        // rockx
        params.setInt32(kKeyTaskNodeId, ST_NN_NODE1_ID);
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

        // for rockx
        params.setInt32(kKeyTaskNodeId, ST_NN_NODE2_ID);
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    }

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::enableAIAlgorithm(std::string type) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    ctx->mStateMutex->lock();
    RT_RET ret = RT_OK;
    INT32 detection = 0;
    if (!ctx->mTaskGraph->hasLinkMode("nn_isp")) {
        RT_LOGE("link mode(nn) unsupport");
        ctx->mStateMutex->unlock();
        return RT_ERR_UNSUPPORT;
    }

    if (ctx->mDetection == 0) {
        ctx->mStateMutex->unlock();
        ret = openAI();
        ctx->mStateMutex->lock();
        CHECK_EQ(ret, RT_OK);
    }

    detection = getDetectionByType(type);
    if ((detection & RT_DETECT_VENDOR) == RT_DETECT_VENDOR) {
        mVendorDetections[type] = RT_TRUE;
    }
    ctx->mDetection |= detection;
    RT_LOGD("type %s, ctx->mDetection = 0x%x", type.c_str(), ctx->mDetection);

    ret = updateAIAlgorithm();
    CHECK_EQ(ret, RT_OK);
    ctx->mStateMutex->unlock();

__FAILED:
    return ret;
}

RT_RET AIUVCGraph::disableAIAlgorithm(std::string type) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    RT_RET ret = RT_OK;
    INT32 detection = 0;

    if ((ctx->mFeature & RT_FEATURE_AIMATTING_MASK) != 0
            && (type == RT_AI_TYPE_FACE_DETECT
                || type == RT_AI_TYPE_FACE_ATTRIBUTE)) {
        return RT_OK;
    }

    detection = getDetectionByType(type);
    if ((detection & RT_DETECT_VENDOR) == RT_DETECT_VENDOR) {
        mVendorDetections[type] = RT_FALSE;
        // it has vendor detection, don't clear vendor detection flags
        RT_BOOL hasVendorDetect = RT_FALSE;
        for (auto &vendorDetect : mVendorDetections) {
            if (vendorDetect.second == RT_TRUE) {
                hasVendorDetect = RT_TRUE;
                break;
            }
        }
        if (!hasVendorDetect) {
            ctx->mDetection &= ~detection;
        }
    } else {
        ctx->mDetection &= ~detection;
    }
    RT_LOGD("type %s, ctx->mDetection = 0x%x", type.c_str(), ctx->mDetection);

    if (ctx->mDetection == 0) {
        goto __FAILED;
    }

    ret = updateAIAlgorithm();
    CHECK_EQ(ret, RT_OK);

__FAILED:
    if (ctx->mDetection == 0) {
        ret = closeAI();
    }
    return ret;
}

RT_RET AIUVCGraph::linkBYPASS(RT_BOOL enable) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    RT_RET ret = RT_OK;
    INT32 isBYPASS = RT_FALSE;
    if (!ctx->mTaskGraph->hasLinkMode("uvc_bypass")) {
        RT_LOGE("link mode(uvc_bypass) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    isBYPASS = ((ctx->mFeature & RT_FEATURE_UVC_MASK) ==  RT_FEATURE_UVC_BYPASS)
                 ? RT_TRUE : RT_FALSE;
    if (isBYPASS == enable) {
        RT_LOGE("eptz mode already is %s", enable ? "bypass" : "non-bypass");
        return ret;
    }
    RT_LOGD("zoom %f enable bypass %d isBYPASS %d", ctx->mZoom, enable, isBYPASS);
    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
    if (ctx->mZoom != 1.0f) {
        if (enable) {
            ctx->mFeature |= RT_FEATURE_UVC_BYPASS;
        }else{
            ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
        }
    } else {
        if (enable) {
            ctx->mFeature |= RT_FEATURE_UVC_BYPASS;
        } else {
            if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
               (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
                ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
            } else {
                ctx->mFeature |= RT_FEATURE_UVC;
            }
        }
    }

    selectLinkMode();
    return ret;
}

RT_RET AIUVCGraph::enableEPTZ(RT_BOOL enableEPTZ) {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    RT_RET ret = RT_OK;
    INT32 isEPTZ = RT_FALSE;
    if (!ctx->mTaskGraph->hasLinkMode("eptz")) {
        RT_LOGE("link mode(eptz) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    isEPTZ = ((ctx->mFeature & RT_FEATURE_UVC_MASK) ==  RT_FEATURE_EPTZ)
                 ? RT_TRUE : RT_FALSE;
    if (isEPTZ == enableEPTZ) {
        RT_LOGE("eptz mode already is %s", enableEPTZ ? "eptz" : "non-eptz");
        return ret;
    }
    RT_LOGD("zoom %f enable eptz %d isEptz %d", ctx->mZoom, enableEPTZ, isEPTZ);
    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
    if (ctx->mZoom != 1.0f) {
        if (enableEPTZ) {
            ctx->mFeature |= RT_FEATURE_EPTZ;
        }else{
            ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
        }
    } else {
        if (enableEPTZ) {
            ctx->mFeature |= RT_FEATURE_EPTZ;
        } else {
            if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
               (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
                ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
            } else {
                ctx->mFeature |= RT_FEATURE_UVC;
            }
        }
    }
    if(isEPTZ && !enableEPTZ){
        RT_LOGD("eptz close reset zoom position");
        RtMetaData params;
        params.clear();
        params.setInt32(kKeyTaskNodeId,      ZOOM_RGA_NODE_ID);
        params.setCString(kKeyPipeInvokeCmd, "reset_position");
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    }

    selectLinkMode();
    return ret;
}

RT_RET AIUVCGraph::setZoom(float val) {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
#if NODE_STATE_DEBUG
    RtMetaData params;
    params.clear();
    params.setInt32(kKeyTaskNodeId,      ISP_BYPASS_NODE_ID);
    params.setCString(kKeyPipeInvokeCmd, "buffer_state");
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    params.clear();
    params.setInt32(kKeyTaskNodeId,      ISP_SCALE0_NODE_ID);
    params.setCString(kKeyPipeInvokeCmd, "buffer_state");
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    params.clear();
    params.setInt32(kKeyTaskNodeId,      ISP_SCALE1_NODE_ID);
    params.setCString(kKeyPipeInvokeCmd, "buffer_state");
    ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);

    ctx->mTaskGraph->dump();
#endif
    if (!ctx->mTaskGraph->hasLinkMode("uvc_zoom")) {
        RT_LOGE("link mode(uvc_zoom) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    if (val < 1.0f || val > 5.0f) {
        RT_LOGE("unsupport zoom %f", val);
        return RT_ERR_UNKNOWN;
    }

    if (ctx->mTaskGraph == RT_NULL || (ctx->mFeature & RT_FEATURE_UVC_MASK) == 0) {
        ctx->mZoom = val;
        return ret;
    }
    if (ctx->mZoom == val) {
        return ret;
    }
    RT_LOGD("ctx->mFeature 0x%x zoom %f", ctx->mFeature, val);
    if (ctx->mZoom == 1.0f) {
        if ((ctx->mFeature & RT_FEATURE_UVC_MASK) == RT_FEATURE_EPTZ) {
            ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
            ctx->mFeature |= RT_FEATURE_EPTZ;
        } else if (ctx->mFeature & RT_FEATURE_UVC) {
            ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
            ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
        }
        selectLinkMode();
    } else if (val == 1.0f) {
        if ((ctx->mFeature & RT_FEATURE_UVC_MASK) == RT_FEATURE_EPTZ) {
            ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
            ctx->mFeature |= RT_FEATURE_EPTZ;
        } else if ((ctx->mFeature & RT_FEATURE_UVC_MASK) == RT_FEATURE_UVC_ZOOM) {
            ctx->mFeature &= ~RT_FEATURE_UVC_MASK;
            if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
               (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
                ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
            } else {
                ctx->mFeature |= RT_FEATURE_UVC;
            }
        }
        selectLinkMode();
    }

    ctx->mZoom = val;
__FAILED:
    return ret;
}

RT_RET AIUVCGraph::waitUntilDone() {
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    if (ctx->mTaskGraph != RT_NULL) {
        return ctx->mTaskGraph->waitUntilDone();
    }

    return RT_OK;
}

RT_RET AIUVCGraph::openAIMatting() {
    RT_LOGD("in");
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    if (!ctx->mTaskGraph->hasLinkMode("aimatting")) {
        RT_LOGE("link mode(aimatting) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    RT_RET ret = enableAIAlgorithm(RT_AI_TYPE_FACE_DETECT);
    CHECK_EQ(ret, RT_OK);
    ret = enableAIAlgorithm(RT_AI_TYPE_FACE_FEATURE);
    CHECK_EQ(ret, RT_OK);

    ctx->mFeature &= ~RT_FEATURE_AIMATTING_MASK;
    ctx->mFeature |= RT_FEATURE_AIMATTING;
    selectLinkMode();
__FAILED:
    return ret;
}

RT_RET AIUVCGraph::closeAIMatting() {
    RT_LOGD("in");
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    if ((ctx->mFeature & RT_FEATURE_NN_MASK) == 0) {
        RT_LOGE("nn is not running, so can't close matting");
        goto __FAILED;
    }

    ret = disableAIAlgorithm(RT_AI_TYPE_FACE_FEATURE);
    CHECK_EQ(ret, RT_OK);

    ctx->mFeature &= ~RT_FEATURE_AIMATTING_MASK;
    selectLinkMode();

__FAILED:
    return RT_OK;
}

RT_RET AIUVCGraph::setEptz(AI_UVC_EPTZ_MODE mode, int val) {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);

    switch (mode) {
      case AI_UVC_EPTZ_PAN:
        if (abs(val) > RT_EPTZ_PAN_MAX) {
            RT_LOGE("unsupport eptz pan val:%d,default set to %d",
                    val, val > 0 ? RT_EPTZ_PAN_MAX : (0 - RT_EPTZ_PAN_MAX));
                val = val > 0 ? RT_EPTZ_PAN_MAX : (0 - RT_EPTZ_PAN_MAX);
        }
        break;
      case AI_UVC_EPTZ_TILT:
        if (abs(val) > RT_EPTZ_TILT_MAX) {
            RT_LOGE("unsupport eptz tilt val:%d,default set to %d",
                    val, val > 0 ? RT_EPTZ_TILT_MAX : (0 - RT_EPTZ_TILT_MAX));
            val = val > 0 ? RT_EPTZ_TILT_MAX : (0 - RT_EPTZ_TILT_MAX);
        }
        break;
      case AI_UVC_EPTZ_AUTO:
        return enableEPTZ(val ? RT_TRUE : RT_FALSE);
      case AI_UVC_BYPASS_LINK:
        return linkBYPASS(val ? RT_TRUE : RT_FALSE);
      default:
        RT_LOGE("unsupport eptz mode:%d", mode);
        return RT_ERR_UNKNOWN;
    }

    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    if (ctx->mTaskGraph == RT_NULL || (ctx->mFeature & RT_FEATURE_UVC_MASK) == 0) {
        ctx->mEptzVal[mode] = val;
        return ret;
    }

    RT_LOGD("ctx feature 0x%x eptz mode %d", ctx->mFeature, mode);
    INT32 isEPTZ = RT_FALSE;
    if (ctx->mZoom == 1.0f && ctx->mWidth >= RT_FORCE_USE_RGA_MIN_WIDTH &&
        ctx->mHeight >= RT_FORCE_USE_RGA_MIN_HEIGHT) {
        RT_LOGE("ptz mode not support this width(>=%d) and height(>=%d) when zoom not open!",
                 RT_FORCE_USE_RGA_MIN_WIDTH, RT_FORCE_USE_RGA_MIN_HEIGHT);
        ctx->mEptzVal[mode] = val;
        return ret;
    }
    isEPTZ = (ctx->mFeature & RT_FEATURE_EPTZ) ? RT_TRUE : RT_FALSE;

    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;

    if (ctx->mZoom != 1.0f) {
        ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
    } else {
        if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
           (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
            ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
        } else {
            ctx->mFeature |= RT_FEATURE_UVC;
        }
    }
    selectLinkMode();
    ctx->mEptzVal[mode] = val;
__FAILED:
    return ret;
}

RT_RET AIUVCGraph::setFaceAE(int enable) {
    RT_RET ret = RT_OK;
    RtMetaData params;

    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RT_LOGD("ctx feature 0x%x setFaceAE", ctx->mFeature);
    if (!ctx->mTaskGraph->hasLinkMode("uvc_faceae")) {
        RT_LOGE("link mode(uvc_faceae) unsupport");
        return RT_ERR_UNSUPPORT;
    }
    if ((enable)){
        ctx->mFeature |= RT_FEATURE_FACEAE_MASK;
    } else {
        ctx->mFeature &= ~RT_FEATURE_FACEAE_MASK;
        params.setInt32(kKeyTaskNodeId, 25);
        params.setCString(kKeyPipeInvokeCmd, "set_faceae_config");
        params.setInt32("enable", 0);
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &params);
    }
    selectLinkMode();
__FAILED:
    return ret;
}

RT_RET AIUVCGraph::setFaceLine(int enable) {
    RT_RET ret = RT_OK;
    RtMetaData params;

    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);

    INT32 isFaceLine = RT_FALSE;
    if (!ctx->mTaskGraph->hasLinkMode("face_line")) {
        RT_LOGE("link mode(face_line) unsupport");
        return RT_ERR_UNSUPPORT;
    }

    isFaceLine = ((ctx->mFeature & RT_FEATURE_UVC_MASK) ==  RT_FEATURE_FACE_LINE)
                 ? RT_TRUE : RT_FALSE;
    if (isFaceLine == enable) {
        RT_LOGE("face line mode already is %s", enable ? "opened" : "closed");
        return ret;
    }
    RT_LOGD("zoom %f enable face_line %d isFaceLine %d", ctx->mZoom, enable, isFaceLine);
    ctx->mFeature &= ~RT_FEATURE_UVC_MASK;

    if (enable) {
        ctx->mFeature |= RT_FEATURE_FACE_LINE;
    } else {
        if (ctx->mHeight <= RT_FORCE_USE_RGA_MIN_HEIGHT ||
           (ctx->mWidth == 1024 && ctx->mHeight == 768)) {
            ctx->mFeature |= RT_FEATURE_UVC_ZOOM;
        } else {
            ctx->mFeature |= RT_FEATURE_UVC;
        }
    }

    selectLinkMode();

    return ret;
}

RT_RET AIUVCGraph::invoke(INT32 cmd, void *data) {
    RT_RET ret = RT_OK;
    AIUVCGraphCtx * ctx = getUVCGraphCtx(mCtx);
    RtMutex::RtAutolock autoLock(ctx->mStateMutex);
    RtMetaData *meta = reinterpret_cast<RtMetaData *>(data);
    if (ctx->mTaskGraph == RT_NULL) {
        RT_LOGE("mTaskGraph is null");
        ret = RT_ERR_NULL_PTR;
        goto __FAILED;
    }

    switch (cmd) {
      case GRAPH_CMD_TASK_NODE_PRIVATE_CMD:
        ret = ctx->mTaskGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, meta);
        break;
      default:
        RT_LOGE("unsupport cmd 0x%x", cmd);
        ret = RT_ERR_UNSUPPORT;
        break;
    }

    CHECK_EQ(ret, RT_OK);
__FAILED:
    return ret;
}

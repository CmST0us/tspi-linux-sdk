// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ai_scene_director.h"
#include "logger/log.h"

#include "ai_uvc_graph.h"
#include "RTMediaBuffer.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AISceneDirector"

#define ZOOM_NODE_ID                3

namespace rockchip {
namespace aiserver {

// send NN data to SDS
RT_RET AISceneDirector::nn_data_output_callback(RTMediaBuffer *buffer) {
#if UVC_DYNAMIC_DEBUG
    if (!access(UVC_DYNAMIC_DEBUG_IPC_BUFFER_CHECK, 0)) {
        LOG_ERROR("send nn data buffer %p\n", buffer);
    }
#endif
    mAITaskManager->processAIData(buffer);
    return RT_OK;
}

// send NN data to SDS
RT_RET AISceneDirector::ai_matting_output_callback(RTMediaBuffer *buffer) {
    mAITaskManager->processAIMatting(buffer);
    return RT_OK;
}

// send MediaBuffer to UVC_APP
RT_RET AISceneDirector::uvc_data_output_callback(RTMediaBuffer *buffer) {
    LOG_DEBUG("callback uvc buffer[%p](id=%d,fd=%d,size=%d,len=%d)\n",
         buffer, buffer->getUniqueID(), buffer->getFd(), buffer->getSize(), buffer->getLength());
    mUVCController->sendUVCBuffer(buffer);
    return RT_OK;
}

AISceneDirector::AISceneDirector() {
    mAITaskManager = new AITaskManager();
    mAIFeatureRetriver = new AIFeatureRetriver();
    mUVCController = new ShmUVCController();
    mUVCController->setControlListener(this);
    mUVCController->startRecvMessage();

    mUVCGraph = nullptr;
}

AISceneDirector::~AISceneDirector() {
    if (nullptr != mUVCGraph) {
        delete mUVCGraph;
        mUVCGraph = nullptr;
    }

    if (mUVCController != nullptr) {
        mUVCController->stopRecvMessage();
        delete mUVCController;
        mUVCController = nullptr;
    }

    if (mAIFeatureRetriver != nullptr) {
        mAIFeatureRetriver->stop();
        delete mAIFeatureRetriver;
        mAIFeatureRetriver = nullptr;
    }

    if (mAITaskManager != nullptr) {
        delete mAITaskManager;
        mAITaskManager = nullptr;
    }
}

int32_t AISceneDirector::setup() {
    prepareUVCGraph();

#if PRELOAD_HANDLE_AI
    RtMetaData meta;
    meta.setCString("preload_handle", "scene_nn");
    LOG_INFO("preload scene nn resource in\n");
    mUVCGraph->preload(&meta);
    LOG_INFO("preload scene nn resource ok\n");
#endif

#if HAVE_STASTERIA
    mAIFeatureRetriver->setup(mAITaskManager);
    mAIFeatureRetriver->start();
#endif
    return 0;
}

int32_t AISceneDirector::prepareUVCGraph() {
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (mUVCGraph == NULL) {
        mUVCGraph = new AIUVCGraph("aiuvc");
        mUVCGraph->observeUVCOutputStream(std::bind(&AISceneDirector::uvc_data_output_callback, this, std::placeholders::_1));
        mUVCGraph->observeNNOutputStream(std::bind(&AISceneDirector::nn_data_output_callback, this, std::placeholders::_1));
        mUVCGraph->observeMattingOutputStream(std::bind(&AISceneDirector::ai_matting_output_callback, this, std::placeholders::_1));
        mUVCGraph->prepare();
        mUVCGraph->start();
    }

    return 0;
}

int32_t AISceneDirector::interrupt() {
    LOG_INFO("interrupt uvc graph(ref=%d)\n", mUVCGraphRef);
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (nullptr != mUVCGraph && mUVCGraphRef == 0) {
        LOG_INFO("stop uvc graph(ref=%d)\n", mUVCGraphRef);
        mUVCGraph->stop();
        LOG_INFO("release uvc graph\n");
        delete mUVCGraph;
        mUVCGraph = nullptr;
    }

    if (nullptr != mAIFeatureRetriver) {
        mAIFeatureRetriver->stop();
    }

    return 0;
}

int32_t AISceneDirector::waitUntilDone() {
    if (nullptr != mUVCGraph) {
        mUVCGraph->waitUntilDone();
    }

    return 0;
}

int32_t AISceneDirector::start(const std::string &appName) {
    LOG_INFO("start app(%s)\n", appName.c_str());
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (appName == RT_APP_UVC) {
        if (!mEnableUVC) {
            LOG_INFO("start app -> open uvc in\n");
            mUVCGraph->openUVC();
            LOG_INFO("start app -> open uvc ok\n");
            mUVCGraphRef++;
            mEnableUVC = 1;
        }
    } else if (appName == RT_APP_NN) {
        if (!mEnableNN) {
            // mUVCGraph->openAI();
            mUVCGraphRef++;
            mEnableNN = 1;
        }
    } else {
        LOG_ERROR("start for unknown app(%s)\n", appName.c_str());
    }

    return 0;
}

int32_t AISceneDirector::stop(const std::string &appName) {
    LOG_INFO("stop app(%s)\n", appName.c_str());
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (appName == RT_APP_UVC) {
        if (mEnableUVC) {
            LOG_INFO("stop app -> close uvc in\n");
            mUVCGraph->closeUVC();
            LOG_INFO("stop app -> close uvc ok\n");
            mUVCGraphRef--;
            mEnableUVC = 0;
        }
    } else if (appName == RT_APP_NN) {
        if (mEnableNN) {
            // mUVCGraph->closeAI();
            mUVCGraphRef--;
            mEnableNN = 0;
        }
    } else {
        LOG_ERROR("stop for unknown app(%s)\n", appName.c_str());
    }
    // interrupt();

    return 0;
}

int32_t AISceneDirector::observeGraphOutput(const std::string &appName, const int32_t &enable) {
    return 0;
}

int32_t AISceneDirector::setEPTZ(const AI_UVC_EPTZ_MODE &mode, const int32_t &val) {
    LOG_INFO("seteptz mode:(%d) val:(%d)\n", mode, val);
    if (nullptr != mUVCGraph) {
        if(mode == AI_UVC_EPTZ_AUTO){
            mUVCGraph->setEptz(mode, val);
            LOG_INFO("seteptz ok\n");
            return 0;
        }
        mUVCGraph->setEptz(mode, val);
        RtMetaData meta;
        meta.setInt32(kKeyTaskNodeId, ZOOM_NODE_ID);
        if(mode == AI_UVC_EPTZ_PAN)
            meta.setCString(kKeyPipeInvokeCmd, "set_pan");
        if(mode == AI_UVC_EPTZ_TILT)
            meta.setCString(kKeyPipeInvokeCmd, "set_tilt");
        if(mode == AI_UVC_BYPASS_LINK)
            meta.setCString(kKeyPipeInvokeCmd, "set_bypass");
        meta.setInt32("value", val);
        mUVCGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &meta);
        LOG_INFO("set ptz ok\n");
    }
    return 0;
}

int32_t AISceneDirector::setZoom(const double &val) {
    LOG_INFO("setZoom(%f)\n", val);
    float zoomVal = (float)val;
    if (nullptr != mUVCGraph) {
        mUVCGraph->setZoom(zoomVal);
        RtMetaData meta;
        meta.setInt32(kKeyTaskNodeId, ZOOM_NODE_ID);
        meta.setCString(kKeyPipeInvokeCmd, "set_zoom");
        meta.setFloat("value", zoomVal);
        mUVCGraph->invoke(GRAPH_CMD_TASK_NODE_PRIVATE_CMD, &meta);
    }
    LOG_INFO("setZoom ok\n");
    return 0;
}
int32_t AISceneDirector::setFaceAE(const int32_t &enabled) {
    LOG_INFO("setFaceAE enabled:(%d)\n", enabled);
    if (nullptr != mUVCGraph) {
        mUVCGraph->setFaceAE(enabled);
    }
    LOG_INFO("setFaceAE ok\n");
    return 0;
}
int32_t AISceneDirector::setFaceLine(const int32_t &enabled) {
    LOG_INFO("setFaceLine enabled:(%d)\n", enabled);
    if (nullptr != mUVCGraph) {
        mUVCGraph->setFaceLine(enabled);
    }
    LOG_INFO("setFaceLine ok\n");
    return 0;
}

int32_t AISceneDirector::enableAIAlgorithm(const std::string &type) {
    LOG_INFO("enableAIAlgorithm(%s)\n", type.c_str());
    if (nullptr != mUVCGraph) {
        mUVCGraph->enableAIAlgorithm(type);
    }
    LOG_INFO("enableAIAlgorithm ok\n");
    return 0;
}

int32_t AISceneDirector::disableAIAlgorithm(const std::string &type) {
    LOG_INFO("disableAIAlgorithm(%s)\n", type.c_str());
    if (nullptr != mUVCGraph) {
        mUVCGraph->disableAIAlgorithm(type);
    }
    LOG_INFO("disableAIAlgorithm ok\n");
    return 0;
}

int32_t AISceneDirector::updateAIAlgorithmParams(const std::string &params) {
    LOG_INFO("updateAIAlgorithmParams(%s)\n", params.c_str());
    int pos = params.find(":");
    std::string params_name = params.substr(0, pos);
    float params_value = atof(params.substr(pos + 1, params.size() - 1).c_str());
    LOG_INFO("updateAIAlgorithmParams name(%s) value(%.2f) \n", params_name.c_str(), params_value);
    RtMetaData meta;
    meta.setCString(kKeyPipeInvokeCmd, "set_nn_params");
    meta.setCString("st_param_type", params_name.c_str());
    meta.setFloat("st_param_value", params_value);
    if (nullptr != mUVCGraph) {
        mUVCGraph->updateNNParams(&meta);
    }
    LOG_INFO("updateAIAlgorithmParams ok\n");
    return 0;
}

int32_t AISceneDirector::openAIMatting() {
    LOG_INFO("openAIMatting in\n");
    if (nullptr != mUVCGraph) {
        mUVCGraph->openAIMatting();
    }
    LOG_INFO("openAIMatting ok\n");
    return 0;
}

int32_t AISceneDirector::closeAIMatting() {
    LOG_INFO("closeAIMatting in\n");
    if (nullptr != mUVCGraph) {
        mUVCGraph->closeAIMatting();
    }
    LOG_INFO("closeAIMatting ok\n");
    return 0;
}


int32_t AISceneDirector::invoke(const std::string &appName, const std::string &actionName, void *params) {
    LOG_INFO("invoke(app=%s, action=%s)\n", appName.c_str(), actionName.c_str());
    if (appName == RT_APP_UVC) {
        return invokeUVC(actionName, params);
    } else if (appName == RT_APP_AI_FEATURE) {
        return invokeFeature(actionName, params);
    } else {
        LOG_ERROR("unsupport app(%s)\n", appName.c_str());
        return -1;
    }

    return 0;
}

int32_t AISceneDirector::invokeFeature(const std::string &actionName, void *params) {
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (nullptr == mAIFeatureRetriver) {
        LOG_ERROR("ai feature retriver not existed\n");
        return -1;
    }

    if (actionName == RT_ACTION_RETRIVE_FEATURE) {
        mAIFeatureRetriver->runTaskOnce(params);
        LOG_INFO("retrive ai feature ok\n");
        return 0;
    }

    LOG_ERROR("unsupport action(%s)\n", actionName.c_str());
    return -1;
}

int32_t AISceneDirector::invokeUVC(const std::string &actionName, void *params) {
    prepareUVCGraph();

    std::lock_guard<std::mutex> lock(mOpMutex);
    if (nullptr == mUVCGraph) {
        LOG_ERROR("uvc graph not existed\n");
        return -1;
    }

    if (actionName == RT_ACTION_CONFIG_CAMERA) {
        RtMetaData *cameraParams = reinterpret_cast<RtMetaData *>(params);
        mUVCGraph->updateCameraParams(cameraParams);
        LOG_INFO("updateCameraParams ok\n");
        return 0;
    }

    LOG_ERROR("unsupport action(%s)\n", actionName.c_str());
    return -1;
}

int32_t AISceneDirector::ctrlSubGraph(const char* nnName, int32_t enable) {
    return 0;
}

} // namespace aiserver
} // namespace rockchip

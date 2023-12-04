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


#include "RTNodeVFilterRockx.h"       // NOLINT
#include "RTVFilterRockx.h"

#include "rt_log.h"                   // NOLINT
#include "rt_string_utils.h"          // NOLINT
#include "RTNodeCommon.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RTNodeVFilterRockx"   // NOLINT
#ifdef DEBUG_FLAG
#undef DEBUG_FLAG
#endif
#define DEBUG_FLAG 0x0

typedef struct _RTNodeRockxCtx {
    RtMutex     *mLock;
    RTVFilterRockx *mRockx;
    INT32        mFlags;
} RTNodeRockxCtx;

#define OFFSET(x) offsetof(RTRockxCfg, x)
RT_RET RTNodeVFilterRockx::initSupportOptions() {
    // TODO(@media-team): The support options also should define by config xml.
    RTTaskNodeOption options[] = {
        /* -- cmd --  / -- name -- / -- detail -- / -- offset -- / -- type -- / -- default -- / -- min -- / -- max -- */
        { "set_nn_config",  OPT_ROCKX_LIB_PATH, "the path to librockx.so", OFFSET(path),
            RtMetaData::TYPE_C_STRING, { .str = RT_NULL }, 0, 0 },   // NOLINT
        { "set_nn_config",  OPT_ROCKX_MODEL, "the model will be used", OFFSET(model),
            RtMetaData::TYPE_C_STRING, { .str = RT_NULL }, 0, 0 },   // NOLINT
        { "set_nn_config",  OPT_STREAM_FMT_IN, "format of input data", OFFSET(format),
            RtMetaData::TYPE_C_STRING, { .str = RT_NULL }, 0, 0 },   // NOLINT
        { "set_nn_config",  OPT_FILTER_WIDTH, "width of input data", OFFSET(width),
            RtMetaData::TYPE_INT32, { .i64 = 0 }, 0, 65535 },   // NOLINT
        { "set_nn_config",  OPT_FILTER_HEIGHT, "height of input data", OFFSET(height),
            RtMetaData::TYPE_INT32, { .i64 = 0 }, 0, 65535},   // NOLINT
    };

    for (INT32 i = 0; i < sizeof (options) / sizeof(RTTaskNodeOption); i++) {
        mSupportOptions.insert(std::pair<std::string, RTTaskNodeOption>(options[i].name, options[i]));
    }

    return RT_OK;
}


RTNodeVFilterRockx::RTNodeVFilterRockx() {
    RTNodeRockxCtx* ctx = rt_malloc(RTNodeRockxCtx);
    RT_ASSERT(ctx != RT_NULL);
    rt_memset(ctx, 0, sizeof(RTNodeRockxCtx));

    ctx->mLock = new RtMutex();
    RT_ASSERT(RT_NULL != ctx->mLock);

    mCtx = reinterpret_cast<void *>(ctx);
    RT_LOGD_IF(DEBUG_FLAG, "this = %p", this);
}

RTNodeVFilterRockx::~RTNodeVFilterRockx() {
    close(RT_NULL);

    RTNodeRockxCtx* ctx = reinterpret_cast<RTNodeRockxCtx *>(mCtx);
    rt_safe_delete(ctx->mLock);

    rt_safe_free(ctx);
    mCtx = RT_NULL;

    RT_LOGD_IF(DEBUG_FLAG, "this = %p", this);
}

RT_RET RTNodeVFilterRockx::open(RTTaskNodeContext *context) {
    RT_RET        err  = RT_OK;
    RtMetaData   *meta = context->options();
    RTNodeRockxCtx *ctx  = reinterpret_cast<RTNodeRockxCtx *>(mCtx);
    if (RT_NULL == ctx) {
        RT_LOGE("found no context for Rockx");
        return RT_ERR_INIT;
    }

    RT_LOGD_IF(DEBUG_FLAG, "this = %p", this);

    RtMutex::RtAutolock autoLock(ctx->mLock);
    ctx->mRockx = new RTVFilterRockx();
    if (RT_NULL == ctx->mRockx) {
        RT_LOGE("failed to create RgaFilter for this task node.");
        return RT_ERR_INIT;
    }
    err = ctx->mRockx->create(meta);

    return err;
}

RT_RET RTNodeVFilterRockx::process(RTTaskNodeContext *context) {
    RT_RET         err          = RT_OK;
    RTMediaBuffer *inputBuffer  = RT_NULL;
    RTMediaBuffer *outputBuffer = RT_NULL;
    RTNodeRockxCtx  *ctx        = reinterpret_cast<RTNodeRockxCtx *>(mCtx);

    if ((RT_NULL == ctx) || (RT_NULL == ctx->mRockx)) {
        RT_LOGE("found no context for Rockx");
        return RT_ERR_INIT;
    }

    RtMutex::RtAutolock autoLock(ctx->mLock);
    if (!context->inputIsEmpty()) {
        outputBuffer = context->dequeOutputBuffer(RT_TRUE, 0);
        if (outputBuffer == RT_NULL) {
            RT_LOGD("outputBuffer = RT_NULL");
            return RT_OK;
        }

        inputBuffer = context->dequeInputBuffer();
        INT32 streamId = context->getInputInfo()->streamId();
        RtMetaData *extraInfo = inputBuffer->extraMeta(streamId);
        err = ctx->mRockx->doFilter(inputBuffer, extraInfo, outputBuffer);
        if (err == RT_OK) {
            context->queueOutputBuffer(outputBuffer);
            inputBuffer->release();
        } else {
            inputBuffer->release();
            outputBuffer->release();
            // RT_LOGD("doFilter fail, release inputBuffer");
            err = RT_OK;
        }
    } else {
        RT_LOGD("found no input buffer");
        err = RT_OK;
    }

    return err;
}

RT_RET RTNodeVFilterRockx::invokeInternal(RtMetaData *meta) {
    const char *command;
    RTNodeRockxCtx* ctx = reinterpret_cast<RTNodeRockxCtx *>(mCtx);
    if ((RT_NULL == meta) || (RT_NULL == ctx)) {
        return RT_ERR_NULL_PTR;
    }

    RtMutex::RtAutolock autoLock(ctx->mLock);
    meta->findCString(kKeyPipeInvokeCmd, &command);

    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command) {
      RTSTRING_CASE("set_nn_config"):
        if (ctx->mRockx != RT_NULL) {
            ctx->mRockx->invoke(reinterpret_cast<void *>(meta));
        }
        break;

      default:
        RT_LOGD("unsupported command=%d", command);
        break;
    }

    return RT_OK;
}


RT_RET RTNodeVFilterRockx::close(RTTaskNodeContext *context) {
    RTNodeRockxCtx* ctx = reinterpret_cast<RTNodeRockxCtx *>(mCtx);
    if (ctx != RT_NULL && ctx->mRockx != RT_NULL) {
        RtMutex::RtAutolock autoLock(ctx->mLock);
        ctx->mRockx->destroy();
        rt_safe_delete(ctx->mRockx);
    }
    return RT_OK;
}

static RTTaskNode* createRockxFilter() {
    return new RTNodeVFilterRockx();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_rockx {
    .mUid          = 0x09,
    .mName         = "rockx",
    .mVersion      = "v1.0",
    .mCreateObj    = createRockxFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL}, },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_rockx);

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
 * module: Video Filter with Rockx
 */

#include "RTVFilterRockx.h"           // NOLINT
#include <dlfcn.h>                    // NOLINT
#include <unistd.h>

// rockit headers
#include "rt_log.h"                   // NOLINT
#include "rt_string_utils.h"          // NOLINT

// rockx npu header
#include <rockx/rockx.h>                    // NOLINT
#ifdef HAVE_ROCKFACE
#include <rockx/rockface.h>
#endif

#include "RTMediaRockx.h"             // NOLINT
#include "RTNodeCommon.h"             // NOLINT
#include "RTAIDetectResults.h"        // NOLINT
#include "RTNodeCommon.h"             // NOLINT
#include "RTVideoFrame.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RTVFilterRockx"      // NOLINT

#ifdef DEBUG_FLAG
#undef DEBUG_FLAG
#endif
#define DEBUG_FLAG 0x0

#define LIBROCKX    "librockx.so"

#define ROCKX_HEAD_DETECT           "rockx_head_detect"
#define ROCKX_FACE_DETECT_V2        "rockx_face_detect_v2"
#define ROCKX_FACE_DETECT_V2_H      "rockx_face_detect_v2_h"
#define ROCKX_FACE_DETECT_V3        "rockx_face_detect_v3"
#define ROCKX_FACE_DETECT_V3_LARGE  "rockx_face_detect_v3_large"
#define ROCKX_PERSON_DETECT_V2      "rockx_person_detect_v2"

#define ROCKX_INPUT_DEBUG "/tmp/rockx_input"

FILE *rockx_input = nullptr;

static const char* sLibRockxPath[] = {
    "/vendor/lib",  // android
    "/system/lib",  // android
    "/oem/usr/lib",  // linux
    "/usr/lib"      // linux
};

typedef struct _RTRockxPixelFmtEntry {
    rockx_pixel_format rockx_fmt;
    const char *name;
} RTRockxPixelFmtEntry;

static const RTRockxPixelFmtEntry sRockxPixelMap[] = {
    {ROCKX_PIXEL_FORMAT_GRAY8,         "image:gray8"},
    {ROCKX_PIXEL_FORMAT_RGB888,        "image:rgb888"},
    {ROCKX_PIXEL_FORMAT_BGR888,        "image:bgr888"},
    {ROCKX_PIXEL_FORMAT_RGBA8888,      "image:abgr8888"},
    {ROCKX_PIXEL_FORMAT_BGRA8888,      "image:argb8888"},
    {ROCKX_PIXEL_FORMAT_YUV420P_YU12,  "image:yuv420p"},
    {ROCKX_PIXEL_FORMAT_YUV420P_YV12,  "image:yv12"},
    {ROCKX_PIXEL_FORMAT_YUV420SP_NV12, "image:nv12"},
    {ROCKX_PIXEL_FORMAT_YUV420SP_NV21, "image:nv21"},
    {ROCKX_PIXEL_FORMAT_YUV422P_YU16,  "image:uyvy422"},
    {ROCKX_PIXEL_FORMAT_YUV422P_YV16,  "image:yv16"},
    {ROCKX_PIXEL_FORMAT_YUV422SP_NV16, "image:nv16"},
    {ROCKX_PIXEL_FORMAT_YUV422SP_NV61, "image:nv61"},
    {ROCKX_PIXEL_FORMAT_GRAY16,        "image:gray16"},
};

rockx_pixel_format getRockxPixelFmt(const char* fmt) {
    if (fmt != RT_NULL) {
        for (INT32 i = 0; i < RT_ARRAY_ELEMS(sRockxPixelMap); i++) {
            if (!util_strcasecmp(sRockxPixelMap[i].name, fmt)) {
                return sRockxPixelMap[i].rockx_fmt;
            }
        }
    }

    RT_LOGE("unknown pixel format(%s)", fmt);
    return ROCKX_PIXEL_FORMAT_MAX;
}

typedef struct _RTRockxFunc {
    rockx_ret_t (*add_config)(rockx_config_t *config, char *key, char *value);
    rockx_ret_t (*create)(rockx_handle_t *handle, rockx_module_t m, void *config, size_t config_size);
    rockx_ret_t (*destroy)(rockx_handle_t handle);

    rockx_ret_t (*pose_body)(rockx_handle_t handle, rockx_image_t *in_img,
                                   rockx_keypoints_array_t *keypoints_array,
                                   rockx_async_callback* callback);
    rockx_ret_t (*pose_finger)(rockx_handle_t handle, rockx_image_t *in_img, rockx_keypoints_t *keypoints);
    rockx_ret_t (*face_detect)(rockx_handle_t handle, rockx_image_t *in_img, rockx_object_array_t *face_array,
                                   rockx_async_callback* callback);
    rockx_ret_t (*head_detect)(rockx_handle_t handle, rockx_image_t *in_img, rockx_object_array_t *face_array,
                                   rockx_async_callback* callback);
    rockx_ret_t (*person_detect)(rockx_handle_t handle, rockx_image_t *in_img, rockx_object_array_t *person_array,
                                   rockx_async_callback* callback);
    rockx_ret_t (*object_track)(rockx_handle_t handle, int width, int height, int max_track_time,
                                   rockx_object_array_t* in_track_objects,
                                   rockx_object_array_t* out_track_objects);
    rockx_ret_t (*face_landmark)(rockx_handle_t handle, rockx_image_t* in_img, rockx_rect_t *in_box,
                                   rockx_face_landmark_t *out_landmark);
} RTRockxFunc;

typedef struct _RockxRequstCell rockx_request_cell;

typedef struct _RTRockxContext {
    //  handle of "librockx.so"
    void                 *mHandler;
    //  function pointer
    RTRockxFunc           mOpts;
    //  handle size of rknn
    INT32                 mRockxHandleSize;
    //  pointer of rknn handler
    rockx_handle_t       *mRockx;
    // rockx's config
    INT32                 mSkipFramePeriod;
    RTRockxCfg            mCfg;
    rockx_image_t        *mImage;
    // 1 means this node is enble
    RT_BOOL               mIsEnable;
    // size of processCount
    INT32                 processCount;
} RTRockxContext;

struct _RockxRequstCell {
    RTRockxContext       *mRockxCtx;
    RTMediaBuffer        *mRockxBuffer;
    rockx_image_t        *mImage;
    UINT32                mNNType;
    UINT32                mWidth;
    UINT32                mHeight;
};

RTRockxContext* getRockxCtx(void* ctx) {
    return reinterpret_cast<RTRockxContext *>(ctx);
}

RT_RET rockx_NN_result_free(void *data) {
    RTRknnAnalysisResults * results = reinterpret_cast<RTRknnAnalysisResults *>(data);

    rt_safe_free(results->results);
    rt_safe_free(results);
    return RT_OK;
}

RT_RET rockx_ai_result_free(void *data) {
    RTAIDetectResults* ai = reinterpret_cast<RTAIDetectResults *>(data);
    if (ai != RT_NULL) {
        rockx_NN_result_free(ai->privData);
        destroyAIDetectResults(ai);
    }
    return RT_OK;
}

void* copyRockxResult(rockx_request_cell* cell, void *result, size_t result_size) {
    RTRknnResult result_item;
    rt_memset(&result_item, 0, sizeof(RTRknnResult));
    result_item.type  = (RTRknnResultType)cell->mNNType;
    result_item.img_w = cell->mWidth;
    result_item.img_h = cell->mHeight;

    rockx_keypoints_array_t *kps_array = reinterpret_cast<rockx_keypoints_array_t*>(result);
    if ((RT_NULL == kps_array) || (kps_array->count <= 0)) {
        return RT_NULL;
    }


    RTRknnResult* nn_result = rt_malloc_array(RTRknnResult, kps_array->count);
    rt_memset(nn_result, 0, sizeof(RTRknnResult)*kps_array->count);

    RTRknnAnalysisResults* analysisResults = rt_malloc(RTRknnAnalysisResults);
    analysisResults->counter = kps_array->count;
    analysisResults->results = nn_result;

    for (INT32 i = 0; i < kps_array->count; i++) {
        switch (result_item.type) {
          case RT_RKNN_TYPE_BODY:
            rt_memcpy(&result_item.body_info.object, &kps_array->keypoints[i], sizeof(rockx_keypoints_t));
            rt_memcpy(&nn_result[i], &result_item, sizeof(RTRknnResult));
            break;
          default:
            break;
        }
    }

    return (void*)analysisResults;  // NOLINT
}

RTVFilterRockx::RTVFilterRockx() {
    RTRockxContext* ctx = rt_malloc(RTRockxContext);
    rt_memset(ctx, 0, sizeof(RTRockxContext));

    mCounter       = 0;
    ctx->mSkipFramePeriod = 1;
    ctx->mImage     = rt_malloc(rockx_image_t);
    ctx->mIsEnable    = RT_TRUE;
    mCtx = reinterpret_cast<void *>(ctx);
}

RTVFilterRockx::~RTVFilterRockx() {
    destroy();

    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL != ctx) && (RT_NULL != ctx->mHandler)) {
        dlclose(ctx->mHandler);
        ctx->mHandler = NULL;
    }

    rt_safe_free(ctx->mImage);
    rt_safe_free(ctx);
    mCtx = RT_NULL;
}

void RTVFilterRockx::freeConfig() {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if (RT_NULL == ctx) {
        return;
    }

    rt_safe_free(ctx->mCfg.path);
    rt_safe_free(ctx->mCfg.model);
    rt_safe_free(ctx->mCfg.format);
}


RT_RET RTVFilterRockx::create(RtMetaData *config) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if (RT_NULL == ctx) {
        return RT_ERR_NULL_PTR;
    }

    parseConfig(config);
    RT_RET err = openLib(config);
    if (RT_OK != err) {
        RT_LOGE("failed to load rockx library!");
        return err;
    }

    const char* modelname = reinterpret_cast<const char*>(ctx->mCfg.model);
    if (RT_NULL == modelname) {
        RT_LOGE("invalid model name, is NULL.");
        return RT_ERR_NULL_PTR;
    }

    RT_LOGD("model: %s is requested, try to load!", modelname);

    INT32 size = 0;
    rockx_module_t models[10];
    if (!util_strcasecmp(modelname, ROCKX_FACE_DETECT_V3)) {
        models[size++] = ROCKX_MODULE_FACE_DETECTION_V3;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else if (!util_strcasecmp(modelname, ROCKX_FACE_DETECT_V3_LARGE)){
        models[size++] = ROCKX_MODULE_FACE_DETECTION_V3_LARGE;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else if (!util_strcasecmp(modelname, ROCKX_FACE_DETECT_V2)){
        models[size++] = ROCKX_MODULE_FACE_DETECTION_V2;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else if (!util_strcasecmp(modelname, ROCKX_FACE_DETECT_V2_H)){
        models[size++] = ROCKX_MODULE_FACE_DETECTION_V2_HORIZONTAL;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else if (!util_strcasecmp(modelname, ROCKX_HEAD_DETECT)){
        models[size++] = ROCKX_MODULE_HEAD_DETECTION;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else if (!util_strcasecmp(modelname, ROCKX_PERSON_DETECT_V2)){
        models[size++] = ROCKX_MODULE_PERSON_DETECTION_V2;
        models[size++] = ROCKX_MODULE_OBJECT_TRACK;
    } else{
        RT_LOGE("model = %s not support", modelname);
        RT_ASSERT(0);
    }

    ctx->mRockx = rt_calloc_size(rockx_handle_t, size);
    ctx->mRockxHandleSize = size;

    for (INT32 i = 0; i < size; i++) {
        rockx_handle_t handle = RT_NULL;
        rockx_ret_t ret = ctx->mOpts.create(&handle, models[i], RT_NULL, 0);
        if (ret != ROCKX_RET_SUCCESS) {
            RT_LOGE("failed to rockx_create with model:%d, err:%d", models[i], ret);
            return RT_ERR_UNKNOWN;
        }
        ctx->mRockx[i] = handle;
    }

    ctx->mSkipFramePeriod = 1;
    if ((config != NULL) && !config->findInt32(OPT_ROCKX_SKIP_FRAME, &(ctx->mSkipFramePeriod))) {
        ctx->mSkipFramePeriod = 1;
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::destroy() {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL != ctx) && (RT_NULL != ctx->mRockx)) {
        for (INT32 i = 0; i < ctx->mRockxHandleSize; i++) {
            if (ctx->mRockx[i] != RT_NULL) {
                ctx->mOpts.destroy(ctx->mRockx[i]);
                ctx->mRockx[i] = RT_NULL;
            }
        }
        ctx->mRockxHandleSize = 0;
        rt_safe_free(ctx->mRockx);
    }

    freeConfig();

    return RT_OK;
}

RT_RET RTVFilterRockx::parseConfig(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == meta)) {
        return RT_ERR_NULL_PTR;
    }

    parseLibPath(meta);
    parseModelName(meta);
    parseInputFormat(meta);
    parseAIAlgorithmEnable(meta);

    return RT_OK;
}

RT_RET RTVFilterRockx::openLib(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if (RT_NULL == ctx) {
        return RT_ERR_NULL_PTR;
    }

    void* handler = RT_NULL;
    char* path    = ctx->mCfg.path;

    // if not set path, using the default
    if (RT_NULL != path) {
        handler = dlopen(path, RTLD_NOW);
        RT_LOGE("try to dlopen(%s). dlerror()=%s", path, dlerror());
    } else {
        char name[128] = {0};
        for (INT32 idx = 0; idx < (sizeof(sLibRockxPath)/sizeof(char*)); idx++) {
            snprintf(name, sizeof(name), "%s/%s", sLibRockxPath[idx], LIBROCKX);
            handler = dlopen(name, RTLD_LAZY);
            // sanitizer: detected memory leaks in _dl_catch_exception
            // there is memory leak in dlopen()/dlclose()
            RT_LOGE("try to dlopen(%s). dlerror()=%s", name, dlerror());
            if (RT_NULL != handler) {
                break;
            }
        }
    }

    if (RT_NULL == handler) {
        RT_LOGE("found no available rockx libraries.");
        return RT_ERR_UNSUPPORT;
    }

    ctx->mOpts.create  = (rockx_ret_t (*)(rockx_handle_t *handle,
                                rockx_module_t m, void *config, size_t config_size))
                            dlsym(handler, "rockx_create");
    ctx->mOpts.destroy = (rockx_ret_t (*)(rockx_handle_t handle))
                            dlsym(handler, "rockx_destroy");
    ctx->mOpts.pose_body = (rockx_ret_t (*)(rockx_handle_t handle, rockx_image_t *in_img,
                                rockx_keypoints_array_t *keypoints_array, rockx_async_callback* callback))
                            dlsym(handler, "rockx_pose_body");
    ctx->mOpts.face_detect = (rockx_ret_t (*)(rockx_handle_t handle, rockx_image_t *in_img,
                                rockx_object_array_t *face_array, rockx_async_callback* callback))
                            dlsym(handler, "rockx_face_detect");
    ctx->mOpts.head_detect = (rockx_ret_t (*)(rockx_handle_t handle, rockx_image_t *in_img,
                                rockx_object_array_t *face_array, rockx_async_callback* callback))
                            dlsym(handler, "rockx_head_detect");
    ctx->mOpts.person_detect = (rockx_ret_t (*)(rockx_handle_t handle, rockx_image_t *in_img,
                                rockx_object_array_t *person_array, rockx_async_callback* callback))
                            dlsym(handler, "rockx_person_detect");
    ctx->mOpts.object_track = (rockx_ret_t (*)(rockx_handle_t handle, int width, int height, int max_track_time,
                                    rockx_object_array_t* in_track_objects,
                                    rockx_object_array_t* out_track_objects))
                            dlsym(handler, "rockx_object_track");
    ctx->mOpts.face_landmark = (rockx_ret_t (*)(rockx_handle_t handle, rockx_image_t* in_img,
                                    rockx_rect_t *in_box,
                                    rockx_face_landmark_t *out_landmark))
                            dlsym(handler, "rockx_face_landmark");
    ctx->mOpts.add_config  = (rockx_ret_t (*)(rockx_config_t *config, char *key, char *value))
                            dlsym(handler, "rockx_add_config");
    if ((ctx->mOpts.create == RT_NULL) || (ctx->mOpts.destroy == RT_NULL)) {
        RT_LOGE("failed to dlsym(%s), error:%s", LIBROCKX, dlerror());
        dlclose(handler);
        return RT_ERR_UNSUPPORT;
    }

    if (ctx->mOpts.pose_body == RT_NULL) {
        RT_LOGD("failed to find rockx_pose_body in %s", LIBROCKX);
    }

    if ((ctx->mOpts.face_detect == RT_NULL) || (ctx->mOpts.object_track == RT_NULL)||
        (ctx->mOpts.face_landmark == RT_NULL) ||(ctx->mOpts.head_detect == RT_NULL)||
        (ctx->mOpts.person_detect == RT_NULL)) {
        RT_LOGD("dlopen rockx, but fail to find the method in %s", LIBROCKX);
    }

    ctx->mHandler = handler;

    return RT_OK;
}

RT_RET RTVFilterRockx::parseLibPath(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == meta)) {
        return RT_ERR_NULL_PTR;
    }

    const char* value = RT_NULL;
    if (meta->findCString(OPT_ROCKX_LIB_PATH,  &value)) {
        char* path = const_cast<char *>(value);
        if (path != RT_NULL) {
            INT32 length = strlen(path);
            // if already store the path before, release it
            rt_safe_free(ctx->mCfg.path);
            ctx->mCfg.path = rt_malloc_size(char, length+1);
            RT_ASSERT(ctx->mCfg.path != RT_NULL);
            rt_memset(ctx->mCfg.path, 0, length+1);
            rt_memcpy(ctx->mCfg.path, path, length);
            RT_LOGD_IF(DEBUG_FLAG, "rockx_lib_path = %s", ctx->mCfg.path);
        }
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::parseModelName(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == meta)) {
        return RT_ERR_NULL_PTR;
    }

    const char* value = RT_NULL;
    if (meta->findCString(OPT_ROCKX_MODEL,   &value)) {
        char *model = const_cast<char *>(value);
        if (model != RT_NULL) {
            rt_safe_free(ctx->mCfg.model);
            INT32 length = strlen(model);
            ctx->mCfg.model = rt_malloc_size(char, length+1);
            RT_ASSERT(ctx->mCfg.model != RT_NULL);
            rt_memset(ctx->mCfg.model, 0, length+1);
            rt_memcpy(ctx->mCfg.model, model, length);
            RT_LOGD_IF(DEBUG_FLAG, "rockx_model = %s", ctx->mCfg.model);
        }
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::parseInputFormat(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == meta)) {
        return RT_ERR_NULL_PTR;
    }

    INT32 intValue = 0;
    if (meta->findInt32(OPT_FILTER_WIDTH,   &intValue)) {
        ctx->mCfg.width = intValue;
    }

    if (meta->findInt32(OPT_FILTER_HEIGHT,   &intValue)) {
        ctx->mCfg.height = intValue;
    }

    const char* value = RT_NULL;
    if (meta->findCString(OPT_STREAM_FMT_IN,   &value)) {
        char* format = const_cast<char *>(value);
        if (format != RT_NULL) {
            rt_safe_free(ctx->mCfg.format);

            INT32 length = strlen(format);
            ctx->mCfg.format = rt_malloc_size(char, length+1);
            rt_memset(ctx->mCfg.format, 0, length+1);
            rt_memcpy(ctx->mCfg.format, format, length);
            RT_LOGD_IF(DEBUG_FLAG, "input format = %s", ctx->mCfg.format);
        }
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::parseAIAlgorithmEnable(RtMetaData *meta) {
    RTRockxContext* ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == meta)) {
        return RT_ERR_NULL_PTR;
    }

    INT32 enable = 0;
    if (meta->findInt32("enable", &enable)) {
        const char* name = RT_NULL;
        if (meta->findCString("detection", &name)) {
            RT_LOGD("mode = %s, name = %s", ctx->mCfg.model, name);
            const char *model = reinterpret_cast<const char *>(ctx->mCfg.model);
            if (!util_strcasecmp(model, ROCKX_FACE_DETECT)
                    && !util_strcasecmp(name, "face")) {
                ctx->mIsEnable = enable;
                RT_LOGD("face enable = %d, this = %p", enable, this);
            } else if (!util_strcasecmp(model, ROCKX_FACE_LANDMARK)
                    && !util_strcasecmp(name, "face_landmark")) {
                ctx->mIsEnable = enable;
                RT_LOGD("face_landmark enable = %d, this = %p", enable, this);
            } else if ((!util_strcasecmp(model, ROCKX_POSE_BODY)
                    || !util_strcasecmp(model, ROCKX_POSE_BODY_V2))
                    && !util_strcasecmp(name, "body")) {
                ctx->mIsEnable = enable;
                RT_LOGD("body enable = %d, this = %p", enable, this);
            }
        }
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::invoke(void *data) {
    RTRockxContext *ctx = getRockxCtx(mCtx);
    RtMetaData *meta    = reinterpret_cast<RtMetaData *>(data);
    if ((RT_NULL == ctx) || (RT_NULL == meta))
        return RT_ERR_NULL_PTR;

    parseConfig(meta);
    return RT_OK;
}

RT_RET RTVFilterRockx::doFilter(RTMediaBuffer *src, RtMetaData *extraInfo, RTMediaBuffer *dst) {
    RT_RET err = RT_OK;

    RTRockxContext *ctx = getRockxCtx(mCtx);
    RTVideoFrame *vframe = reinterpret_vframe(src);
    if ((RT_NULL == ctx) || (RT_NULL == src)) {
        return RT_ERR_NULL_PTR;
    }

    if (!ctx->mIsEnable) {
        return RT_ERR_BAD;
    }

    ctx->processCount++;
    #if 0
    if (ctx->mSkipFramePeriod > 1
            && (ctx->processCount % ctx->mSkipFramePeriod != 0)) {
        RT_LOGD_IF(DEBUG_FLAG, "skip frame interval %d, process count %d",
                    ctx->mSkipFramePeriod, ctx->processCount);
        return err;
    }
    #endif
    INT32 width  = vframe->getWidth();
    if(width == 0)
      width  = ctx->mCfg.width;
    INT32 height = vframe->getHeight();
    if(height == 0)
      height = ctx->mCfg.height;
    rockx_pixel_format format = getRockxPixelFmt(ctx->mCfg.format);


    RT_LOGD_IF(DEBUG_FLAG, "%dx%d, format:%d", width, height, format);
    if (width == 0 || height == 0 || format == ROCKX_PIXEL_FORMAT_MAX) {
        RT_LOGE("invalid parameters, width:%d, height:%d, format:%d", width, height, format);
        mCounter++;
        return RT_ERR_UNKNOWN;
    }

    rockx_image_t input_img;
    input_img.width  = width;
    input_img.height = height;
    input_img.data   = reinterpret_cast<uint8_t *>(src->getData());
    //RT_LOGD_IF(1, "procss dofilter(src addr :%p)",src);
    if (!access(ROCKX_INPUT_DEBUG, 0)) {
        if (nullptr == rockx_input) {
            rockx_input = fopen("/userdata/rockx_input","w+b");
        }
        if (nullptr != rockx_input && nullptr != input_img.data){
            RT_LOGD_IF(1, "DEBUG_ROCKX_INPUT data %p ", reinterpret_cast<uint8_t *>(src->getData()));
            fwrite(reinterpret_cast<uint8_t *>(src->getData()), 1, 1382400, rockx_input);
        }
    }else{
        if (nullptr != rockx_input)
            fclose(rockx_input);
    }

    input_img.pixel_format = format;
    const char *model = reinterpret_cast<const char *>(ctx->mCfg.model);
    //RT_LOGD_IF(1, "procss begin(model:%s, size= %d)", model, src->getLength());
    if (!util_strcasecmp(model, ROCKX_FACE_DETECT_V2) || !util_strcasecmp(model, ROCKX_FACE_DETECT_V3) ||
        !util_strcasecmp(model, ROCKX_FACE_DETECT_V2_H) || !util_strcasecmp(model, ROCKX_FACE_DETECT_V3_LARGE)) {
        err = faceDetect(src, dst->getMetaData(), &input_img);
    } else if(!util_strcasecmp(model, ROCKX_HEAD_DETECT)){
        err = headDetect(src, dst->getMetaData(), &input_img);
    } else if(!util_strcasecmp(model, ROCKX_PERSON_DETECT_V2)){
        err = personDetect(src, dst->getMetaData(), &input_img);
    } else {
        RT_LOGE("model:%s is not supported.", model);
        RT_ASSERT(0);
    }

    RT_LOGD_IF(DEBUG_FLAG, "procss end(model=%s)", model);
    mCounter++;
    return err;
}

void RTVFilterRockx::dumpRockxObject(void *obj) {
    if (obj == RT_NULL)
        return;

    rockx_object_t *object = reinterpret_cast<rockx_object_t *>(obj);
    RT_LOGD_IF(DEBUG_FLAG, "*****************begine frame = %d***************", mCounter);
    RT_LOGD_IF(DEBUG_FLAG, "id = %d, cls_idx = %d, score = %f",
        object->id, object->cls_idx, object->score);
    RT_LOGD_IF(DEBUG_FLAG, "rect (left =%d, top = %d, right = %d, bottom = %d)",
        object->box.left, object->box.top, object->box.right, object->box.bottom);
    RT_LOGD_IF(DEBUG_FLAG, "********************end******************************");
}

RT_RET RTVFilterRockx::faceDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image) {
    RTRockxContext *ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == src) || (RT_NULL == image)) {
        RT_LOGE("invalid parameters, src or image is NULL");
        return RT_ERR_NULL_PTR;
    }

    rockx_handle_t handle_facedetect   = ctx->mRockx[0];
    rockx_handle_t handle_object_track = ctx->mRockx[1];
    if ((RT_NULL == ctx->mOpts.face_detect) || (RT_NULL == handle_facedetect)) {
        RT_LOGE("invalid parameters, rockx face_detect is not ready!");
        return RT_ERR_NULL_PTR;
    }

    if ((RT_NULL == ctx->mOpts.object_track) || (RT_NULL == handle_object_track)) {
        RT_LOGE("invalid parameters, rockx object_track is not ready!");
        return RT_ERR_NULL_PTR;
    }

    rockx_object_array_t face_array;
    rockx_object_array_t object_array;
    rt_memset(&face_array, 0, sizeof(rockx_object_array_t));
    rockx_ret_t ret = ctx->mOpts.face_detect(handle_facedetect, image, &face_array, RT_NULL);
    if ((ret != ROCKX_RET_SUCCESS) || (face_array.count <= 0)) {
        // RT_LOGE("failed to rockx_face_detect, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    ret = ctx->mOpts.object_track(handle_object_track, image->width, image->height, \
                                  1, &face_array, &object_array);
    if ((ret != ROCKX_RET_SUCCESS) || (object_array.count <= 0)) {
        RT_LOGE("failed to rockx_object_track, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    RTRknnResult result_item;
    rt_memset(&result_item, 0, sizeof(RTRknnResult));
    result_item.type = RT_RKNN_TYPE_FACE;

    // store result to MediaBuffer
    RTRknnAnalysisResults* analysisResults = rt_malloc(RTRknnAnalysisResults);
    RT_ASSERT(analysisResults != RT_NULL);
    RTRknnResult* nn_result = rt_malloc_array(RTRknnResult, object_array.count);
    RT_ASSERT(nn_result != RT_NULL);
    rt_memset(nn_result, 0, sizeof(RTRknnResult)*object_array.count);

    analysisResults->counter = object_array.count;
    analysisResults->results = nn_result;

    if (extraInfo != RT_NULL) {
        fillAIResultToMeta(extraInfo, reinterpret_cast<void*>(analysisResults));
    } else {
        RT_LOGD("extraInfo = RT_NULL");
    }

    for (INT32 i = 0; i < object_array.count; i++) {
        rockx_object_t *object = &object_array.object[i];
        rt_memcpy(&result_item.face_info.object, object, sizeof(rockx_object_t));
        result_item.img_w = image->width;
        result_item.img_h = image->height;

        dumpRockxObject(reinterpret_cast<void *>(object));
        rt_memcpy(&nn_result[i], &result_item, sizeof(RTRknnResult));
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::headDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image) {
    RTRockxContext *ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == src) || (RT_NULL == image)) {
        RT_LOGE("invalid parameters, src or image is NULL");
        return RT_ERR_NULL_PTR;
    }

    rockx_handle_t handle_headdetect   = ctx->mRockx[0];
    rockx_handle_t handle_object_track = ctx->mRockx[1];
    if ((RT_NULL == ctx->mOpts.head_detect) || (RT_NULL == handle_headdetect)) {
        RT_LOGE("invalid parameters, rockx face_detect is not ready!");
        return RT_ERR_NULL_PTR;
    }

    if ((RT_NULL == ctx->mOpts.object_track) || (RT_NULL == handle_object_track)) {
        RT_LOGE("invalid parameters, rockx object_track is not ready!");
        return RT_ERR_NULL_PTR;
    }

    rockx_object_array_t face_array;
    rockx_object_array_t object_array;
    rt_memset(&face_array, 0, sizeof(rockx_object_array_t));
    rockx_ret_t ret = ctx->mOpts.head_detect(handle_headdetect, image, &face_array, RT_NULL);
    if ((ret != ROCKX_RET_SUCCESS) || (face_array.count <= 0)) {
        // RT_LOGE("failed to rockx_face_detect, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    ret = ctx->mOpts.object_track(handle_object_track, image->width, image->height, \
                                  1, &face_array, &object_array);
    if ((ret != ROCKX_RET_SUCCESS) || (object_array.count <= 0)) {
        RT_LOGE("failed to rockx_object_track, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    RTRknnResult result_item;
    rt_memset(&result_item, 0, sizeof(RTRknnResult));
    result_item.type = RT_RKNN_TYPE_FACE;

    // store result to MediaBuffer
    RTRknnAnalysisResults* analysisResults = rt_malloc(RTRknnAnalysisResults);
    RT_ASSERT(analysisResults != RT_NULL);
    RTRknnResult* nn_result = rt_malloc_array(RTRknnResult, object_array.count);
    RT_ASSERT(nn_result != RT_NULL);
    rt_memset(nn_result, 0, sizeof(RTRknnResult)*object_array.count);

    analysisResults->counter = object_array.count;
    analysisResults->results = nn_result;

    if (extraInfo != RT_NULL) {
        fillAIResultToMeta(extraInfo, reinterpret_cast<void*>(analysisResults));
    } else {
        RT_LOGD("extraInfo = RT_NULL");
    }

    for (INT32 i = 0; i < object_array.count; i++) {
        rockx_object_t *object = &object_array.object[i];
        rt_memcpy(&result_item.face_info.object, object, sizeof(rockx_object_t));
        result_item.img_w = image->width;
        result_item.img_h = image->height;

        dumpRockxObject(reinterpret_cast<void *>(object));
        rt_memcpy(&nn_result[i], &result_item, sizeof(RTRknnResult));
    }

    return RT_OK;
}

RT_RET RTVFilterRockx::personDetect(RTMediaBuffer *src, RtMetaData *extraInfo, rockx_image_t *image) {
    RTRockxContext *ctx = getRockxCtx(mCtx);
    if ((RT_NULL == ctx) || (RT_NULL == src) || (RT_NULL == image)) {
        RT_LOGE("invalid parameters, src or image is NULL");
        return RT_ERR_NULL_PTR;
    }

    rockx_handle_t handle_persondetect   = ctx->mRockx[0];
    rockx_handle_t handle_object_track = ctx->mRockx[1];
    if ((RT_NULL == ctx->mOpts.person_detect) || (RT_NULL == handle_persondetect)) {
        RT_LOGE("invalid parameters, rockx face_detect is not ready!");
        return RT_ERR_NULL_PTR;
    }

    if ((RT_NULL == ctx->mOpts.object_track) || (RT_NULL == handle_object_track)) {
        RT_LOGE("invalid parameters, rockx object_track is not ready!");
        return RT_ERR_NULL_PTR;
    }

    rockx_object_array_t person_array;
    rockx_object_array_t object_array;
    rt_memset(&person_array, 0, sizeof(rockx_object_array_t));
    rockx_ret_t ret = ctx->mOpts.person_detect(handle_persondetect, image, &person_array, RT_NULL);
    if ((ret != ROCKX_RET_SUCCESS) || (person_array.count <= 0)) {
        // RT_LOGE("failed to rockx_face_detect, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    ret = ctx->mOpts.object_track(handle_object_track, image->width, image->height, \
                                  1, &person_array, &object_array);
    if ((ret != ROCKX_RET_SUCCESS) || (object_array.count <= 0)) {
        RT_LOGE("failed to rockx_object_track, error=%d", ret);
        return RT_ERR_UNKNOWN;
    }

    RTRknnResult result_item;
    rt_memset(&result_item, 0, sizeof(RTRknnResult));
    result_item.type = RT_RKNN_TYPE_FACE;

    // store result to MediaBuffer
    RTRknnAnalysisResults* analysisResults = rt_malloc(RTRknnAnalysisResults);
    RT_ASSERT(analysisResults != RT_NULL);
    RTRknnResult* nn_result = rt_malloc_array(RTRknnResult, object_array.count);
    RT_ASSERT(nn_result != RT_NULL);
    rt_memset(nn_result, 0, sizeof(RTRknnResult)*object_array.count);

    analysisResults->counter = object_array.count;
    analysisResults->results = nn_result;

    if (extraInfo != RT_NULL) {
        fillAIResultToMeta(extraInfo, reinterpret_cast<void*>(analysisResults));
    } else {
        RT_LOGD("extraInfo = RT_NULL");
    }

    for (INT32 i = 0; i < object_array.count; i++) {
        rockx_object_t *object = &object_array.object[i];
        rt_memcpy(&result_item.face_info.object, object, sizeof(rockx_object_t));
        result_item.img_w = image->width;
        result_item.img_h = image->height;

        dumpRockxObject(reinterpret_cast<void *>(object));
        rt_memcpy(&nn_result[i], &result_item, sizeof(RTRknnResult));
    }

    return RT_OK;
}
RT_RET  RTVFilterRockx::fillAIResultToMeta(RtMetaData *meta, void *data) {
    if (meta != RT_NULL) {
        RTAIDetectResults* aiResult = createAIDetectResults();
        RT_ASSERT(aiResult != RT_NULL);

        snprintf(aiResult->vendor, RT_AI_MAX_LEN, "%s", VENDOR_RK_ROCKX);
        snprintf(aiResult->version, RT_AI_MAX_LEN, "1.0.0");
        aiResult->privSize = (data == RT_NULL) ? 0 : sizeof(RTRknnAnalysisResults);
        aiResult->privData = data;

        meta->setPointer(OPT_AI_DETECT_RESULT,
                              aiResult,
                              rockx_ai_result_free);
        return RT_OK;
    } else {
        return RT_ERR_NULL_PTR;
    }
}

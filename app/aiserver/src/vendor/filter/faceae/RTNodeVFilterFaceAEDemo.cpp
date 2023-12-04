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
#include <math.h>
#include <stdlib.h>

#include "RTNodeVFilterFaceAEDemo.h"          // NOLINT
#include "RTNodeCommon.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RTNodeVFilter"
#define kStubRockitFaceAEDemo                MKTAG('f', 'a', 'a', 'e')

#if DBSERVER_SUPPORT
#include "dbserver.h"
#include "json-c/json.h"
#endif
RTNodeVFilterFaceAE::RTNodeVFilterFaceAE()
{
    mLock = new RtMutex();
    RT_ASSERT(RT_NULL != mLock);
}

RTNodeVFilterFaceAE::~RTNodeVFilterFaceAE()
{
    rt_safe_delete(mLock);
}

RT_RET RTNodeVFilterFaceAE::calculatePersonRect(FaceAeAiData *faceae_ai_data,
        FaceAeRect *result_person)
{
    RT_RET ret = RT_OK;
    int countPerson = 0;
    int result[4] = {0, 0, 0, 0};

    if (mSrcWidth == 0 || mSrcHeight == 0)
    {
        RT_LOGI("mSrcWidth and mSrcHeight data is null,width = %d,height = %d \n",mSrcWidth,mSrcHeight);
        return RT_ERR_BAD;
    }
    for (INT32 i = 0; i < faceae_ai_data->face_count; i++)
    {
        INT32 npuW = mFaceAeInfo.face_npu_width;
        INT32 npuH = mFaceAeInfo.face_npu_height;
        float score = faceae_ai_data->face_data[i].score;
        if (score < mFaceAeInfo.face_facedetect_score_shold)
        {
            RT_LOGD("face score can't reach %.2f, now is %.2f \n",
                    mFaceAeInfo.face_facedetect_score_shold, score);
            continue;
        }
        countPerson++;
        INT32 x1 = faceae_ai_data->face_data[i].left;
        INT32 y1 = faceae_ai_data->face_data[i].top;
        INT32 x2 = faceae_ai_data->face_data[i].right;
        INT32 y2 = faceae_ai_data->face_data[i].bottom;
        FaceAeRect rect = {x1 * mSrcWidth / npuW,
                           y1 * mSrcHeight / npuH,
                           (x2 - x1) * mSrcWidth / npuW,
                           (y2 - y1) * mSrcHeight / npuH
                          };
        //RT_LOGD("facedata[%d] ltrb[%d,%d,%d,%d] \n", i, rect.x, rect.y, rect.w,rect.h);
        if (i == 0)
        {
            result[0] = rect.x;
            result[1] = rect.y;
            result[2] = rect.x + rect.w;
            result[3] = rect.y + rect.h;
        }
        else
        {
            if (result[0] > rect.x || result[0] == 0)
                result[0] = rect.x;
            if (result[1] > rect.y || result[1] == 0)
                result[1] = rect.y;
            if (result[2] < rect.x + rect.w || result[2] == 0)
                result[2] = rect.x + rect.w;
            if (result[3] < rect.y + rect.h || result[3] == 0)
                result[3] = rect.y + rect.h;
        }
    }

    if (countPerson == 0)
    {
        return RT_ERR_BAD;
    }
    (*result_person).x = result[0];
    (*result_person).y = result[1];
    (*result_person).w = result[2];
    (*result_person).h = result[3];
    return ret;
}
RT_RET RTNodeVFilterFaceAE::calculateResultRect(FaceAeRect rect)
{
    INT32 result[4];
    bool isMove= false;
    int grid_w,grid_h = 0;
    std::string sendbuf;
    result[0] = rect.x;
    result[1] = rect.y;
    result[2] = rect.w;
    result[3] = rect.h;

    static INT32 temp[4] = {result[0], result[1], result[2], result[3]};
    //判断人物是否在快速移动中, 取中心点
    if (abs(temp[0] - result[0]) > 40 || abs(temp[2] - result[2]) > 40 ||
            abs(temp[1] - result[1]) > 40 || abs(temp[3] - result[3]) > 40)
    {
        ++mFastMoveCount;
        if (mFastMoveCount < mFaceAeInfo.face_fast_move_frame_judge)
        {
            result[0] = temp[0];
            result[1] = temp[1];
            result[2] = temp[2];
            result[3] = temp[3];
        }
        else
        {
            temp[0] = result[0];
            temp[1] = result[1];
            temp[2] = result[2];
            temp[3] = result[3];
            isMove = true;
        }
    }
    else
    {
        result[0] = temp[0];
        result[1] = temp[1];
        result[2] = temp[2];
        result[3] = temp[3];
        mFastMoveCount = 0;
    }
    if (isMove)
    {
        RT_LOGD("current calculateClipRect face_resultXY [%d %d %d %d] \n ",
                result[0], result[1], result[2], result[3]);
        grid_w = mSrcWidth / 15;
        grid_h = mSrcWidth / 15;
        int left = result[0] / grid_w + 1;
        int top  = result[1] / grid_h + 1;
        int right  = result[2] / grid_w + 1;
        int bottom = result[3] / grid_h + 1;
        int grid_value = 0;
        for (int i = left; i <= right; i++)
        {
            for (int j = top; j <= bottom; j++)
            {
                grid_value = (j-1)*15+i-1;
                if (grid_value < 0 || grid_value > 225) continue;
                sendbuf += std::to_string(grid_value);
                if (((j-1)*15+i) != ((bottom -1)*15 + right))
                {
                    sendbuf += ",";
                }
            }
        }
        RT_LOGD("current sendbuf:%s \n ",sendbuf.c_str());
        doIspProcess(sendbuf.c_str(),0);
    }
    return RT_OK;
}
RT_RET RTNodeVFilterFaceAE::faceAE(FaceAeAiData *faceae_ai_data, int delay_time)
{
    RT_RET ret = RT_OK;
    FaceAeRect result_person;
    const char* buf = ",";
    ret = calculatePersonRect(faceae_ai_data, &result_person);
    if (RT_OK == ret)
    {
        ret = calculateResultRect(result_person);
        mNoPersonCount = 0;
    }
    else
    {
        ++ mNoPersonCount;
        if (mNoPersonCount == delay_time * 30)
        {
            RT_LOGD("current sendbuf:%s \n ",buf);
            doIspProcess(buf,0);
        }
    }
    return ret;
}
RT_RET RTNodeVFilterFaceAE::doIspProcess(const char* buf,int evbias)
{
#if DBSERVER_SUPPORT
    char *ret = NULL;
    char *table = TABLE_IMAGE_ADJUSTMENT;
    struct json_object *js = NULL;
    js = json_object_new_object();
    if (NULL == js)
    {
        RT_LOGD("+++new json object failed.\n");
        return RT_ERR_INIT;
    }
    if (NULL != buf)
        json_object_object_add(js, "sGridWeight", json_object_new_string(buf));
    if (evbias > 0)
        json_object_object_add(js, "iEvbias", json_object_new_int(evbias));
    ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
    json_object_put(js);
    dbserver_free(ret);
#endif
    return RT_OK;
}
RT_RET RTNodeVFilterFaceAE::open(RTTaskNodeContext *context)
{
    RtMetaData* inputMeta   = context->options();
    RT_RET err              = RT_OK;

    //RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_WIDTH, &mSrcWidth));
    //RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_HEIGHT, &mSrcHeight));
    RT_ASSERT(inputMeta->findInt32("opt_evbias", &mEvbias));

    mFaceAeInfo.face_src_width = mSrcWidth;
    mFaceAeInfo.face_src_height = mSrcHeight;
    //2K以上sensor建议使用1280x720数据,低于2K使用640x360
    mFaceAeInfo.face_npu_width = 640;
    mFaceAeInfo.face_npu_height = 360;
    //V2远距离建议0.4，V3近距离建议0.6
    mFaceAeInfo.face_facedetect_score_shold = 0.40;
    mFaceAeInfo.face_zoom_speed = 1;
    mFaceAeInfo.face_fast_move_frame_judge = 5;
    RT_LOGD("lly faceae_info src_wh [%d %d] ,mEvbias =%d \n",
            mFaceAeInfo.face_src_width, mFaceAeInfo.face_src_height,mEvbias);
    if (mEvbias >= 0)
    {
        doIspProcess(NULL, mEvbias);
    }
    return RT_OK;
}

RT_RET RTNodeVFilterFaceAE::process(RTTaskNodeContext *context)
{
    RT_RET         err       = RT_OK;
    RTMediaBuffer *srcBuffer = RT_NULL;
    RTMediaBuffer *dstBuffer = RT_NULL;

    // 此处是上级NN人脸检测节点输出人脸区域信息，SDK默认数据流路径是scale1->NN->EPTZ
    if (context->hasInputStream("image:rect"))
    {
        INT32 count = context->inputQueueSize("image:rect");
        if (count == 0)
        {
            FaceAeAiData faceae_ai_data;
            faceae_ai_data.face_count = 0;
            faceAE(&faceae_ai_data,2);
        }
        while (count)
        {
            dstBuffer = context->dequeInputBuffer("image:rect");
            if (dstBuffer == RT_NULL)
                continue;

            count--;

            void* result = getAIDetectResults(dstBuffer);
            RTRknnAnalysisResults *nnResult  = reinterpret_cast<RTRknnAnalysisResults *>(result);
            if (nnResult != RT_NULL)
            {
                RTRect result;
                INT32 faceCount = nnResult->counter;
                RT_RET ret = RT_OK;
                FaceAeAiData faceae_ai_data;
                faceae_ai_data.face_data = (FaceData *)malloc(faceCount * sizeof(FaceData));
                faceae_ai_data.face_count = faceCount;
                if (faceae_ai_data.face_data)
                {
                    for (int i = 0; i<faceCount; i++)
                    {
                        faceae_ai_data.face_data[i].left = nnResult->results[i].face_info.object.box.left;
                        faceae_ai_data.face_data[i].top = nnResult->results[i].face_info.object.box.top;
                        faceae_ai_data.face_data[i].right = nnResult->results[i].face_info.object.box.right;
                        faceae_ai_data.face_data[i].bottom = nnResult->results[i].face_info.object.box.bottom;
                        faceae_ai_data.face_data[i].score = nnResult->results[i].face_info.object.score;
                    }
                }
                faceAE(&faceae_ai_data,2);
                if (faceae_ai_data.face_data)
                    free(faceae_ai_data.face_data);
            }
            dstBuffer->release();
        }
    }
    else
    {
        RT_LOGE("don't has nn data stream!");
    }

    // 此处是用于预览的原始YUV数据，SDK默认数据流路径是bypass->EPTZ->RGA(输出给下级节点裁剪)
    INT32 count = context->inputQueueSize("image:nv12");
    while (count)
    {
        srcBuffer = context->dequeInputBuffer("image:nv12");
        if (srcBuffer == RT_NULL)
            continue;
        count--;
        INT32 streamId = context->getInputInfo()->streamId();
        RtMetaData *extraInfo = srcBuffer->getMetaData();
        extraInfo->findInt32(kKeyVCodecWidth, &mSrcWidth);
        extraInfo->findInt32(kKeyVCodecHeight, &mSrcHeight);

        extraInfo->findInt32(OPT_FILTER_DST_VIR_WIDTH, &mVirWidth);
        extraInfo->findInt32(OPT_FILTER_DST_VIR_HEIGHT, &mVirHeight);
        srcBuffer->release();
    }
    return err;
}
RT_RET RTNodeVFilterFaceAE::invokeInternal(RtMetaData *meta)
{
    const char *command;
    int enable = 0;
    if ((RT_NULL == meta))
    {
        return RT_ERR_NULL_PTR;
    }

    RtMutex::RtAutolock autoLock(mLock);
    meta->findCString(kKeyPipeInvokeCmd, &command);
    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command)
    {
        RTSTRING_CASE("set_faceae_config"):
            if (meta->findInt32("enable", &enable))
        {
            if (enable == 0)
            {
                doIspProcess(",",0);
            }
        }
        break;
    default:
        RT_LOGD("unsupported command=%d", command);
        break;
    }
    return RT_OK;
}

RT_RET RTNodeVFilterFaceAE::close(RTTaskNodeContext *context)
{
    RT_RET err = RT_OK;

    return err;
}

static RTTaskNode* createFaceAeFilter()
{
    return new RTNodeVFilterFaceAE();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_faceae_demo
{
    .mUid          = kStubRockitFaceAEDemo,
    .mName         = "faceae",
    .mVersion      = "v1.0",
    .mCreateObj    = createFaceAeFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC,  {RT_NULL, RT_NULL} },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_faceae_demo);

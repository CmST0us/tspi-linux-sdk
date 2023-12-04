// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>
#include <sys/time.h>

#include "st_task_handler.h"
#include "logger/log.h"
#include "parse/ai_results_parse.h"
#include "RTAIDetectResults.h"

#define MAX_KEEP_FACE_COUNT     24
#define FACE_FIRST_SEND_DELAY   8
#define FACE_SAME_SEND_DELAY    30

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "STTaskHandler"

namespace rockchip {
namespace aiserver {

static std::map<int32_t, MattingFaceHolder *> mFaceMap;

static int64_t getNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

STTaskHandler::STTaskHandler() {
    mShmNNcontroller = new ShmNNController();
}

STTaskHandler::~STTaskHandler() {
    if (mShmNNcontroller != nullptr) {
        delete mShmNNcontroller;
        mShmNNcontroller = nullptr;
    }

    std::lock_guard<std::mutex> lock(mOpMutex);
    std::map<int32_t, MattingFaceHolder *>::iterator it;
    for (it = mFaceMap.begin(); it != mFaceMap.end();) {
        MattingFaceHolder *holder = it->second;
        it = mFaceMap.erase(it);
        LOG_INFO("clear matting face[%d]\n", holder->faceID);
        freeMattingFaceInfo(holder);
    }
}

int32_t STTaskHandler::processAIData(RTMediaBuffer *buffer) {
    if (buffer == NULL)
        return 0;

    void *nnResult = getAIDetectResults(buffer);
    if (nnResult != NULL) {
        postNNData(nnResult);
    } else {
        LOG_INFO("processAIData not find ai results\n");
    }
    buffer->release();

    return 0;
}

int32_t STTaskHandler::processAIMatting(RTMediaBuffer *buffer) {
    RtMetaData *extraInfo = RT_NULL;
    void       *nnResult  = RT_NULL;

    extraInfo = buffer->getMetaData();
    if (RT_NULL != extraInfo) {
        extraInfo->findPointer("aimatting_out_result", &nnResult);
        if (RT_NULL != nnResult) {
            void* imgData = buffer->getData();
            postAIMattingData(nnResult, imgData);
        }
    }
    buffer->release();

    return 0;
}

int32_t STTaskHandler::processAIFeature(RTMediaBuffer *buffer) {
    RtMetaData *extraInfo = RT_NULL;
    void       *nnResult  = RT_NULL;
    const char *uuid      = RT_NULL;
    INT32       retryAI   = 0;
    INT32       hasFeature = 0;

    extraInfo = buffer->getMetaData();
    if (RT_NULL != extraInfo) {
        if (!extraInfo->findCString("stream_uuid", &uuid)) {
            uuid = "feature_extract";
        }

        nnResult = getAIDetectResults(buffer);
        if (NULL == nnResult) {
            LOG_INFO("ststst processAIFeature not find ai results\n");
            goto __FAILED;
        }

        auto aiResult = (STDetectResult *)(nnResult);
        LOG_INFO("anlyse result(face:%d body:%d hand=%d)\n", \
                aiResult->faceCount, aiResult->bodyCount, aiResult->handCount);

        for (INT32 idx = 0; idx < aiResult->faceCount; idx++) {
            if(aiResult->faces[idx].featureLen > 0) {
                hasFeature = 1;
            }
        }
        if (hasFeature) {
            extraInfo->setInt32("ai_retry", retryAI);
            postFeatureData(nnResult, uuid);
        } else {
            if (!extraInfo->findInt32("ai_retry", &retryAI)) {
                retryAI = 8;
            }

            if (retryAI > 0) {
                retryAI--;
                extraInfo->setInt32("ai_retry", retryAI);
                LOG_INFO("retry to recognize image(buf=%p,cnt=%d)\n", buffer, retryAI);
                return -1;
            }
        }
    }

__FAILED:
    LOG_INFO("aifeature callback(buf=%p,uuid=%s)\n", buffer, uuid);
    if (hasFeature == 0) {
        postEmptyFeatureData(uuid);
    }
    buffer->release();

    return 0;
}

void STTaskHandler::postNNData(void *nnResult) {
    std::lock_guard<std::mutex> lock(mOpMutex);
    KKMessage kkMsg;
    KKAIData *mNNData = kkMsg.mutable_kkaidata();
    auto stRes = (STDetectResult *)(nnResult);
    if (stRes->faceCount <= 0 && stRes->handCount <= 0 && stRes->bodyCount <= 0) {
        return;
    }
    pushDetectInfo(mNNData, stRes, 1);
    mNNData->set_width(1280);
    mNNData->set_height(720);
    mNNData->set_function("KK_AI_DATA");
    kkMsg.set_msg_type(1);
    kkMsg.set_msg_name("KK_AI_DATA");

    std::string sendbuf;
    kkMsg.SerializeToString(&sendbuf);
    mShmNNcontroller->send(sendbuf);
}

void STTaskHandler::pushDetectInfo(KKAIData *mNNData, STDetectResult *detectResult, int32_t size) {
    int32_t i,j;
    FaceData *facedetect;

    LOG_DEBUG("bodycnt=%d handcnt=%d\n", detectResult->bodyCount, detectResult->handCount);
    if (detectResult->faceCount > 0) {
        for (i = 0;i<detectResult->faceCount;i++) {
            facedetect = mNNData->add_facedata();
            facedetect->set_id(detectResult->faces[i].id);

            RectF *rectf = facedetect->mutable_rect();
            rectf->set_left(detectResult->faces[i].rect.left);
            rectf->set_right(detectResult->faces[i].rect.right);
            rectf->set_top(detectResult->faces[i].rect.top);
            rectf->set_bottom(detectResult->faces[i].rect.bottom);

            AngleF *anglef = facedetect->mutable_angle();
            anglef->set_pitch(detectResult->faces[i].pitch);
            anglef->set_yaw(detectResult->faces[i].yaw);
            anglef->set_roll(detectResult->faces[i].roll);

            facedetect->set_quality(detectResult->faces[i].quality);

            for (j = 0; j < detectResult->faces[i].pointCount; j++) {
                PointF *mPoint = facedetect->add_points();

                mPoint->set_x(detectResult->faces[i].points[j].x);
                mPoint->set_y(detectResult->faces[i].points[j].y);
                //Visibilities *visibilities = facedetect->add_visibilities();
                //visibilities->set_visibilities(detectResult->faces[i].visibilities[j]);
            }

            facedetect->set_distance(detectResult->faces[i].distance);

            for (j = 0;j< detectResult->faces[i].attributeCount;j++) {
                Attr *attributes = facedetect->add_attrs();

                //attributes->set_category(detectResult->faces[i].attributes[j].category);
                attributes->set_name(detectResult->faces[i].attributes[j].category);
                attributes->set_value(detectResult->faces[i].attributes[j].label);
                attributes->set_score(detectResult->faces[i].attributes[j].score);
            }

            if(detectResult->faces[i].feature != nullptr) {
                Feature *feature = facedetect->mutable_feature();
                feature->set_data((char *)detectResult->faces[i].feature, detectResult->faces[i].featureLen);
                feature->set_length(detectResult->faces[i].featureLen);
            }
        }
    }

    if (detectResult->handCount > 0) {
        HandData *handdetect;
        for (i = 0; i<detectResult->handCount; i++) {
            handdetect = mNNData->add_handdata();
            handdetect->set_id(detectResult->hands[i].id);
            handdetect->set_faceid(detectResult->hands[i].faceId);
            handdetect->set_quality(1.0);
            RectF *rectf = handdetect->mutable_rect();
            rectf->set_left(detectResult->hands[i].rect.left);
            rectf->set_top(detectResult->hands[i].rect.top);
            rectf->set_right(detectResult->hands[i].rect.right);
            rectf->set_bottom(detectResult->hands[i].rect.bottom);

            AngleF *anglef = handdetect->mutable_angle();
            anglef->set_pitch(0);
            anglef->set_yaw(0);
            anglef->set_roll(0);
            for (j = 0; j < detectResult->hands[i].pointCount; j++) {
              PointF *hPoint = handdetect->add_points();
              hPoint->set_x(detectResult->hands[i].points[j].x);
              hPoint->set_y(detectResult->hands[i].points[j].y);
            }

            Attr *attributes = handdetect->add_attrs();
            attributes->set_name("action");
            switch(detectResult->hands[i].action) {
                case 0x00000200:
                attributes->set_value("0");
                break;
                case 0x00000008:
                attributes->set_value("1");
                break;
                case 0x00000001:
                attributes->set_value("2");
                break;
                case 0x00001000:
                attributes->set_value("3");
                break;
                case 0x00000080:
                attributes->set_value("4");
                break;
                case 0x00004000:
                attributes->set_value("5");
                break;
                case 0x00000100:
                attributes->set_value("6");
                break;
                case 0x00000002:
                attributes->set_value("7");
                break;
                case 0x00000040:
                attributes->set_value("8");
                break;
                case 0x00000010:
                attributes->set_value("9");
                break;
                case 0x00000020:
                attributes->set_value("10");
                break;
                case 0x00000400:
                attributes->set_value("11");
                break;
                case 0x00000800:
                attributes->set_value("12");
                break;
                case 0x00000004:
                attributes->set_value("13");
                break;
                case 0x00002000:
                attributes->set_value("14");
                break;
                default:
                attributes->set_value("999");
                break;
            }
        //handdetect->set_action(detectResult->hands[i].action);
        //handdetect->set_event(detectResult->hands[i].event);
        }
    }

    if(detectResult->bodyCount > 0) {
        BodyData *bodydetect;
        for (i = 0; i<detectResult->bodyCount; i++) {
            bodydetect = mNNData->add_bodydata();
            bodydetect->set_id(detectResult->bodys[i].id);
            bodydetect->set_faceid(detectResult->bodys[i].faceId);
            RectF *rectf = bodydetect->mutable_rect();
            rectf->set_left(detectResult->bodys[i].rect.left);
            rectf->set_top(detectResult->bodys[i].rect.top);
            rectf->set_right(detectResult->bodys[i].rect.right);
            rectf->set_bottom(detectResult->bodys[i].rect.bottom);
            for (j = 0; j < detectResult->bodys[i].pointCount; j++){
                PointF *bPoint = bodydetect->add_points();
                bPoint->set_x(detectResult->bodys[i].points[j].x);
                bPoint->set_y(detectResult->bodys[i].points[j].y);
                //Visibilities *bvisibilities = bodydetect->add_visibilities();
                //bvisibilities->set_visibilities(detectResult->bodys[i].visibilities[j]);
            }
            bodydetect->set_quality(detectResult->bodys[i].quality);
        }
    }
}

void STTaskHandler::postAIMattingData(void *mattingBuffer, void *imgData) {
    if (!mattingBuffer || !imgData)
        return;

    std::lock_guard<std::mutex> lock(mOpMutex);
    auto mAiMattingResult = (RTKKAIMattingResult *)mattingBuffer;
    int faceCnt = mAiMattingResult->faceCount;
    for (int32_t i = 0; i < faceCnt; i++) {
        RTKKMattingFaceInfo *faceInfo = &mAiMattingResult->faceInfo[i];
        MattingFaceHolder *holder = saveMattingFaceInfo(faceInfo, imgData);
        if (holder == nullptr)
            continue;

        if ((!holder->sended && holder->repeatCnt > FACE_FIRST_SEND_DELAY) ||
            (holder->sended && holder->repeatCnt > FACE_SAME_SEND_DELAY)) {
            LOG_DEBUG("post matting face[%d] repeat[%d]\n", holder->faceID, holder->repeatCnt);
            holder->sended = 1;
            holder->repeatCnt = 0;
            doPostMattingFace(holder);
        }
    }
}

void STTaskHandler::doPostMattingFace(MattingFaceHolder *holder) {
    RTKKMattingFaceInfo* faceInfo = holder->faceInfo;
    void* imgData = holder->faceData;

    KKMessage kkMsg;
    ImageClip* imgClip = kkMsg.mutable_imgclip();
    imgClip->set_data(imgData, faceInfo->dataSize);
    imgClip->set_height(faceInfo->width);
    imgClip->set_width(faceInfo->height);
    imgClip->set_format(faceInfo->format);
    imgClip->set_angle(faceInfo->angle);
    imgClip->set_mirror(faceInfo->mirror);
    imgClip->set_imgid(faceInfo->faceID);
    kkMsg.set_msg_type(2);
    kkMsg.set_msg_name("KK_AI_IMAGE_CLIP");
    std::string sendbuf;
    kkMsg.SerializeToString(&sendbuf);
    mShmNNcontroller->send(sendbuf);
}

void STTaskHandler::freeMattingFaceInfo(MattingFaceHolder* holder) {
    if (!holder)
        return;

    if (holder->faceInfo != nullptr) {
        if (holder->faceInfo->feature) {
            free(holder->faceInfo->feature);
            holder->faceInfo->feature = nullptr;
        }
        free(holder->faceInfo);
        holder->faceInfo = nullptr;
    }

    if (holder->faceData != nullptr) {
        free(holder->faceData);
        holder->faceData = nullptr;
    }

    free(holder);
}

MattingFaceHolder* STTaskHandler::saveMattingFaceInfo(RTKKMattingFaceInfo *faceInfo, void *imgData) {
    MattingFaceHolder* faceHolder = nullptr;

    std::map<int32_t, MattingFaceHolder *>::iterator faceIter = mFaceMap.find(faceInfo->faceID);
    if (faceIter != mFaceMap.end()) {
        faceHolder = faceIter->second;
    }

    if (faceHolder != nullptr && faceInfo->featureLen > 0 && faceInfo->dataSize > 0) {
        LOG_INFO("remove for update matting face[%d]\n", faceHolder->faceID);
        mFaceMap.erase(faceHolder->faceID);
        freeMattingFaceInfo(faceHolder);
        faceHolder = nullptr;
    }

    if (faceHolder == nullptr) {
        if (faceInfo->featureLen <= 0 || faceInfo->dataSize <= 0) {
            LOG_DEBUG("empty face[%d] feature\n", faceInfo->faceID);
            return nullptr;
        }

        // remove oldest face if over max size
        if (mFaceMap.size() >= MAX_KEEP_FACE_COUNT) {
            int64_t recTime = getNowUs();
            MattingFaceHolder *oldHolder = nullptr;
            std::map<int32_t, MattingFaceHolder *>::iterator it;
            for (it = mFaceMap.begin(); it != mFaceMap.end();) {
                MattingFaceHolder *holder = it->second;
                if (holder->recTime < recTime) {
                    recTime = holder->recTime;
                    oldHolder = holder;
                }
                it++;
            }

            if (oldHolder != nullptr) {
                LOG_DEBUG("remove oldest matting face[%d] repeat[%d]\n", oldHolder->faceID, oldHolder->repeatCnt);
                mFaceMap.erase(oldHolder->faceID);
                freeMattingFaceInfo(oldHolder);
            } else {
                LOG_ERROR("error record time!!!\n");
            }
        }

        faceHolder = (MattingFaceHolder *)malloc(sizeof(MattingFaceHolder));
        memset(faceHolder, 0, sizeof(MattingFaceHolder));
        faceHolder->recTime = getNowUs();
        faceHolder->faceID = faceInfo->faceID;
        faceHolder->dataSize = faceInfo->dataSize;
        faceHolder->faceData = (unsigned char*)malloc(faceInfo->dataSize);
        faceHolder->faceInfo = (RTKKMattingFaceInfo *)malloc(sizeof(RTKKMattingFaceInfo));
        memcpy(faceHolder->faceData, imgData, faceInfo->dataSize);
        memcpy(faceHolder->faceInfo, faceInfo, sizeof(RTKKMattingFaceInfo));
        faceHolder->faceInfo->feature = (unsigned char*)malloc(faceInfo->featureLen);
        memcpy(faceHolder->faceInfo->feature, faceInfo->feature, faceInfo->featureLen);
        mFaceMap[faceInfo->faceID] = faceHolder;
        LOG_DEBUG("save matting face[%d] size[%d]\n", faceHolder->faceID, mFaceMap.size());
    } else {
        faceHolder->recTime = getNowUs();
        faceHolder->repeatCnt += 1;
    }

    return faceHolder;
}

void STTaskHandler::postFeatureData(void* nnResult, const char* uuid) {
    std::lock_guard<std::mutex> lock(mOpMutex);
    KKMessage kkMsg;
    KKAIData *mNNData = kkMsg.mutable_kkaidata();
    auto st_result = (STDetectResult *)(nnResult);
    pushDetectInfo(mNNData, st_result, 1);
    mNNData->set_width(1280);
    mNNData->set_height(720);
    mNNData->set_index(0);
    mNNData->set_function("ANALYSE");
    mNNData->set_uuid(uuid);
    kkMsg.set_msg_type(4);
    kkMsg.set_msg_name("ANALYSE");

    std::string sendBuf;
    kkMsg.SerializeToString(&sendBuf);
    mShmNNcontroller->send(sendBuf);
}

void STTaskHandler::postEmptyFeatureData(const char* uuid) {
    STDetectResult nnResult;
    memset(&nnResult, 0, sizeof(STDetectResult));
    postFeatureData(&nnResult, uuid);
}

int32_t STTaskHandler::convertDetectType(int32_t detectType) {
    detectType = (detectType <= 0) ? 30: detectType;
    int32_t st_type = 0;
    if (detectType&(0x2)) {
        // face detection ( type = 2)
        st_type = st_type | ST_DETECT_FACE;
    }
    if (detectType&(0x4)) {
        // face attribute ( type = 4), face detection is a precondition
        st_type = st_type | ST_DETECT_FACE | ST_DETECT_FACE_ATTRIBUTE;
    }
    if (detectType&(0x8)) {
        // face keypoint  ( type = 8), equals face detection
        st_type = st_type | ST_DETECT_FACE;
    }
    if (detectType&(0x10)) {
        // face keypoint  ( type = 16), face feature is a precondition
        st_type = st_type | ST_DETECT_FACE | ST_DETECT_FACE_FEATURE;
    }
    LOG_INFO("ai detection type = %x\n", st_type);
    return st_type;
}

} // namespace aiserver
} // namespace rockchip

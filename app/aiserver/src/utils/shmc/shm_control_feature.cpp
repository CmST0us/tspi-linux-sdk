
#ifndef SHM_QUEUE_TRANSCEIER_H_
#define SHM_QUEUE_TRANSCEIER_H_

#include <shmc/shm_queue.h>
#include "shm_control_feature.h"
#include "logger/log.h"

#define MAX_IMAGE_SIZE (1920*1088*2)
const  char *kShmFeatureKey = "0x100ff";
static int   mShmReadOK   = 0;
static int   mShmWriteOK  = 0;
static shmc::ShmQueue<shmc::SVIPC>  mShmQueue;
static shmc::ShmQueue<shmc::SVIPC> *mShmReadQueue = nullptr;

pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

void shm_queue_send_buffer(std::string buffer) {
    shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
        LOG_INFO("[%d] %s\n", lv, s);
    });
    if (mShmWriteOK == 0) {
        mShmWriteOK = mShmQueue.InitForWrite(kShmFeatureKey, MAX_IMAGE_SIZE);
    }
    if (mShmWriteOK > 0) {
        pthread_mutex_lock(&gMutex);
        mShmQueue.Push(buffer);
        pthread_mutex_unlock(&gMutex);
    }
}

void shm_queue_recv_buffer(std::string *buffer) {
    bool hasRetry = false;
    shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
        LOG_INFO("[%d] %s\n", lv, s);
    });
RETRY:
    if (mShmReadQueue == nullptr) {
        mShmReadQueue = new shmc::ShmQueue<shmc::SVIPC>();
    }
    if (mShmReadOK == 0) {
        mShmReadOK = mShmReadQueue->InitForRead(kShmFeatureKey);
    }
    if (mShmReadOK > 0) {
        pthread_mutex_lock(&gMutex);
        mShmReadQueue->Pop(buffer);
        pthread_mutex_unlock(&gMutex);
        if (buffer->length() == 0 && hasRetry == false) {
            hasRetry = true;
            delete mShmReadQueue;
            mShmReadQueue = nullptr;
            mShmReadOK = 0;
            LOG_ERROR("shm_queue_recv_buffer RETRY %s", kShmFeatureKey);
            goto RETRY;
        }
    } else {
        LOG_ERROR("shm_queue_recv_buffer read error");
    }
}

#endif

#include <iostream>
#include "update.h"
#include "deviceInfo.h"
#include "log.h"
#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#include "download.h"
#include "tomcat.h"
#include "rkimage.h"
#include "partition.h"
#include "rkboot_control.h"

double processvalue = 0;
static char * _url = NULL;
/*
static pthread_t a_thread;
static void *thread_function(void *arg){
    *((int*)arg) = -1;
    int res;
    res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if(res != 0){
        LOGE("Thread pthread_setcancelstate failed.\n");
        exit(EXIT_FAILURE);
    }
    //res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    if(res != 0){
        LOGE("Thread pthread_setcanceltype failed");
        exit(EXIT_FAILURE);
    }
    LOGI("thread_function is running\n");
    res = getDataFromUrl(_url);
    if(res == 0){
        *((int*)arg) = 0;
    }
    LOGI("thread_function is stop\n");
    pthread_exit(0);
}*/

void RK_ota_set_url(char *url){
    if(url == NULL){
        LOGE("RK_ota_set_url : url is NULL.\n");
        return ;

    }
    _url = url;
}

void RK_ota_start(RK_upgrade_callback cb){
    LOGI("start RK_ota_start.\n");
    cb(NULL, RK_UPGRADE_START);
    if(_url == NULL){
        LOGE("url is NULL\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }
#if 0
    if(getDataFromUrl(_url) != 0){
        LOGE("getDataFromUrl failed.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }
#endif

    download_file(_url, "/tmp/update.img");
    if(!writeImageToPartition("/tmp/update.img")){
        LOGE("writeImageToPartition failed.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }
    if(setSlotActivity() != 0){
        LOGE("setSlotActivity error.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }
    //CheckImageFile_2(_url);
    //RKIMAGE_HDR hdr;
    //CheckImageFile(_url, &hdr);

/*
    int res;
    int update_result = 0;
    void *thread_result;

    res = pthread_create(&a_thread, NULL, thread_function, &update_result);
    if(res != 0){
        LOGE("Thread creation failed.");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }

    LOGI("waiting for thread to finish...\n");

    res = pthread_join(a_thread, &thread_result);
    if(update_result != 0){
        LOGE("update_result return failed.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }
    if(res != 0){
        LOGE("Thread join failed.\n");
        cb(NULL, RK_UPGRADE_ERR);
        exit(EXIT_FAILURE);
    }
*/
    LOGI("RK_ota_start is ok!");
    cb(NULL, RK_UPGRADE_FINISHED);
}

void RK_ota_stop(){
    int res;
    LOGI("start RK_ota_stop.\n");
	/*
    res = pthread_cancel(a_thread);
    if(res != 0){
        LOGE("Thread cancelation failed");
        exit(EXIT_FAILURE);
    }*/
}


int RK_ota_get_progress(){
#if 0
    return showProgressValue();
#endif
    return processvalue;
}

void RK_ota_get_sw_version(char *buffer, int maxLength){
    DeviceInfoInternel info;
    info.setCurrentConfig();

    std::string ver = info.getCurrentVersion();
    if(ver.size() <= maxLength){
        memcpy(buffer, ver.c_str(), ver.size());
    }
}

bool RK_ota_check_version(const char *url){
    DeviceInfoInternel info;
    info.setHost(url);
    info.setCurrentConfig();
    info.setTargetConfig();
    return info.compareVersion();
}

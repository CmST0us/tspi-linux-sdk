#include <iostream>
#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include "data.h"
#include "partition.h"
#include "log.h"
#include <string.h>
#include <unistd.h>
#include "tomcat.h"
//#include "bootcontrol.h"
#include "rkboot_control.h"

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
    //((std::string*)stream)->append((char*)ptr, 0, size* count);
    if(size * count <= 512)
        std::cout<<"append size * count "<<size * count<<std::endl;
    struct ImageData data;
    struct ImageData *pData = &data;
    static unsigned int currentOffset;
    pData->data = (char*)malloc(size * count);
    pData->offset = currentOffset;
    pData->size = size * count;
    //LOGI("pData->offset = %u\n", currentOffset);
    //LOGI("pData->size = %u\n", pData->size);
    memcpy(pData->data, ptr, count * size);
    int res = writeDataToPartition(pData);
    currentOffset += size * count;
    free(pData->data);
    //pthread_testcancel();
    if(res != 0){
        return -1;
    }
    return size * count;
}

RK_showprogress_callback cb = NULL;

double progressValue = 0;

int showProgressValue(){
    return progressValue;
}

int getProgressValue(void* clientp, double dltotal, double dlnow, double ulttotal, double ulnow){
    progressValue = dlnow / dltotal * 100;
    return 0;
}

int getDataFromUrl(char *url)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        //std::string data;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &return_val);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        //curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, getProgressValue);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, getProgressValue);
        //curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, "flag");


        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        // data now holds response

        // write curl response to string variable..
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            LOGE("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        //std::cout << data << std::endl;
        //std::cout << data.length() << std::endl;
        /*
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLength);
        if(!res){
            LOGE("downloadFileLength = %0.f\n", downloadFileLength);
        }else{
            LOGE("downloadFileLength error \n");
        }*/

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    if(res != CURLE_OK){
        LOGE("update Error.\n");
        return -1;
    }else{
        LOGE("update ok.\n");
        setSlotActivity();
    }
    return 0;
}

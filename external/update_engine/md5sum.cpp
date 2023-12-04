/*************************************************************************
	> File Name: md5sum.cpp
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Thu 07 Mar 2019 03:06:21 PM CST
 ************************************************************************/

#include <openssl/md5.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "log.h"
bool checkdata(const char *dest_path, unsigned char *out_md5sum, long long offset, long long checkSize){
    MD5_CTX ctx;
    unsigned char md5sum[16];
    char buffer[512];
    int len = 0;

    FILE *fp = fopen(dest_path, "rb");

    if(fp == NULL){
        LOGE("open file failed %s", dest_path);
        return false;
    }

    fseek(fp, offset, SEEK_SET);

    MD5_Init(&ctx);

    long long readSize = 0;
    LOGE("hkh checkSize = %lld.\n", checkSize);
    #if 0
    while((len = fread(buffer, 1, 512, fp)) > 0){
        readSize += len;
        if(readSize > checkSize){
            LOGE("len = %d.\n", len);
            len = (len - (readSize-checkSize));
            LOGE("len = %d.\n", len);
        }

        MD5_Update(&ctx, buffer, len);
        memset(buffer, 0, sizeof(buffer));
    }
    #endif
    int step = 512;
    while(checkSize > 0){
        readSize = checkSize > step ? step: checkSize;
        if(fread(buffer, 1, readSize, fp) != readSize){
            LOGE("fread error.\n");
            return false;
        }
        checkSize = checkSize - readSize;
        MD5_Update(&ctx, buffer, readSize);
        memset(buffer, 0, sizeof(buffer));
    }
    MD5_Final(md5sum, &ctx);
    fclose(fp);

    printf("\n");
    printf("new md5:");
    for(int i = 0; i < 16; i++){
        printf("%02x", md5sum[i]);
    }
    //change
    if(out_md5sum != NULL){
        memset(out_md5sum, 0, 16);
        memcpy(out_md5sum, md5sum, 16);
    }
    #if 0
    printf("\n");
    printf("old md5:");
    for(int i = 0; i < 32; i++){
        printf("%02x", source_md5sum[i]);
    }
    printf("\n");

        for(int i = 0; i < 16; i++){
            if(md5sum[i] != source_md5sum[i]){
                LOGE("MD5Check is error of %s", dest_path);
                return false;
            }
        }
    #endif
    LOGI("MD5Check is ok of %s", dest_path);
    return true;
}

bool comparefile(const char *dest_path, const char *source_path, long long dest_offset, long long source_offset, long long checkSize){
    unsigned char md5sum_source[16];
    unsigned char md5sum_dest[16];
    checkdata(dest_path, md5sum_dest, dest_offset, checkSize);
    checkdata(source_path, md5sum_source, source_offset, checkSize);
    for(int i = 0; i < 16; i++){
        if(md5sum_dest[i] != md5sum_source[i]){
            LOGE("MD5Check is error of %s", dest_path);
            return false;
        }
    }
    return true;
}

bool compareMd5sum(const char *dest_path, unsigned char *source_md5sum, long long offset, long long checkSize){
    unsigned char md5sum[16];

    checkdata(dest_path, md5sum, offset, checkSize);

    unsigned char tmp[16][2] = {
        0x30, 0x00,
        0x31, 0x01,
        0x32, 0x02,
        0x33, 0x03,
        0x34, 0x04,
        0x35, 0x05,
        0x36, 0x06,
        0x37, 0x07,
        0x38, 0x08,
        0x39, 0x09,
        0x61, 0x0a,
        0x62, 0x0b,
        0x63, 0x0c,
        0x64, 0x0d,
        0x65, 0x0e,
        0x66, 0x0f,
    };
    for(int i = 0; i < 32; i = i+2){
        for(int j = 0; j < 16; j++){
            if(tmp[j][1] == (md5sum[i/2] >> 4)){
                if(source_md5sum[i] != tmp[j][0]){
                    LOGE("MD5Check is error of %s", dest_path);
                    return false;
                }
            }
            if(tmp[j][1] == (md5sum[i/2] & 0x0f)){
                if(source_md5sum[i+1] != tmp[j][0]){
                    LOGE("MD5Check is error of %s", dest_path);
                    return false;
                }
            }
        }
    }
    return true;
}

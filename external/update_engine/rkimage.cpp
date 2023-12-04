/*************************************************************************
	> File Name: rkimage.cpp
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Tue 30 Oct 2018 09:56:15 AM CST
 ************************************************************************/
#include <stdio.h>
#include "rkimage.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"
#include <errno.h>
#include "data.h"
#include "partition.h"

#include "md5sum.h"

#define DEBUG_READ_IMAGE 0

void display_RKIMAGE_ITEM(RKIMAGE_ITEM *item){
    //char name[PART_NAME];
    //char file[RELATIVE_PATH];
    //unsigned int offset;
    //unsigned int flash_offset;
    //unsigned int usespace;
    //unsigned int size;

    LOGD("name = %s", item->name);
    LOGD("file = %s", item->file);
    LOGD("offset = %d", item->offset);
    LOGD("flash_offset = %d", item->flash_offset);
    LOGD("usespace = %d", item->usespace);
    LOGD("size = %d", item->size);
}

void display_RKIMAGE_HDR(RKIMAGE_HDR* hdr){
    //unsigned int tag;
    //unsigned int size;
    //char machine_model[MAX_MACHINE_MODEL];
    //char manufacturer[MAX_MANUFACTURER];
    //unsigned int version;
    //int item_count;
    //RKIMAGE_ITEM item[MAX_PACKAGE_FILES];

    LOGD("tag = %d", hdr->tag);
    LOGD("size = %d", hdr->size);
    LOGD("machine_model = %s", hdr->machine_model);
    LOGD("manufacturer = %s", hdr->manufacturer);
    LOGD("version = %d", hdr->version);
    for(int i = 0; i < hdr->item_count; i++){
        LOGI("================================================");
        display_RKIMAGE_ITEM(&(hdr->item[i]));
    }
}

void adjustFileOffset(RKIMAGE_HDR* hdr, int offset)
{
    for(int i = 0; i< hdr->item_count; i++)
        hdr->item[i].offset += offset;
    return;
}

unsigned int gFwOffset = 0;

int CheckImageFile(const char* path, RKIMAGE_HDR* hdr){

    /* Try to open the image.
     */
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open %s\n", path);
        return -1;
    }

    /* Need to read the documents before 512 bytes,
     * to determine whether the new way of packing update.
     * If not be, according to the way to handle before then
     * If the new packaging mode, the firmware of the offset each file to adjust accordingly
     *
     */
    gFwOffset = 0;
    char buf[512] = "";
    unsigned int fwSize = 0;
    unsigned int fileSize = 0;
    int m_signMd5Size;
    unsigned char m_md5[32];

    if(read(fd, buf, 512) != 512)
    {
        LOGE("Can't read %s\n(%s)\n", path, strerror(errno));
        close(fd);
        return -2;
    }

    // Confirm whether the new packaging format
    if(*((unsigned int*)buf) == 0x57464B52)
    {
        gFwOffset = *(unsigned int*)(buf+0x21);
        fwSize = *(unsigned int *)(buf + 0x25);
    }

    fileSize = lseek(fd, 0L, SEEK_END);
    LOGE("fileSize is %d.\n", fileSize);

    if(lseek(fd, -32, SEEK_END) == -1){
        LOGE("lseek failed.\n");
        return -2;
    }

    if(read(fd, m_md5, 32) != 32){
        LOGE("Can't read %s\n(%s)\n", path, strerror(errno));
        close(fd);
        return -2;
    }

    if(lseek(fd, gFwOffset, SEEK_SET) == -1){
        LOGE("lseek failed.\n");
        return -2;
    }

    if(read(fd, (char*)hdr, sizeof(RKIMAGE_HDR)) != sizeof(RKIMAGE_HDR)){
        LOGE("Can't read %s\n(%s)\n", path, strerror(errno));
        close(fd);
        return -2;
    }
    close(fd);

    if(hdr->tag != RKIMAGE_TAG)
    {
        LOGI("tag: %x\n", hdr->tag);
        LOGE("Invalid image\n");
        return -3;
    }

    if(!compareMd5sum((char*)path, m_md5, 0, fileSize-32)){
        LOGE("Md5Check update.img fwSize:%ld", fileSize-32);
        return -1;
    }

    if(gFwOffset)
        adjustFileOffset(hdr, gFwOffset);

    display_RKIMAGE_HDR(hdr);
    return 0;
}

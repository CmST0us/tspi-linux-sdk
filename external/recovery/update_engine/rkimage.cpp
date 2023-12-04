/*************************************************************************
    > File Name: rkimage.cpp
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Fri 17 May 2019 02:31:53 PM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "rkimage.h"
#include "log.h"
#include "md5sum.h"
#include "rktools.h"
#include "update.h"

static void display_head(PSTRUCT_RKIMAGE_HEAD pHead){
    LOGD("uiTag = %x.\n", pHead->uiTag);
    LOGD("usSize = %x.\n", pHead->usSize);
    LOGD("dwVersion = %x.\n", pHead->dwVersion);
    UINT btMajor = ((pHead->dwVersion) & 0XFF000000) >> 24;
    UINT btMinor = ((pHead->dwVersion) & 0X00FF0000) >> 16;
    UINT usSmall = ((pHead->dwVersion) & 0x0000FFFF);
    LOGD("btMajor = %x, btMinor = %x, usSmall = %02x.\n", btMajor, btMinor, usSmall);
    LOGD("dwBootOffset = %x.\n", pHead->dwBootOffset);
    LOGD("dwBootSize = %x.\n", pHead->dwBootSize);
    LOGD("dwFWOffset = %x.\n", pHead->dwFWOffset);
    LOGD("dwFWSize = %x.\n", pHead->dwFWSize);
}

static void display_item(PRKIMAGE_ITEM pitem) {
    //char name[PART_NAME];
    //char file[RELATIVE_PATH];
    //unsigned int offset;
    //unsigned int flash_offset;
    //unsigned int usespace;
    //unsigned int size;

    LOGD("name = %s", pitem->name);
    LOGD("file = %s", pitem->file);
    LOGD("offset = %d", pitem->offset);
    LOGD("flash_offset = %d", pitem->flash_offset);
    LOGD("usespace = %d", pitem->usespace);
    LOGD("size = %d", pitem->size);
}

static void display_hdr(PRKIMAGE_HDR phdr) {

    //unsigned int tag;
    //unsigned int size;
    //char machine_model[MAX_MACHINE_MODEL];
    //char manufacturer[MAX_MANUFACTURER];
    //unsigned int version;
    //int item_count;
    //RKIMAGE_ITEM item[MAX_PACKAGE_FILES];

    LOGD("tag = %d", phdr->tag);
    LOGD("size = %d", phdr->size);
    LOGD("machine_model = %s", phdr->machine_model);
    LOGD("manufacturer = %s", phdr->manufacturer);
    LOGD("version = %d", phdr->version);
    LOGD("item = %d.\n", phdr->item_count);
    for(int i = 0; i < phdr->item_count; i++){
        LOGI("================================================");
        display_item(&(phdr->item[i]));
    }
}

void adjustFileOffset(PRKIMAGE_HDR phdr, int offset, int loader_offset, int loader_size)
{
    for(int i = 0; i< phdr->item_count; i++){
        if( strcmp(phdr->item[i].name, "bootloader") == 0){
            phdr->item[i].offset = loader_offset;
            phdr->item[i].size = loader_size;
            continue ;
        }
        phdr->item[i].offset += offset;
    }
}

//解析固件，获得固件头部信息
int analyticImage(const char *filepath, PRKIMAGE_HDR phdr) {
    long long ulFwSize;
    STRUCT_RKIMAGE_HEAD rkimage_head;
    unsigned char m_md5[32];


    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open %s\n", filepath);
        return -2;
    }

    //1. image 头部信息读取
    if (read(fd, &rkimage_head, sizeof(STRUCT_RKIMAGE_HEAD)) != sizeof(STRUCT_RKIMAGE_HEAD)) {
        LOGE("Can't read %s\n(%s)\n", filepath, strerror(errno));
        close(fd);
        return -2;
    }

    if ((rkimage_head.reserved[14]=='H')&&(rkimage_head.reserved[15]=='I')) {
        ulFwSize = *((DWORD *)(&rkimage_head.reserved[16]));
        ulFwSize <<= 32;
        ulFwSize += rkimage_head.dwFWOffset;
        ulFwSize += rkimage_head.dwFWSize;
    } else {
        ulFwSize = rkimage_head.dwFWOffset + rkimage_head.dwFWSize;
    }
    rkimage_head.dwFWSize = ulFwSize - rkimage_head.dwFWOffset;
    display_head(&rkimage_head);

    //2. 固件md5 校验
    long long fileSize;
    int nMd5DataSize;

    fileSize = lseek64(fd, 0L, SEEK_END);
    nMd5DataSize = fileSize - ulFwSize;
    if (nMd5DataSize >= 160) {
        LOGE("md5 : not support sign image.\n");
        //sign image
        //m_bSignFlag = true;
        //m_signMd5Size = nMd5DataSize-32;
        //fseeko64(m_pFile,ulFwSize,SEEK_SET);
        //fread(m_md5,1,32,m_pFile);
        //fread(m_signMd5,1,nMd5DataSize-32,m_pFile);
    } else {
        lseek64(fd, -32, SEEK_END);
        if ( read(fd, m_md5, 32) != 32) {
            LOGE("lseek failed.\n");
            close(fd);
            return -2;
        }
    }

    //3. image 地址信息读取
    if (lseek64(fd, rkimage_head.dwFWOffset, SEEK_SET) == -1) {
        LOGE("lseek failed.\n");
        close(fd);
        return -2;
    }

    if (read(fd, phdr, sizeof(RKIMAGE_HDR)) != sizeof(RKIMAGE_HDR)) {
        LOGE("Can't read %s\n(%s)\n", filepath, strerror(errno));
        close(fd);
        return -2;
    }

    if (phdr->tag != RKIMAGE_TAG) {
        LOGE("tag: %x\n", phdr->tag);
        LOGE("Invalid image\n");
        close(fd);
        return -3;
    }

    if ((phdr->manufacturer[56]==0x55)&&(phdr->manufacturer[57]==0x66))
    {
        USHORT *pItemRemain;
        pItemRemain = (USHORT *)(&phdr->manufacturer[58]);
        phdr->item_count += *pItemRemain;
    }

    if (rkimage_head.dwFWOffset) {
        adjustFileOffset(phdr, rkimage_head.dwFWOffset, rkimage_head.dwBootOffset, rkimage_head.dwBootSize);
    }

    display_hdr(phdr);

    close(fd);
    #if 1
    if (!compareMd5sum((char*)filepath, m_md5, 0, fileSize-32)) {
        LOGE("Md5Check update.img fwSize:%ld", fileSize-32);
        return -1;
    }
    #endif
    LOGI("analyticImage ok.\n");
    return 0;
}

// 获得Image 打包版本号
bool getImageVersion(const char *filepath, char *version, int maxLength) {
    STRUCT_RKIMAGE_HEAD rkimage_head;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open %s\n", filepath);
        return false;
    }

    //1. image 头部信息读取
    if (read(fd, &rkimage_head, sizeof(STRUCT_RKIMAGE_HEAD)) != sizeof(STRUCT_RKIMAGE_HEAD)) {
        LOGE("Can't read %s\n(%s)\n", filepath, strerror(errno));
        close(fd);
        return false;
    }
    close(fd);
    UINT btMajor = ((rkimage_head.dwVersion) & 0XFF000000) >> 24;
    UINT btMinor = ((rkimage_head.dwVersion) & 0x00FF0000) >> 16;
    UINT usSmall = ((rkimage_head.dwVersion) & 0x0000FFFF);

    //转换成字符串
    sprintf(version, "%d.%d.%d", btMajor, btMinor, usSmall);

    return true;
}

#if 0
int main(int argc, char *argv[]){
    analyticImage(argv[1]);
    compareVersion();
}
#endif

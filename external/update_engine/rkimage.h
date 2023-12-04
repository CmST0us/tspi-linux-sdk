/*************************************************************************
	> File Name: rkimage.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Tue 30 Oct 2018 09:53:49 AM CST
 ************************************************************************/

#ifndef _RKIMAGE_H
#define _RKIMAGE_H


#define SHA_DIGEST_SIZE 20
#define PART_NAME 32
#define RELATIVE_PATH 64
#define MAX_PARTS 20
#define MAX_MACHINE_MODEL 64
#define MAX_MANUFACTURER 60
#define MAX_PACKAGE_FILES 16
#define RKIMAGE_TAG 0x46414B52
#define  IMAGE_RESERVED_SIZE 61

typedef struct tagRKIMAGE_ITEM
{
    char name[PART_NAME];
    char file[RELATIVE_PATH];
    unsigned int offset;
    unsigned int flash_offset;
    unsigned int usespace;
    unsigned int size;
}RKIMAGE_ITEM;


typedef struct tagRKIMAGE_HDR
{
    unsigned int tag;
    unsigned int size;
    char machine_model[MAX_MACHINE_MODEL];
    char manufacturer[MAX_MANUFACTURER];
    unsigned int version;
    int item_count;
    RKIMAGE_ITEM item[MAX_PACKAGE_FILES];
}RKIMAGE_HDR;

void display_RKIMAGE_HDR(RKIMAGE_HDR* hdr);
void adjustFileOffset(RKIMAGE_HDR* hdr, int offset);
int CheckImageFile(const char* path, RKIMAGE_HDR* hdr);
#endif

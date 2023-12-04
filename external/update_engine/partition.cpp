#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include "data.h"
#include <openssl/md5.h>
#include "rkimage.h"

#include <map>
#include <string>
#include "md5sum.h"
using namespace std;

extern double processvalue;
int getNowSlot(){
    char cmdline[1024];
    int fd = open("/proc/cmdline", O_RDONLY);
    read(fd, (char*)cmdline, 1024);
    close(fd);
    //LOGI("cmdline = %s", cmdline);
    char *slot = strstr(cmdline, "androidboot.slot_suffix");
    if(slot != NULL){
        slot = strstr(slot, "=");
        if(slot != NULL){
            slot +=2;
            if((*slot) == 'a'){
                LOGI("boot from slot_a.\n");
                return SLOT_A;
            }else if((*slot) == 'b'){
                LOGI("boot from slot_b.\n");
                return SLOT_B;
            }

        }
    }
    LOGE("unknow boot from where");
    return SLOT_UNKNOW;
}

bool getPartitionType(char* name, char *dest_path){
    map<string, string> Partition = {
        map<string, string>::value_type("boot_a",   "/dev/block/by-name/boot_a"),
        map<string, string>::value_type("boot_b",   "/dev/block/by-name/boot_b"),
        map<string, string>::value_type("rootfs_a", "/dev/block/by-name/system_a"),
        map<string, string>::value_type("rootfs_b", "/dev/block/by-name/system_b"),
        map<string, string>::value_type("vbmeta_a",    "/dev/block/by-name/vbmeta_a"),
        map<string, string>::value_type("vbmeta_b",    "/dev/block/by-name/vbmeta_b"),
    };

    //LOGI("item->name is %s", name);
    char tmp_name[120];
    strcpy(tmp_name, name);
    map<string, string>::iterator it;

    switch(getNowSlot()){
        case SLOT_A:
            strcat(tmp_name, "_b");
            break;
        case SLOT_B:
            strcat(tmp_name, "_a");
            break;
        default:
            return false;
            break;
    }
    it = Partition.find(tmp_name);

    if(it == Partition.end()){
        LOGE("no found");
        dest_path = NULL;
        return false;
    }

    LOGI("getPartitionType is %s", it->second.c_str());
    strcpy(dest_path, it->second.c_str());
    return true;
}

void RKWrite(char *filename, char *data, unsigned int len){
    //getfilename
    static char last_filename[128];
    static int fd;
    if(strcmp(last_filename, filename) != 0){
        fsync(fd);
        close(fd);
        fd = -1;
    }

    if(fd == -1){
        char des_name[120];
        if(!getPartitionType(filename, des_name)){
            LOGE("%s no need write.\n", filename);
            return ;
        }
//        fd = open(filename, O_RDWR | O_CREAT, 0644);
        fd = open(des_name, O_RDWR | O_CREAT, 0644);
        strcpy(last_filename, filename);
        LOGI("open filename is %s\n", filename);
    }
    if(fd == -1){
        LOGE("open %s failed.\n", filename);
        return ;
    }
    write(fd, data, len);
    strcpy(last_filename, filename);
}

int writeDataToPartition(struct ImageData *data){
    /* Need to read the documents before 512 bytes,
    * to determine whether the new way of packing update.
    * If not be, according to the way to handle before then
    * If the new packaging mode, the firmware of the offset each file to adjust accordingly
    *
    */
    static unsigned int currentOffset;
    static unsigned RKIMAGE_HDR_READ;
    unsigned int pos = 0;
    static unsigned int gFwOffset;
    static unsigned int fwSize;

    //1. IMAGE_HEADER
    if(data->offset < 512){
        static char headTmp[512];
        if(data->offset + data->size < 512){
            memcpy(headTmp + currentOffset, data->data, data->size);
            currentOffset = data->offset + data->size;
        }else{
            memcpy(headTmp + currentOffset, data->data, 512-data->offset);
            currentOffset = 512;
        }
        if(currentOffset == 512){
            /*
            if(data->size < 512){
                LOGE("OTA file is error.\n");
                return -1;
            }*/

            // Confirm whether the new packaging format
            if( *((unsigned int*)headTmp) == 0x57464B52 )
            {
                gFwOffset = *(unsigned int*)(headTmp + 0x21);
                fwSize = *(unsigned int *)(headTmp + 0x25);
            }
            currentOffset = 512;
            pos = 512;
            LOGI("gFwOffset = %d, fwSize = %d\n", gFwOffset, fwSize);
            if(fwSize <= 0){
                LOGE("OTA file is error.\n");
                return -1;
            }
        }
    }

    //2. BOOT_DATA
    //<=gFwOffset
    int RKIMAGE_HDR_Len = sizeof(RKIMAGE_HDR);
    static RKIMAGE_HDR hdr;
    static char hdrTmp[2000];
    static bool flag;
    if(data->offset + data->size < gFwOffset){
        return 0;
    }else if (gFwOffset != 0){
        if(RKIMAGE_HDR_READ < RKIMAGE_HDR_Len){
            unsigned int len_boot = data->offset + data->size;
            if(len_boot >= gFwOffset && data->offset <= gFwOffset){
                if(data->offset+data->size-gFwOffset >= RKIMAGE_HDR_Len){
                    memcpy(hdrTmp, data->data+(gFwOffset-data->offset), RKIMAGE_HDR_Len);
                    RKIMAGE_HDR_READ += RKIMAGE_HDR_Len;
                }else{
                    memcpy(hdrTmp, data->data+(gFwOffset-data->offset), data->offset+data->size-gFwOffset);
                    RKIMAGE_HDR_READ += data->offset+data->size-gFwOffset;
                }
            }else if(data->offset > gFwOffset){
                if(data->size <= (RKIMAGE_HDR_Len - RKIMAGE_HDR_READ)){
                    memcpy(hdrTmp + RKIMAGE_HDR_READ, data->data, data->size);
                    RKIMAGE_HDR_READ += data->size;
                }else{
                    memcpy(hdrTmp + RKIMAGE_HDR_READ, data->data, RKIMAGE_HDR_Len-RKIMAGE_HDR_READ);
                    RKIMAGE_HDR_READ += RKIMAGE_HDR_Len - RKIMAGE_HDR_READ;
                }
            }
        }
        if(RKIMAGE_HDR_READ == RKIMAGE_HDR_Len){
            if(!flag){
                memcpy(&hdr, hdrTmp, RKIMAGE_HDR_Len);
                //display_RKIMAGE_HDR(&hdr);
                if(hdr.tag != RKIMAGE_TAG){
                    LOGE("tag is error\n");
                    return -1;
                }
                if(gFwOffset){
                    adjustFileOffset(&hdr, gFwOffset);
                    display_RKIMAGE_HDR(&hdr);
                }
                flag = true;
            }

            //find Item
            RKIMAGE_HDR *pRKImage = &hdr;
            unsigned int write_offset;
            unsigned int write_len;
            for(int i = 0; i < pRKImage->item_count; i++){
                if(data->offset < pRKImage->item[i].offset && data->offset + data->size > pRKImage->item[i].offset){
                    write_offset = pRKImage->item[i].offset - data->offset;
                    if(data->offset + data->size <= pRKImage->item[i].offset + pRKImage->item[i].size){
                        write_len = data->offset + data->size - pRKImage->item[i].offset;
                        RKWrite(pRKImage->item[i].name, data->data + write_offset, write_len);
                        return 0;
                    }else{
                        write_len = pRKImage->item[i].size;
                        RKWrite(pRKImage->item[i].name, data->data + write_offset, write_len);
                        struct ImageData tmp;
                        tmp.size = data->offset + data->size - (pRKImage->item[i].offset + pRKImage->item[i].size);
                        tmp.offset = pRKImage->item[i].offset + pRKImage->item[i].size;
                        tmp.data = data->data + (pRKImage->item[i].offset + pRKImage->item[i].size - data->offset);
                        writeDataToPartition(&tmp);
                        return 0;
                    }
                }else if(data->offset >= pRKImage->item[i].offset && data->offset < pRKImage->item[i].offset + pRKImage->item[i].size){
                    if(data->offset + data->size <= pRKImage->item[i].offset + pRKImage->item[i].size){
                        write_len = data->size;
                        RKWrite(pRKImage->item[i].name, data->data, write_len);
                        return 0;
                    }else{
                        write_len = pRKImage->item[i].offset + pRKImage->item[i].size - data->offset;
                        RKWrite(pRKImage->item[i].name, data->data, write_len);
                        struct ImageData tmp;
                        tmp.size = data->offset + data->size - pRKImage->item[i].size - pRKImage->item[i].offset;
                        tmp.offset = pRKImage->item[i].offset + pRKImage->item[i].size;
                        tmp.data = data->data + write_len;
                        writeDataToPartition(&tmp);
                        return 0;
                    }
                }
            }
        }
    }


    return 0;
    //3. FIRMWARE_DATA

    //4. MD5 check
}
bool writeImageToPartition(const char* path){
    RKIMAGE_HDR hdr;
    char data_buf[16*1024] = {0};
    if(CheckImageFile(path, &hdr) != 0){
        LOGE("CheckImageFile filed.\n");
        return false;
    }
    processvalue = 95;
    for(int i = 0; i < hdr.item_count; i++){
        //char name[PART_NAME];
        //char file[RELATIVE_PATH];
        //unsigned int offset;
        //unsigned int flash_offset;
        //unsigned int usespace;
        //unsigned int size;
        char dest_path[256] = {0};
        int fd_src = 0;
        int fd_dest = 0;
        int src_offset, dest_offset;
        int src_step, dest_step;
        int src_remain, dest_remain;
        int src_file_offset = 0;
        dest_offset = 0;

        if(getPartitionType(hdr.item[i].name, dest_path)){
            //write
            LOGI("write image of %s to %s.\n", hdr.item[i].name, dest_path);
            fd_src = open(path, O_RDONLY);
            if (fd_src < 0) {
                LOGE("Can't open %s\n", path);
                return false;
            }
            src_offset = hdr.item[i].offset;
            dest_remain = src_remain = hdr.item[i].size;
            dest_step = src_step = 16*1024;

            lseek(fd_src, src_offset, SEEK_SET);
            src_file_offset = src_offset;

            //???
            fd_dest = open(dest_path, O_RDWR | O_CREAT, 0644);
            if(fd_dest < 0){
                LOGE("Can't open %s\n", path);
                return false;
            }
            lseek(fd_dest, dest_offset, SEEK_SET);

            int read_count, write_count;
            while(src_remain > 0 && dest_remain > 0){
                memset(data_buf, 0, 16*1024);
                read_count = src_remain>src_step?src_step:src_remain;

                if(read(fd_src, data_buf, read_count) != read_count){
                    close(fd_src);
                    close(fd_dest);
                    LOGE("Read failed(%s)\n", strerror(errno));
                    return false;
                }

                src_remain -= read_count;
                src_file_offset += read_count;

                write_count = (src_remain == 0)?dest_remain:dest_step;

                if(write(fd_dest, data_buf, dest_step) != dest_step){
                    close(fd_src);
                    close(fd_dest);
                    LOGE("Write failed(%s)\n", strerror(errno));
                    return false;
                }
                dest_remain -= write_count;
            }
            fsync(fd_dest);
            close(fd_src);
            close(fd_dest);
            //check
            LOGI("check image of %s to %s.\n", hdr.item[i].name, dest_path);
            if(!comparefile(dest_path, path, 0, src_offset, hdr.item[i].size)){
                LOGE("check image of %s to %s. failed.\n", hdr.item[i].name, dest_path);
                return false;
            }
        }
    }
    processvalue = 100;
    return true;

}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "rktools.h"
#include "common.h"

/**
 * 从/proc/cmdline 获取串口的节点
 *
*/
char *getSerial(){
    char *ans = (char*)malloc(20);
    char param[1024];
    int fd, ret;
    char *s = NULL;
    fd = open("/proc/cmdline", O_RDONLY);
    ret = read(fd, (char*)param, 1024);
    printf("cmdline=%s\n",param);
    s = strstr(param,"console");
    if(s == NULL){
        printf("no found console in cmdline\n");
        free(ans);
        ans = NULL;
        return ans;
    }else{
        s = strstr(s, "=");
        if(s == NULL){
            free(ans);
            ans = NULL;
            return ans;
        }

        strcpy(ans, "/dev/");
        char *str = ans + 5;
        s++;
        while(*s != ' '){
            *str = *s;
            str++;
            s++;
        }
        *str = '\0';
        printf("read console from cmdline is %s\n", ans);
    }

    return ans;
}

/**
 *  设置flash 节点
 */
static char result_point[4][20]={'\0'}; //0-->emmc, 1-->sdcard, 2-->SDIO, 3-->SDcombo
int readFile(DIR* dir, char* filename){
    char name[30] = {'\0'};
    int i;

    strcpy(name, filename);
    strcat(name, "/type");
    int fd = openat(dirfd(dir), name, O_RDONLY);
    if(fd == -1){
        printf("Error: openat %s error %s.\n", name, strerror(errno));
        return -1;
    }
    char resultBuf[10] = {'\0'};
    read(fd, resultBuf, sizeof(resultBuf));
    for(i = 0; i < strlen(resultBuf); i++){
        if(resultBuf[i] == '\n'){
            resultBuf[i] = '\0';
            break;
        }
    }
    for(i = 0; i < 4; i++){
        if(strcmp(typeName[i], resultBuf) == 0){
            //printf("type is %s.\n", typeName[i]);
            return i;
        }
    }

    printf("Error:no found type!\n");
    return -1;
}

void init_sd_emmc_point(){
    DIR* dir = opendir("/sys/bus/mmc/devices/");
    if(dir != NULL){
        struct dirent* de;
        while((de = readdir(dir))){
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 )
                continue;
            //if (de->d_type == 4)    //dir
            //    printf("dir name : %s \n", de->d_name);

            if (strncmp(de->d_name, "mmc", 3) == 0){
                //printf("find mmc is %s.\n", de->d_name);
                char flag = de->d_name[3];
                int ret = -1;
                ret = readFile(dir, de->d_name);
                if(ret != -1){
                    strcpy(result_point[ret], point_items[flag - '0']);
                }else{
                    strcpy(result_point[ret], "");
                }
            }
        }
    }
    closedir(dir);
}

static void wait_for_device(const char* fn) {
    int tries = 0;
    int ret;
    struct stat buf;
    do {
        ++tries;
        ret = stat(fn, &buf);
        if (ret) {
            printf("stat %s try %d: %s\n", fn, tries, strerror(errno));
            sleep(1);
        }
    } while (ret && tries < 10);
    if (ret) {
        printf("failed to stat %s\n", fn);
    }
}

void setFlashPoint(){
    if (!isMtdDevice())
        wait_for_device(MISC_PARTITION_NAME_BLOCK);

    init_sd_emmc_point();
    setenv(EMMC_POINT_NAME, result_point[MMC], 1);
    //SDcard 有两个挂载点

    if(access(result_point[SD], F_OK) == 0)
        setenv(SD_POINT_NAME_2, result_point[SD], 1);
    char name_t[22];
    if(strlen(result_point[SD]) > 0){
        strcpy(name_t, result_point[SD]);
        strcat(name_t, "p1");
    }
    if(access(name_t, F_OK) == 0)
        setenv(SD_POINT_NAME, name_t, 1);

    printf("emmc_point is %s\n", getenv(EMMC_POINT_NAME));
    printf("sd_point is %s\n", getenv(SD_POINT_NAME));
    printf("sd_point_2 is %s\n", getenv(SD_POINT_NAME_2));
}

#define MTD_PATH "/proc/mtd"
//判断是MTD还是block 设备
bool isMtdDevice() {
    char param[2048];
    int fd, ret;
    char *s = NULL;
    fd = open("/proc/cmdline", O_RDONLY);
    ret = read(fd, (char*)param, 2048);
    close(fd);
    s = strstr(param,"storagemedia");
    if(s == NULL){
        printf("no found storagemedia in cmdline, default is not MTD.\n");
        return false;
    }else{
        s = strstr(s, "=");
        if (s == NULL) {
            printf("no found storagemedia in cmdline, default is not MTD.\n");
            return -1;
        }

        s++;
        while (*s == ' ') {
            s++;
        }

        if (strncmp(s, "mtd", 3) == 0 ) {
            printf("Now is MTD.\n");
            return true;
        } else if (strncmp(s, "sd", 2) == 0) {
            printf("Now is SD.\n");
            if ( !access(MTD_PATH, F_OK) ) {
                fd = open(MTD_PATH, O_RDONLY);
                ret = read(fd, (char*)param, 2048);
                close(fd);

                s = strstr(param,"mtd");
                if(s == NULL){
                    LOGI("no found mtd.\n");
                    return false;
                }
                LOGI("Now is MTD.\n");
                return true;
            }
        }
    }
    LOGI("devices is not MTD.\n");
    return false;
}

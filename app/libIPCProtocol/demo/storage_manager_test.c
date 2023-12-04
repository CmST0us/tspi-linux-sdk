#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>

#include <pthread.h>

#include "json-c/json.h"
#include "dbserver.h"
#include "storage_manager.h"
#include "IPCProtocol.h"

struct ScanPath {
    char *media_path;
    char *thumb_path;
    int type;
    char *format;
    int count;
    int camid;

    long medio_size;
    long thumb_size;
    pthread_t tid;
    int run;
    struct ScanPath *next;
};

struct StorageConfig {
    int freesize;
    char *mountpath;
    int status;
    struct ScanPath *scanpath;
};

struct StorageConfig storageconfig;

int create_dir(const char *sPathName)
{
    char DirName[256];
    int i;
    strcpy(DirName, sPathName);
    int len = strlen(DirName);

    for (i = 1; i < len; i++) {
        if (DirName[i] == '/') {
            DirName[i] = 0;
            if (access(DirName, R_OK) != 0) {
                if(mkdir(DirName, 0755) == -1) {
                    printf("mkdir error\n");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }
    if (access(DirName, R_OK) != 0) {
        if(mkdir(DirName, 0755) == -1) {
            printf("mkdir error\n");
            return -1;
        }
    }
    return 0;
}

void signal_update_disks_status(void *user_data)
{
    printf("%s, %s\n", __func__, (char *)user_data);
}

void signal_update_media_path(void *user_data)
{
    printf("%s, %s\n", __func__, (char *)user_data);
}

char *creatfilename(char *folder, char *format, int type, int count, int *needcount)
{
    char *filename;
    time_t nSeconds;
    struct tm * pTM;
    char *tmp = strstr(format, "%Y");

    if (tmp) {
        char name[128] = {0};
        time(&nSeconds);
        pTM = localtime(&nSeconds);
        strftime(name, sizeof(name), format, pTM);
        if (type == 0)//video mp4
            filename = g_strdup_printf("%s/%s.mp4", folder, name);
        else if (type == 1)//photo jpg
            filename = g_strdup_printf("%s/%s.jpg", folder, name);
    } else {
        if (type == 0)//video mp4
            filename = g_strdup_printf("%s/%s.mp4", folder, format);
        else if (type == 1)//photo jpg
            filename = g_strdup_printf("%s/%s.jpg", folder, format);
    }

    tmp = strstr(filename, "CCC");
    if (tmp) {
        int cnt = 0;
        int mul = 1;
        while (tmp[cnt] == 'C') {
            cnt++;
            mul *= 10;
        }
        for (int i = 0; i < cnt; i++) {
            tmp[i] = (count % mul) * 10 / mul + '0';
            mul /= 10;
        }
        *needcount = 1;
    }

    printf("filename = %s\n", filename);
    return filename;
}

void creatfile(char *name, long size, int *run)
{
    FILE *fp;
    int buflen = 4 * 1024 * 1024;
    unsigned char *buf = (unsigned char *)malloc(buflen);
    fp = fopen(name, "wb+");
    while (size > 0) {
        if (size > buflen) {
            fwrite(buf, 1, buflen, fp);
            size -= buflen;
        } else {
            fwrite(buf, 1, size, fp);
            break;
        }
    }
    fclose(fp);
    free(buf);
}

static void *creat_file_thread(void *arg)
{
    struct ScanPath *scanpath = (struct ScanPath *)arg;

    create_dir(scanpath->media_path);
    if (scanpath->thumb_path && (!g_str_equal(scanpath->thumb_path, "")))
        create_dir(scanpath->thumb_path);
    while (scanpath->run) {
        int needcount = 0;
        printf("%s\n", __func__);
        int type;
        if (scanpath->type == TYPE_VIDEO)
            type = 0;
        else
            type = 1;

        char *filename = creatfilename(scanpath->media_path, scanpath->format, type, scanpath->count, &needcount);
        creatfile(filename, scanpath->medio_size, &scanpath->run);
        g_free(filename);
        if (scanpath->thumb_path && (!g_str_equal(scanpath->thumb_path, ""))) {
            filename = creatfilename(scanpath->thumb_path, scanpath->format, 1, scanpath->count, &needcount);
            creatfile(filename, scanpath->thumb_size, &scanpath->run);
            g_free(filename);
        }
        if (needcount)
            scanpath->count++;
        if (scanpath->type == 1)
            sleep(10);
        else
            sleep(1);
    }
err_event_monitor:
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

void creat_mediafile_test(void)
{
    struct ScanPath *scanpath = storageconfig.scanpath;

    if (storageconfig.status == 0)
        return;

    while (scanpath) {
        if (scanpath->type == 0)
            scanpath->medio_size = 200 * 1024 * 1024;
        else
            scanpath->medio_size = 4 * 1024 * 1024;
        scanpath->thumb_size = 32 * 1024;
        scanpath->run = 1;
        pthread_create(&scanpath->tid, NULL, creat_file_thread, scanpath);
        scanpath = scanpath->next;
    }
}

void storage_manager_get_disks_status_test(void)
{
    char *json_str = storage_manager_get_disks_status("");
    printf("%s, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void storage_manager_get_filelist_id_test(void)
{
    char *json_str = storage_manager_get_filelist_id(2, 0, 0, "0,20");
    printf("%s, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void storage_manager_get_filelist_path_test(void)
{
    int starttime = 1583975593;
    int endtime = 1583982408;
    char *json_str = storage_manager_get_filelist_path("/userdata/video1", &starttime, &endtime, 0, "0,20");
    printf("%s, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void storage_manager_get_media_path_test(void)
{
    json_object *j_config;
    json_object *j_array = NULL;
    int videolen;
    int videomaxsize;
    char *mountpath;
    char *json_str = storage_manager_get_media_path();

    if (json_str == NULL)
        return;

    printf("%s, %s\n", __func__, json_str);
    memset(&storageconfig, 0, sizeof(struct StorageConfig));

    j_config = json_tokener_parse(json_str);

    storageconfig.status = (int)json_object_get_int(json_object_object_get(j_config, "iStatus"));
    storageconfig.freesize = (int)json_object_get_int(json_object_object_get(j_config, "iFreeSize"));
    storageconfig.mountpath = g_strdup((char *)json_object_get_string(json_object_object_get(j_config, "sMountPath")));

    j_array = json_object_object_get(j_config, "sScanPath");
    if (j_array) {
        int num = json_object_array_length(j_array);
        for (int i = 0; i < num; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);
            struct ScanPath *scanpath = (struct ScanPath *)calloc(1, sizeof(struct ScanPath));
            if (storageconfig.scanpath == NULL) {
                storageconfig.scanpath = scanpath;
            } else {
                struct ScanPath *tmp = storageconfig.scanpath;
                while (tmp->next) {
                    tmp = tmp->next;
                }
                tmp->next = scanpath;
            }
            scanpath->media_path = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sMediaPath")));
            scanpath->thumb_path = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sThumbPath")));
            scanpath->format = g_strdup((char *)json_object_get_string(json_object_object_get(j_obj, "sFormat")));
            scanpath->camid = (int)json_object_get_int(json_object_object_get(j_obj, "iCamId"));
            scanpath->type = (int)json_object_get_int(json_object_object_get(j_obj, "iType"));
            scanpath->count = (int)json_object_get_int(json_object_object_get(j_obj, "iCount"));
        }
    }
    json_object_put(j_config);

    if (json_str)
        g_free(json_str);
}

void storage_manager_diskformat_test(void)
{
    char *json_str = storage_manager_diskformat("/media/usb0", "fat32");

    printf("%s json_str = %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    for (int i = 0; i < 100; i++) {
        json_str = storage_manager_get_disks_status("/media/usb0");
        printf("%s, %s\n", __func__, json_str);
        if (json_str)
            g_free(json_str);
        usleep(100000);
    }
}

void storage_manager_test(void)
{
    dbus_monitor_signal_registered(STORAGEMANAGER_INTERFACE, "UpdateDisksStatus", &signal_update_disks_status);
    storage_manager_get_disks_status_test();
    storage_manager_get_filelist_id_test();
    storage_manager_get_filelist_path_test();
    storage_manager_get_media_path_test();
    //storage_manager_diskformat_test();
    //creat_mediafile_test();
}

int main( int argc , char ** argv)
{
    //IPCProtocol_init();
    //int signal_id = dbus_monitor_signal_registered(STORAGEMANAGER_PATH, STORAGEMANAGER, STORAGEMANAGER_INTERFACE, SM_SIGNAL_UPDATEDISKSTATUS, &signal_update_disks_status);
    storage_manager_test();

    while (1){
        sleep(1);
    }

    //dbus_monitor_signal_unregistered(signal_id);

    return 0;
}
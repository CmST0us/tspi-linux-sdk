#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "rkdb.h"
#include "common.h"

#define TABLE_STORAGE_DISK_PATH     "StorageDiskPath"
#define TABLE_STORAGE_MEDIA_FOLDER  "StorageMediaFolder"
#define TABLE_STORAGE_CONFIG        "StorageConfig"
#define TABLE_STORAGE_VERSION       "StorageVersion"
#define TABLE_STORAGE_PLAN_SNAP     "StoragePlanSnap"
#define TABLE_STORAGE_ADVANCE_PARA  "StorageAdvancePara"

#define TYPE_VIDEO         0
#define TYPE_PHOTO         1
#define TYPE_BLACK_LIST    2
#define TYPE_SNAPSHOT      3
#define TYPE_WHITE_LIST    4

#define STORAGE_VERSION             "1.0.3"

int storage_dbus_register(DBusConnection *dbus_conn)
{
    g_dbus_register_interface(dbus_conn, "/",
                              DB_STORAGE_INTERFACE,
                              methods,
                              signals, NULL, DB_STORAGE_INTERFACE, NULL);

    return 0;
}

void storage_init(void)
{
    char *col_para;

    if (equal_version(TABLE_STORAGE_VERSION, STORAGE_VERSION))
        return;

    g_free(rkdb_drop(TABLE_STORAGE_DISK_PATH));
    g_free(rkdb_drop(TABLE_STORAGE_MEDIA_FOLDER));
    g_free(rkdb_drop(TABLE_STORAGE_CONFIG));
    g_free(rkdb_drop(TABLE_STORAGE_PLAN_SNAP));
    g_free(rkdb_drop(TABLE_STORAGE_ADVANCE_PARA));
    g_free(rkdb_drop(TABLE_STORAGE_VERSION));

    creat_version_table(TABLE_STORAGE_VERSION, STORAGE_VERSION);

    col_para = "id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "sPath TEXT NOT NULL UNIQUE," \
               "sName TEXT DEFAULT ''," \
               "iMount INT DEFAULT 0";

    g_free(rkdb_create(TABLE_STORAGE_DISK_PATH, col_para));
    g_free(rkdb_insert(TABLE_STORAGE_DISK_PATH, "sPath,sName,iMount", "'/mnt/sdcard','SD Card',0"));
    g_free(rkdb_insert(TABLE_STORAGE_DISK_PATH, "sPath,sName,iMount", "'/media/usb0','U Disk',0"));
    g_free(rkdb_insert(TABLE_STORAGE_DISK_PATH, "sPath,sName,iMount", "'/userdata/media','Emmc',0"));

    col_para = "id INTEGER PRIMARY KEY," \
               "sMediaFolder TEXT NOT NULL UNIQUE," \
               "sThumbFolder TEXT," \
               "sFormat TEXT," \
               "iCamId INT," \
               "iType INT," \
               "iDuty INT," \
               "iMaxNum INT";

    g_free(rkdb_create(TABLE_STORAGE_MEDIA_FOLDER, col_para));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "0,'video0','video0/.thumb','VIDEO_%Y%m%d%H%M%S',0,0,45,-1"));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "1,'photo0','photo0/.thumb','PHOTO_%Y%m%d%H%M%S',0,1,5,-1"));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "2,'video1','video1/.thumb','VIDEO_%Y%m%d%H%M%S',1,0,45,-1"));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "3,'photo1','photo1/.thumb','PHOTO_%Y%m%d%H%M%S',1,1,5,-1"));

    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "4,'black_list','','%Y%m%d%H%M%S',0,2,-1,-1"));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "5,'snapshot','','%Y%m%d%H%M%S',0,3,0,-1"));
    g_free(rkdb_insert(TABLE_STORAGE_MEDIA_FOLDER, "id,sMediaFolder,sThumbFolder,sFormat,iCamId,iType,iDuty,iMaxNum",
                                          "6,'white_list','','%Y%m%d%H%M%S',0,4,-1,-1"));

    col_para = "id INTEGER PRIMARY KEY," \
               "iFreeSize INT DEFAULT -1," \
               "iFreeSizeNotice INT," \
               "sMountPath TEXT NOT NULL UNIQUE";
    g_free(rkdb_create(TABLE_STORAGE_CONFIG, col_para));
    //The unit of iFreeSize is MB
    g_free(rkdb_insert(TABLE_STORAGE_CONFIG, "id,iFreeSize,iFreeSizeNotice,sMountPath","0,1024,512,'/userdata/media'"));

    col_para = "id INTEGER PRIMARY KEY," \
               "iEnabled INT DEFAULT 0," \
               "sImageType TEXT," \
               "sResolution TEXT," \
               "iImageQuality INT DEFAULT 10," \
               "iShotInterval INT DEFAULT 1000," \
               "iShotNumber INT DEFAULT 1";
    g_free(rkdb_create(TABLE_STORAGE_PLAN_SNAP, col_para));
    g_free(rkdb_insert(TABLE_STORAGE_PLAN_SNAP, "id,iEnabled,sImageType,sResolution,iImageQuality,iShotInterval,iShotNumber",
                                          "0,0,'JPEG','2688*1520',10,10000,1"));
    g_free(rkdb_insert(TABLE_STORAGE_PLAN_SNAP, "id,iEnabled,sImageType,sResolution,iImageQuality,iShotInterval,iShotNumber",
                                          "1,0,'JPEG','2688*1520',10,1000,4"));

    col_para = "id INTEGER PRIMARY KEY," \
               "iEnabled INT DEFAULT 0";
    g_free(rkdb_create(TABLE_STORAGE_ADVANCE_PARA, col_para));
    g_free(rkdb_insert(TABLE_STORAGE_ADVANCE_PARA, "id","0"));
}

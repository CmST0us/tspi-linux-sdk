/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "database.h"
#include "face_common.h"

#define DATABASE_TABLE "face_data"
#define DATABASE_VERSION "version_0"

static sqlite3 *g_db = NULL;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void database_bak(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cp %s %s", DATABASE_PATH, BAK_DATABASE_PATH);
    system(cmd);
}

int database_init(void)
{
    char *err;
    char cmd[256];

    if (sqlite3_open(DATABASE_PATH, &g_db) != SQLITE_OK) {
        printf("%s open database %s failed!\n", __func__, DATABASE_PATH);
        return -1;
    }
    snprintf(cmd, sizeof(cmd),
             "CREATE TABLE IF NOT EXISTS %s (data blob, name varchar(%d), id INTEGER PRIMARY KEY, mask blob, %s INTEGER)",
             DATABASE_TABLE, NAME_LEN, DATABASE_VERSION);
    if (sqlite3_exec(g_db, cmd, 0, 0, &err) != SQLITE_OK) {
        sqlite3_close(g_db);
        g_db = NULL;
        printf("%s create table %s failed!\n", __func__, DATABASE_TABLE);
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "SELECT %s FROM %s;", DATABASE_VERSION, DATABASE_TABLE);
    if (sqlite3_exec(g_db, cmd, 0, 0, &err) != SQLITE_OK) {
        sqlite3_close(g_db);
        g_db = NULL;
        unlink(DATABASE_PATH);
        printf("%s table %s %s mismatch!\n", __func__, DATABASE_TABLE, DATABASE_VERSION);
        return database_init();
    }

    database_bak();

    return 0;
}

void database_exit(void)
{
    sqlite3_close(g_db);
    database_bak();
}

void database_reset(void)
{
    pthread_mutex_lock(&g_mutex);
    database_exit();
    unlink(DATABASE_PATH);
    database_init();
    pthread_mutex_unlock(&g_mutex);
}

int database_insert(void *data, size_t size, const char *name, size_t n_size, int id, bool sync_flag, void *mask, size_t mask_size)
{
    char cmd[256];
    sqlite3_stmt *stat = NULL;

    if (n_size > NAME_LEN) {
        printf("%s n_size error\n", __func__);
        return -1;
    }
    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "REPLACE INTO %s VALUES(?, '%s', %d, ?, 0);", DATABASE_TABLE, name, id);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }
    sqlite3_exec(g_db, "begin transaction", NULL, NULL, NULL);
    sqlite3_bind_blob(stat, 1, data, size, NULL);
    sqlite3_bind_blob(stat, 2, mask, mask_size, NULL);
    sqlite3_step(stat);
    sqlite3_finalize(stat);
    sqlite3_exec(g_db, "commit transaction", NULL, NULL, NULL);
    if (sync_flag) {
        sync();
        database_bak();
    }
    pthread_mutex_unlock(&g_mutex);

    return 0;
}

int database_record_count(void)
{
    int ret = 0;
    char cmd[256];
    sqlite3_stmt *stat = NULL;

    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "SELECT COUNT(*) FROM %s;", DATABASE_TABLE);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return 0;
    }
    if (sqlite3_step(stat) == SQLITE_ROW)
        ret = sqlite3_column_int(stat, 0);
    sqlite3_finalize(stat);
    pthread_mutex_unlock(&g_mutex);

    return ret;
}

int database_get_data(void *dst, const int cnt, size_t d_size, size_t d_off,
                      size_t i_size, size_t i_off, int mask)
{
    int ret = 0;
    char cmd[256];
    sqlite3_stmt *stat = NULL;
    int index = 0;
    const void *data;
    size_t size;
    int id;
    const size_t sum_size = d_size + i_size;

    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "SELECT * FROM %s;", DATABASE_TABLE);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return 0;
    }
    while (1) {
        ret = sqlite3_step(stat);
        if (ret != SQLITE_ROW)
            break;
        if (mask) {
            data = sqlite3_column_blob(stat, 3);
            size = sqlite3_column_bytes(stat, 3);
        } else {
            data = sqlite3_column_blob(stat, 0);
            size = sqlite3_column_bytes(stat, 0);
        }
        if (size <= d_size)
            memcpy((char*)dst + index * sum_size + d_off, data, size);
        id = sqlite3_column_int(stat, 2);
        memcpy((char*)dst + index * sum_size + i_off, &id, i_size);

        if (++index >= cnt)
            break;
    }
    sqlite3_finalize(stat);
    pthread_mutex_unlock(&g_mutex);

    return index;
}

bool database_is_name_exist(const char *name)
{
    bool exist = false;
    int ret = 0;
    char cmd[256];
    sqlite3_stmt *stat = NULL;

    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "SELECT * FROM %s WHERE name = '%s' LIMIT 1;", DATABASE_TABLE, name);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return false;
    }
    ret = sqlite3_step(stat);
    if (ret == SQLITE_ROW)
        exist = true;
    else
        exist = false;
    sqlite3_finalize(stat);
    pthread_mutex_unlock(&g_mutex);

    return exist;
}

bool database_is_id_exist(int id, char *name, size_t size)
{
    bool exist = false;
    int ret = 0;
    char cmd[256];
    sqlite3_stmt *stat = NULL;

    memset(name, 0, size);
    snprintf(cmd, sizeof(cmd), "SELECT * FROM %s WHERE id = %d LIMIT 1;", DATABASE_TABLE, id);
    pthread_mutex_lock(&g_mutex);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return false;
    }
    ret = sqlite3_step(stat);
    if (ret == SQLITE_ROW) {
        const char *n = sqlite3_column_text(stat, 1);
        size_t s = sqlite3_column_bytes(stat, 1);
        if (s <= size)
            strncpy(name, n, size - 1);
        exist = true;
    } else {
        exist = false;
    }
    sqlite3_finalize(stat);
    pthread_mutex_unlock(&g_mutex);

    return exist;
}

int database_get_user_name_id(void)
{
    int ret = 0;
    char cmd[256];
    sqlite3_stmt *stat = NULL;
    int id = 0;
    int max_id = -1;
    int *save_id = NULL;
    int ret_id = 0;

    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "SELECT * FROM %s where id=(select max(id) from %s);",
             DATABASE_TABLE, DATABASE_TABLE);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }
    while (1) {
        ret = sqlite3_step(stat);
        if (ret != SQLITE_ROW)
            break;
        max_id = sqlite3_column_int(stat, 2);
    }
    sqlite3_finalize(stat);

    if (max_id < 0) {
        pthread_mutex_unlock(&g_mutex);
        return 0;
    }

#if 0
    save_id = calloc(max_id + 1, sizeof(int));
    if (!save_id) {
        printf("%s: memory alloc fail!\n", __func__);
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "SELECT * FROM %s;",
             DATABASE_TABLE);
    if (sqlite3_prepare(g_db, cmd, -1, &stat, 0) != SQLITE_OK) {
        ret_id = -1;
        goto exit;
    }
    while (1) {
        ret = sqlite3_step(stat);
        if (ret != SQLITE_ROW)
            break;
        id = sqlite3_column_int(stat, 2);
        save_id[id] = 1;
    }
    sqlite3_finalize(stat);

    for (int i = 0; i < max_id + 1; i++) {
        if (!save_id[i]) {
            ret_id = i;
            goto exit;
        }
    }
#endif
    ret_id = max_id + 1;

exit:
    if (save_id)
        free(save_id);
    pthread_mutex_unlock(&g_mutex);
    return ret_id;
}

void database_delete(int id, bool sync_flag)
{
    char cmd[256];

    pthread_mutex_lock(&g_mutex);
    snprintf(cmd, sizeof(cmd), "DELETE FROM %s WHERE id = %d;", DATABASE_TABLE, id);
    sqlite3_exec(g_db, "begin transaction", NULL, NULL, NULL);
    sqlite3_exec(g_db, cmd, NULL, NULL, NULL);
    sqlite3_exec(g_db, "commit transaction", NULL, NULL, NULL);
    if (sync_flag)
        sync();
    database_bak();
    pthread_mutex_unlock(&g_mutex);
}

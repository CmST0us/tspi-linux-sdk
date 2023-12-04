#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <glib.h>

#include "json-c/json.h"

static sqlite3 *db;

static int callback(void *param, int argc, char **argv, char **azColName)
{
    int i;
    json_object *j_array = (json_object *)param;
    json_object *j_cfg = json_object_new_object();

    for (i = 0; i < argc; i++) {
        if (azColName[i][0] == 'i') {
            json_object_object_add(j_cfg, azColName[i], json_object_new_int(atoi(argv[i])));
        } else {
            json_object_object_add(j_cfg, azColName[i], json_object_new_string(argv[i] ? argv[i] : ""));
        }
    }
    json_object_array_add(j_array, j_cfg);

    return 0;
}

char *rkdb_sql(char *sql)
{
    int rc;
    char *zErrMsg = 0;
    json_object *j_cfg = json_object_new_object();
    json_object *j_array = json_object_new_array();

    rc = sqlite3_exec(db, sql, callback, (void*)j_array, &zErrMsg);
    json_object_object_add(j_cfg, "iReturn", json_object_new_int(rc));
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", zErrMsg);
        json_object_object_add(j_cfg, "sErrMsg", json_object_new_string(zErrMsg));
        sqlite3_free(zErrMsg);
    } else {
        json_object_object_add(j_cfg, "sErrMsg", json_object_new_string(""));
        //printf("Operation done successfully\n");
    }
    json_object_object_add(j_cfg, "jData", j_array);
    char *ret = g_strdup((char *)json_object_get_string(j_cfg));
    json_object_put(j_cfg);

    return ret;
}

char *rkdb_drop(char *table)
{
    char *sql = g_strdup_printf("DROP TABLE %s;", table);

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

char *rkdb_create(char *table, char *col_para)
{
    char *sql = g_strdup_printf("CREATE TABLE %s(%s);", table, col_para);

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

char *rkdb_insert(char *table, char *cols, char *vals)
{
    char *sql = g_strdup_printf("INSERT INTO %s (%s) VALUES (%s);", table, cols, vals);

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

char *rkdb_update(char *table, char *set, char *where)
{
    char *sql = g_strdup_printf("UPDATE %s SET %s WHERE %s;", table, set, where);

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

char *rkdb_delete(char *table, char *where)
{
    char *sql;

    if (where)
        sql = g_strdup_printf("DELETE from %s WHERE %s;", table, where);
    else
        sql = g_strdup_printf("DELETE from %s;", table);

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

char *rkdb_select(char *table, char *colname, char *where, char *order, char *limit)
{
    char *sql = NULL;
    char *tmp;

    if (colname)
        sql = g_strdup_printf("SELECT %s from %s", colname, table);
    else
        sql = g_strdup_printf("SELECT * from %s", table);

    if (where) {
        tmp = sql;
        sql = g_strdup_printf("%s WHERE %s", tmp, where);
        g_free(tmp);
    }

    if (order) {
        tmp = sql;
        sql = g_strdup_printf("%s ORDER BY %s", tmp, order);
        g_free(tmp);
    }

    if (limit) {
        tmp = sql;
        sql = g_strdup_printf("%s LIMIT %s", tmp, limit);
        g_free(tmp);
    }

    char *ret = rkdb_sql(sql);
    g_free(sql);

    return ret;
}

int rkdb_init(char *file)
{
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_open(file, &db);
    assert(!rc);

    return 0;
}

void rkdb_deinit(void)
{
    if (db)
        sqlite3_close(db);

    db = NULL;
}
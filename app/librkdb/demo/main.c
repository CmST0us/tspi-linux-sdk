#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>
#include "json-c/json.h"
#include "rkdb.h"

#define USERINFO_TABLE    "userinfo"

void create_table(void)
{
    char *col_para = "iID INTEGER PRIMARY KEY AUTOINCREMENT," \
                     "sUserName TEXT NOT NULL UNIQUE," \
                     "sPwd TEXT DEFAULT ''," \
                     "iAge INT DEFAULT 0);";

    g_free(rkdb_create(USERINFO_TABLE, col_para));
}

void insert_data(void)
{
    g_free(rkdb_insert(USERINFO_TABLE, "sUserName,sPwd,iAge", "'hello','123456',20"));
    g_free(rkdb_insert(USERINFO_TABLE, "sUserName,sPwd,iAge", "'abc','223344',16"));
}

void printf_json_array(json_object *j_array)
{
    int i;
    int array_len = json_object_array_length(j_array);
    printf("%s\n", (char *)json_object_get_string(j_array));
    for (i = 0; i < array_len; i++) {
        json_object *j_obj = json_object_array_get_idx(j_array, i);
        json_object_object_foreach(j_obj, key, val) {
            if (json_object_get_type(val) == json_type_int)
                printf("%s = %d\n", key, (int)json_object_get_int(val));
            else
                printf("%s = %s\n", key, (char *)json_object_get_string(val));
        }
    }
}

void select_data(void)
{
    int i;
    {
        char *json_str = rkdb_select(USERINFO_TABLE, NULL, NULL, NULL, NULL);
        json_object *j_ret = json_tokener_parse(json_str);
        json_object *j_array = json_object_object_get(j_ret, "jData");
        printf_json_array(j_array);
        json_object_put(j_ret);
        g_free(json_str);
    }
    {
        char *json_str = rkdb_select(USERINFO_TABLE, "sUserName,iAge", "iAge>18", NULL, NULL);
        json_object *j_ret = json_tokener_parse(json_str);
        json_object *j_array = json_object_object_get(j_ret, "jData");
        printf_json_array(j_array);
        json_object_put(j_ret);
        g_free(json_str);
    }
}

void update_data(void)
{
    g_free(rkdb_update(USERINFO_TABLE, "iAge=21", "sUserName='hello' and sPwd='123456'"));
    g_free(rkdb_update(USERINFO_TABLE, "sPwd='666666',iAge=17", "sUserName='abc'"));
}

void delete_data(void)
{
    g_free(rkdb_delete(USERINFO_TABLE, "sUserName='hello'"));
    g_free(rkdb_delete(USERINFO_TABLE, "iAge=17"));
}

int main( int argc , char ** argv)
{
    rkdb_init("/userdata/rkdbdemo.db");
    create_table();
    insert_data();
    select_data();
    update_data();
    select_data();
    delete_data();

    rkdb_deinit();

    return 0;
}

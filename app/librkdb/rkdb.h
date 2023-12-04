#ifndef __RKDB_H__
#define __RKDB_H__

#ifdef __cplusplus
extern "C" {
#endif

char *rkdb_sql(char *sql);
char *rkdb_drop(char *table);
char *rkdb_create(char *table, char *col_para);
char *rkdb_update(char *table, char *set, char *where);
char *rkdb_insert(char *table, char *cols, char *vals);
char *rkdb_delete(char *table, char *where);
char *rkdb_select(char *table, char *colname, char *where, char *order, char *limit);
int rkdb_init(char *file);
void rkdb_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
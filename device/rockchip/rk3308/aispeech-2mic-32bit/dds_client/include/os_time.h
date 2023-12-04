#ifndef OS_TIME_H
#define OS_TIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef enum {
    OS_TIME_TYPE_CLOCK,
    OS_TIME_TYPE_SECOND,
    OS_TIME_TYPE_MILLISECOND
} os_time_type_t;

typedef struct {
    union {
        struct {
            int year;
            int month;
            int day;
            int hour;
            int minute;
            int second;
            int millisecond;
            int week;
            int year_day;
        } clock;
        size_t second;
        size_t millisecond;
    };  
    os_time_type_t type;
}os_time_t;

int os_time_now(os_time_t *timestamp);

#ifdef __cplusplus
}
#endif
#endif

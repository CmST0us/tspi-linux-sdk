#ifndef __DEBUG_H__
#define __DEBUG_H__


#define PCBA_VERSION             	0x00010300
#define COPYRIGHT                       "PCBA V1.3.0 Copyright (C) 2012 Rockchip"


#define CMD_PIPE_NAME                   "/tmp/cmd_pipe"
#define VOLUME_PIPE_NAME                "/tmp/volume_pipe"
#define CAMERA_PIPE_NAME                "/tmp/camera_pipe"

#define TEST_COMPLETION                 "done"

#define DB_LOG_LEVEL                    4

#if   DB_LOG_LEVEL == 1
#define DB_ERROR
#elif DB_LOG_LEVEL == 2
#define DB_ERROR
#define DB_WARN
#elif DB_LOG_LEVEL == 3
#define DB_ERROR
#define DB_WARN
#define DB_MSG
#elif DB_LOG_LEVEL == 4
#define DB_ERROR
#define DB_WARN
#define DB_MSG
#define DB_DEBUG
#elif DB_LOG_LEVEL == 5
#define DB_ERROR
#define DB_WARN
#define DB_MSG
#define DB_DEBUG
#define DB_DUMP
#endif

#ifdef DB_ERROR
#define db_error(fmt, ...) \
    do { fprintf(stderr, "(error): "); fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#else
#define db_error(fmt, ...)
#endif /* DB_ERROR */

#ifdef DB_WARN
#define db_warn(fmt, ...) \
    do { fprintf(stdout, "(warn): "); fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define db_warn(fmt, ...)
#endif /* DB_WARN */

#ifdef DB_MSG
#define db_msg(fmt, ...) \
    do { fprintf(stdout, "(msg): "); fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define db_msg(fmt, ...)
#endif /* DB_MSG */

#ifdef DB_DEBUG
#define db_debug(fmt, ...) \
    do { fprintf(stdout, "(debug): "); fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define db_debug(fmt, ...)
#endif /* DB_DEBUG */

#ifdef DB_DUMP
#define db_dump(fmt, ...) \
    do { fprintf(stdout, "(dump): "); fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
#else
#define db_dump(fmt, ...)
#endif

#endif 


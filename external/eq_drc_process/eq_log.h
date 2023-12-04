#ifndef __EQ_LOG__
#define __EQ_LOG__
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSLOG_DEBUG

#ifdef SYSLOG_DEBUG
#define eq_debug(fmt, ...)		syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define eq_info(fmt, ...)		syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#define eq_warning(fmt, ...)	syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define eq_err(fmt, ...)		syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#else
#define eq_debug				printf
#define eq_info					printf
#define eq_warning				printf
#define eq_err					printf
#endif

#ifdef __cplusplus
}
#endif

#endif

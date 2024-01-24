#include "trace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#define TRACE_ON

#ifdef TRACE_ON

#ifdef __cplusplus
#define CC_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))
#else
#define CC_LIKELY( exp )    (__builtin_expect( !!(exp), 1 ))
#define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), 0 ))
#endif

#define ATRACE_MESSAGE_LENGTH 1024

static int atrace_marker_fd     = -1;
static int init_ok = 0;

static int trace_init_once(void)
{
    if (init_ok == 0)
    {
         atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        if (atrace_marker_fd == -1) {
            printf("Error opening trace file: %s (%d)", strerror(errno), errno);
            return -1;
        }
    }

    init_ok = 1;
    return 0;
}

#define WRITE_MSG(format_begin, format_end, name, value) { \
    char buf[ATRACE_MESSAGE_LENGTH]; \
    if (CC_UNLIKELY(!init_ok)) \
        trace_init_once(); \
    int pid = getpid(); \
    int len = snprintf(buf, sizeof(buf), format_begin "%s" format_end, pid, \
        name, value); \
    if (len >= (int) sizeof(buf)) { \
        /* Given the sizeof(buf), and all of the current format buffers, \
         * it is impossible for name_len to be < 0 if len >= sizeof(buf). */ \
        int name_len = strlen(name) - (len - sizeof(buf)) - 1; \
        /* Truncate the name to make the message fit. */ \
        printf("Truncated name in %s: %s\n", __FUNCTION__, name); \
        len = snprintf(buf, sizeof(buf), format_begin "%.*s" format_end, pid, \
            name_len, name, value); \
    } \
    if (write(atrace_marker_fd, buf, len) != len) \
        printf("Warning: write failed\n"); \
}

void atrace_begin_body(const char* name)
{
    WRITE_MSG("B|%d|", "%s", name, "");
}

void atrace_end_body()
{
    WRITE_MSG("E|%d", "%s", "", "");
}

void atrace_async_begin_body(const char* name, int32_t cookie)
{
    WRITE_MSG("S|%d|", "|%" PRId32, name, cookie);
}

void atrace_async_end_body(const char* name, int32_t cookie)
{
    WRITE_MSG("F|%d|", "|%" PRId32, name, cookie);
}

void atrace_int_body(const char* name, int32_t value)
{
    WRITE_MSG("C|%d|", "|%" PRId32, name, value);
}

void atrace_int64_body(const char* name, int64_t value)
{
    WRITE_MSG("C|%d|", "|%" PRId64, name, value);
}

#else

void atrace_begin_body(__attribute__((unused)) const char* name)
{
}

void atrace_end_body()
{
}

void atrace_async_begin_body(__attribute__((unused)) const char* name, __attribute__((unused)) int32_t cookie)
{
}

void atrace_async_end_body(__attribute__((unused)) const char* name, __attribute__((unused)) int32_t cookie)
{
}

void atrace_int_body(__attribute__((unused)) const char* name, __attribute__((unused)) int32_t value)
{
}

void atrace_int64_body(__attribute__((unused)) const char* name, __attribute__((unused)) int64_t value)
{
}

#endif

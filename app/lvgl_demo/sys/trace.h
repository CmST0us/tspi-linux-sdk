#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdint.h>

#if defined(__cplusplus)
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

__BEGIN_DECLS

void atrace_begin_body(const char* name);
void atrace_end_body();
void atrace_async_begin_body(const char* name, int32_t cookie);
void atrace_async_end_body(const char* name, int32_t cookie);
void atrace_int_body(const char* name, int32_t value);
void atrace_int64_body(const char* name, int64_t value);

__END_DECLS

#endif

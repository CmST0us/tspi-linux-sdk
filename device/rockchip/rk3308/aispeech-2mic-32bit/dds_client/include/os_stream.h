#ifndef OS_STREAM_H
#define OS_STREAM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct os_stream* os_stream_handle_t;

__attribute ((visibility("default"))) os_stream_handle_t  os_stream_create(size_t size);
__attribute ((visibility("default"))) int os_stream_start(os_stream_handle_t self);
__attribute ((visibility("default"))) int os_stream_read(os_stream_handle_t self, char *data, size_t data_len);
__attribute ((visibility("default"))) int os_stream_read2(os_stream_handle_t self, char *data, size_t data_len); 
__attribute ((visibility("default"))) int os_stream_write(os_stream_handle_t self, const char *data, size_t data_len);
__attribute ((visibility("default"))) int os_stream_write2(os_stream_handle_t self, const char *data, size_t data_len);
__attribute ((visibility("default"))) int os_stream_finish(os_stream_handle_t self);
__attribute ((visibility("default"))) int os_stream_stop(os_stream_handle_t self);
__attribute ((visibility("default"))) int os_stream_stop2(os_stream_handle_t self);
__attribute ((visibility("default"))) int os_stream_reset(os_stream_handle_t self);
__attribute ((visibility("default"))) void os_stream_destroy(os_stream_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

#ifndef OS_QUEUE_H
#define OS_QUEUE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct os_queue* os_queue_handle_t;

__attribute ((visibility("default"))) os_queue_handle_t os_queue_create(size_t item_count, size_t item_size);
__attribute ((visibility("default"))) int os_queue_send(os_queue_handle_t self, const void *data);
__attribute ((visibility("default"))) int os_queue_send_font(os_queue_handle_t self, const void *data);
__attribute ((visibility("default"))) int os_queue_receive(os_queue_handle_t self, void *data);
__attribute ((visibility("default"))) int os_queue_receive_back(os_queue_handle_t self, void *data);
__attribute ((visibility("default"))) int os_queue_stop(os_queue_handle_t self);
__attribute ((visibility("default"))) int os_queue_finish(os_queue_handle_t self);
__attribute ((visibility("default"))) int os_queue_peek(os_queue_handle_t self, void *data);
__attribute ((visibility("default"))) void os_queue_destroy(os_queue_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

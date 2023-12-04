#ifndef OS_EVENT_GROUP_H
#define OS_EVENT_GROUP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct os_event_group* os_event_group_handle_t;

typedef uint64_t os_event_bit_t;

__attribute ((visibility("default"))) os_event_group_handle_t os_event_group_create();
__attribute ((visibility("default"))) os_event_bit_t os_event_group_set_bits(os_event_group_handle_t self, os_event_bit_t bits);
__attribute ((visibility("default"))) os_event_bit_t os_event_group_wait_bits(os_event_group_handle_t self, os_event_bit_t bits, bool all, bool clear);
__attribute ((visibility("default"))) os_event_bit_t os_event_group_clear_bits(os_event_group_handle_t self, os_event_bit_t bits);
__attribute ((visibility("default"))) int os_event_group_reset_bits(os_event_group_handle_t self);
__attribute ((visibility("default"))) void os_event_group_destroy(os_event_group_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

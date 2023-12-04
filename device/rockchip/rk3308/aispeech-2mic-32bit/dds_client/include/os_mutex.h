#ifndef OS_MUTEX_H
#define OS_MUTEX_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_mutex* os_mutex_handle_t;

__attribute ((visibility("default"))) os_mutex_handle_t os_mutex_create();
__attribute ((visibility("default"))) int os_mutex_lock(os_mutex_handle_t self);
__attribute ((visibility("default"))) int os_mutex_unlock(os_mutex_handle_t self);
__attribute ((visibility("default"))) void os_mutex_destroy(os_mutex_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

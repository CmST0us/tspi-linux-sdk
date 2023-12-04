#ifndef OS_SEMAPHORE_H
#define OS_SEMAPHORE_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_semaphore* os_semaphore_handle_t;

__attribute ((visibility("default"))) os_semaphore_handle_t os_semaphore_create();
__attribute ((visibility("default"))) int os_semaphore_take(os_semaphore_handle_t self);
__attribute ((visibility("default"))) int os_semaphore_give(os_semaphore_handle_t self);
__attribute ((visibility("default"))) void os_semaphore_destroy(os_semaphore_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

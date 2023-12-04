#ifndef OS_THREAD_H
#define OS_THREAD_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*run)(void *args);
    void *args;
}os_thread_cfg_t;

typedef struct os_thread* os_thread_handle_t;

__attribute ((visibility("default"))) os_thread_handle_t os_thread_create(os_thread_cfg_t *cfg);
__attribute ((visibility("default"))) void os_thread_exit(os_thread_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

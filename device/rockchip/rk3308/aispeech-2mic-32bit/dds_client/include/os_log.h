#ifndef OS_LOG_H
#define OS_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OS_LOG_LEVEL_DEBUG,
    OS_LOG_LEVEL_INFO,
    OS_LOG_LEVEL_WARNING,
    OS_LOG_LEVEL_ERROR,
}os_log_level_t;

typedef void (*print_module_log_t)(void *self, const char *func, int line, os_log_level_t level, const char *message, ...);

typedef struct {
    const char *module;
    os_log_level_t level;
    print_module_log_t print;
} os_log_block_t;

__attribute ((visibility("default"))) void os_print_module_log(void *self, const char *func, int line, os_log_level_t level, const char *message, ...);

#ifdef OS_DEBUG_LEVEL_NONE
#define os_log_init(file)
#define os_log_deinit()
#define os_log_create_module(module, level)
#define OS_LOG_D(module, message, ...)
#define OS_LOG_I(module, message, ...)
#define OS_LOG_W(module, message, ...)
#define OS_LOG_E(module, message, ...)
#else
__attribute ((visibility("default"))) int os_log_init(const char *file);
__attribute ((visibility("default"))) void os_log_deinit();
#define os_log_create_module(module, level) \
    os_log_block_t os_log_block_##module = \
    {   \
        #module,    \
        (level),    \
        os_print_module_log    \
    }

#define OS_LOG_D(module, message, ...)  \
    do {    \
        extern os_log_block_t os_log_block_##module; \
        os_log_block_##module.print(&os_log_block_##module,    \
                __func__,   \
                __LINE__,   \
                OS_LOG_LEVEL_DEBUG, \
                (message),  \
                ##__VA_ARGS__); \
    } while (0)

#define OS_LOG_I(module, message, ...)  \
    do {    \
        extern os_log_block_t os_log_block_##module; \
        os_log_block_##module.print(&os_log_block_##module,    \
                __func__,   \
                __LINE__,   \
                OS_LOG_LEVEL_INFO, \
                (message),  \
                ##__VA_ARGS__); \
    } while (0)

#define OS_LOG_W(module, message, ...)  \
    do {    \
        extern os_log_block_t os_log_block_##module; \
        os_log_block_##module.print(&os_log_block_##module,    \
                __func__,   \
                __LINE__,   \
                OS_LOG_LEVEL_WARNING, \
                (message),  \
                ##__VA_ARGS__); \
    } while (0)

#define OS_LOG_E(module, message, ...)  \
    do {    \
        extern os_log_block_t os_log_block_##module; \
        os_log_block_##module.print(&os_log_block_##module,    \
                __func__,   \
                __LINE__,   \
                OS_LOG_LEVEL_ERROR, \
                (message),  \
                ##__VA_ARGS__); \
    } while (0)
#endif


#ifdef __cplusplus
}
#endif
#endif

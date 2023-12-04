#ifndef OS_MEMORY_H
#define OS_MEMORY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

__attribute ((visibility("default"))) void *os_malloc(size_t size);
__attribute ((visibility("default"))) void os_free(void *ptr);
__attribute ((visibility("default"))) void *os_calloc(size_t nmemb, size_t size);
__attribute ((visibility("default"))) void *os_realloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif
#endif

#ifndef DUI_H
#define DUI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "dui_msg.h"

typedef void (*user_listen_cb)(dui_msg_t *msg);

int dui_library_init(const char *cfg, user_listen_cb listen);

void dui_start_recorder();
void dui_stop_recorder();

void dui_library_cleanup(void);

#ifdef __cplusplus
}
#endif
#endif

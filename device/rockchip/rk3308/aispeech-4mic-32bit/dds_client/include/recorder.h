#ifndef RECORDER_H
#define RECORDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "alsa_cfg.h"
#include <stdlib.h>

typedef struct recorder_handle* recorder_handle_t;
__attribute ((visibility("default"))) recorder_handle_t recorder_open(alsa_open_config_t *config);
__attribute ((visibility("default"))) int recorder_start(recorder_handle_t self);
//注意alsa的一帧值一个采样时刻所有通道的数据
//计算方法：
//  一帧字节数 = config->bits * config->channels / 8;
//  frame_count必须为config->period_size;
__attribute ((visibility("default"))) int recorder_read(recorder_handle_t self, char *frame, size_t frame_count);
__attribute ((visibility("default"))) int recorder_stop(recorder_handle_t self);
__attribute ((visibility("default"))) void recorder_close(recorder_handle_t self);

#ifdef __cplusplus
}
#endif
#endif

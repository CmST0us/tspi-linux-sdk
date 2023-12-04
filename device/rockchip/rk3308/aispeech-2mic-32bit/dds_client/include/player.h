#ifndef PLAYER_H
#define PLAYER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

__attribute ((visibility("default"))) int player_init();

typedef struct player* player_handle_t;

typedef struct {
    size_t preprocess_buf_size;
    size_t decode_buf_size;
    char *name;
} player_cfg_t;

__attribute ((visibility("default"))) player_handle_t player_create(player_cfg_t *cfg);

typedef enum {
    //本地流
    PLAY_TYPE_NATIVE = 0,
    //网络流
    PLAY_TYPE_NETWORK,
    //应用流
    PLAY_TYPE_APP,
}play_type_t;

typedef struct {
    play_type_t type;
    char *target;
    bool need_free;

    //当播放PCM数据流时需要指定下面三个参数
    int samplerate;
    int bits;
    int channels;
}play_cfg_t;

typedef enum {
    //空闲状态
    PLAYER_STATE_IDLE = 0,
    //运行状态
    PLAYER_STATE_RUNNING,
    //暂停状态
    PLAYER_STATE_PAUSED
}player_state_t;

__attribute ((visibility("default"))) int player_play(player_handle_t self, play_cfg_t *cfg);
__attribute ((visibility("default"))) int player_pause(player_handle_t self);
__attribute ((visibility("default"))) int player_resume(player_handle_t self);
__attribute ((visibility("default"))) int player_stop(player_handle_t self);
__attribute ((visibility("default"))) int player_wait_idle(player_handle_t self);
__attribute ((visibility("default"))) void player_destroy(player_handle_t self);
__attribute ((visibility("default"))) void player_deinit();

#ifdef __cplusplus
}
#endif
#endif

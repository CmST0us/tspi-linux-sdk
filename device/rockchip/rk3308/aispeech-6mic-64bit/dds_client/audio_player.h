#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_PLAYER_EXPORT __attribute ((visibility("default")))

#define AUDIO_PLAYER_EV_BEGIN    0x01
#define AUDIO_PLAYER_EV_START    0x02
#define AUDIO_PLAYER_EV_END      0x03
#define AUDIO_PLAYER_EV_ERROR    0x04
#define AUDIO_PLAYER_EV_PAUSED   0x05
#define AUDIO_PLAYER_EV_PLAYING  0x06
#define AUDIO_PLAYER_EV_STOPPED  0x07

typedef int (*audio_player_callback)(void *userdata, int ev);
typedef struct audio_player audio_player_t;

AUDIO_PLAYER_EXPORT audio_player_t *audio_player_new(audio_player_callback ccb, void *userdata);
AUDIO_PLAYER_EXPORT int audio_player_delete(audio_player_t *aplayer);
AUDIO_PLAYER_EXPORT int audio_player_play(audio_player_t *aplayer, char *path);
AUDIO_PLAYER_EXPORT int audio_player_pause(audio_player_t *aplayer);
AUDIO_PLAYER_EXPORT int audio_player_resume(audio_player_t *aplayer);
AUDIO_PLAYER_EXPORT int audio_player_stop(audio_player_t *aplayer);

AUDIO_PLAYER_EXPORT int audio_player_get_volume(char *dev, int *l_vol, int *r_vol);
AUDIO_PLAYER_EXPORT int audio_player_set_volume(char *dev, int l_vol, int r_vol);

AUDIO_PLAYER_EXPORT int audio_player_set_channel_volume(audio_player_t *aplayer, float multiplier);

AUDIO_PLAYER_EXPORT int audio_player_set_device(audio_player_t *aplayer, char *device);

#ifdef __cplusplus
}
#endif

#endif

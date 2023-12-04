#ifndef ALSA_CFG_H
#define ALSA_CFG_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int samplerate;
    int bits;
    int channels;
    int period_size;
    const char *device;
}alsa_open_config_t;

__attribute ((visibility("default"))) int playback_check_samplerate(int samplerate);
#define capture_check_samplerate(samplerate) playback_check_samplerate(samplerate)

#ifdef __cplusplus
}
#endif
#endif

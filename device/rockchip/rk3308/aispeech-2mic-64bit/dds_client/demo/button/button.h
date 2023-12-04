#ifndef BUTTON_H
#define BUTTON_H
#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    //短按触发
    BUTTON_EVENT_VOLUME_ADD = 0,
    //短按触发
    BUTTON_EVENT_VOLUME_SUB,
    //长按每隔1.5秒触发一次
    BUTTON_EVENT_PREV,
    //长按每隔1.5秒触发一次
    BUTTON_EVENT_NEXT,
    //短按触发
    BUTTON_EVENT_PLAY_PAUSE,
    //长按1.5秒后触发
    BUTTON_EVENT_PLAY_PAUSE_LONG,
    //短按触发
    BUTTON_EVENT_MUTE_UNMUTE,
    //长按1.5秒触发
    BUTTON_EVENT_MUTE_UNMUTE_LONG,
    //长按3秒触发
    BUTTON_EVENT_MODE_WIFI,
    //短按触发
    BUTTON_EVENT_MODE_NORMAL,
    BUTTON_EVENT_MAX
}button_event_t;


typedef void (*button_event_cb)(button_event_t ev, void *userdata);

typedef struct {
    char *dev;
    button_event_cb cb;
    void *userdata;
}button_config_t;

typedef struct button* button_handle_t;

button_handle_t button_create(button_config_t *config);

//默认内部100ms检查一次
int button_run(button_handle_t self);
int button_run2(button_handle_t self, int ms);

void button_destroy(button_handle_t self);


#ifdef __cplusplus
}
#endif
#endif

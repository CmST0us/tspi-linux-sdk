#include "button.h"
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct button{
    pthread_t pid;
    int fd;
    button_event_cb cb;
    void *userdata;
};

//RK3308麦克风采集自带5个按键，按键设备为/dev/input/event1，每个按键的code码如下：

typedef enum {
    //短按代表音量加；长按代表上一首
    PHY_BUTTON_CODE_VOLUME_ADD = 115,

    //短按代表音量减；长按代表下一首
    PHY_BUTTON_CODE_VOLUME_SUB = 114,

    //只支持短按
    PHY_BUTTON_CODE_MUTE = 248,

    //只支持短按
    PHY_BUTTON_CODE_PLAY_PAUSE = 207,

    //长按代表进入配网模式 
    PHY_BUTTON_CODE_MODE = 373
}phy_button_code_t;

typedef struct {
    int val;
    struct timeval last_time;
    bool long_pressed;
}phy_button_t;

#define PHY_BUTTON_NUM 5

static bool is_long_pressed(struct timeval *now, struct timeval *before, int interval) {
    int64_t expire = now->tv_sec * 1000 + now->tv_usec / 1000 - (before->tv_sec * 1000 + before->tv_usec / 1000);
    if (expire >= interval) {
        return true;
    }
    return false;
}

int button_run(button_handle_t self) {
    return button_run2(self, 100);
}

int button_run2(button_handle_t self, int ms) {

    fd_set rfd;
    struct timeval tv;
    struct input_event ev;
    int ret;

    phy_button_t buttons[PHY_BUTTON_NUM];
    memset(buttons, 0, sizeof(buttons));
    /*
    buttons[0]  ---> PHY_BUTTON_CODE_VOLUME_ADD
    buttons[1]  ---> PHY_BUTTON_CODE_VOLUME_SUB
    buttons[2]  ---> PHY_BUTTON_CODE_MUTE
    buttons[3]  ---> PHY_BUTTON_CODE_PLAY_PAUSE
    buttons[4]  ---> PHY_BUTTON_CODE_MODE
    */

    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = ms * 1000;
        FD_ZERO(&rfd);
        FD_SET(self->fd, &rfd);

        ret = select(self->fd + 1, &rfd, NULL, NULL, &tv);
        if (ret == 0) {
            struct timeval now;
            gettimeofday(&now, NULL);
            //if (buttons[0].long_pressed == false && buttons[0].val == 1 && is_long_pressed(&now, &buttons[0].last_time, 1500)) {
            if (buttons[0].val == 1 && is_long_pressed(&now, &buttons[0].last_time, 1500)) {
                self->cb(BUTTON_EVENT_PREV, self->userdata);
                buttons[0].long_pressed = true;
                buttons[0].last_time = now;
            //} else if (buttons[1].long_pressed == false && buttons[1].val == 1 && is_long_pressed(&now, &buttons[1].last_time, 1500)) {
            } else if (buttons[1].val == 1 && is_long_pressed(&now, &buttons[1].last_time, 1500)) {
                self->cb(BUTTON_EVENT_NEXT, self->userdata);
                buttons[1].long_pressed = true;
                buttons[1].last_time = now;
            } else if (buttons[2].long_pressed == false && buttons[2].val == 1 && is_long_pressed(&now, &buttons[2].last_time, 1500)) {
                buttons[2].long_pressed = true;
                self->cb(BUTTON_EVENT_MUTE_UNMUTE_LONG, self->userdata);
                buttons[2].long_pressed = true;
                buttons[2].last_time = now;
            } else if (buttons[3].long_pressed == false && buttons[3].val == 1 && is_long_pressed(&now, &buttons[3].last_time, 1500)) {
                buttons[3].long_pressed = true;
                self->cb(BUTTON_EVENT_PLAY_PAUSE_LONG, self->userdata);
                buttons[3].long_pressed = true;
                buttons[3].last_time = now;
            } else if (buttons[4].long_pressed == false && buttons[4].val == 1 && is_long_pressed(&now, &buttons[4].last_time, 3000)) {
                self->cb(BUTTON_EVENT_MODE_WIFI, self->userdata);
                buttons[4].long_pressed = true;
                buttons[4].last_time = now;
            }
        }

        if ((ret > 0) && FD_ISSET(self->fd, &rfd)) {
            memset(&ev, 0, sizeof(ev));
            read(self->fd, &ev, sizeof(ev));
            if (ev.type == EV_KEY) {
                //printf("type: %d, code: %d, value: %d\n", ev.type, ev.code, ev.value);
                //按键状态发生改变
                switch (ev.code) {
                    case PHY_BUTTON_CODE_VOLUME_ADD:
                        if (buttons[0].val == 0 && ev.value == 1) {
                            //按键按下，开始计时
                            gettimeofday(&buttons[0].last_time, NULL);
                        } else if (buttons[0].val == 1 && ev.value == 0) {
                            //按键松开
                            if (!buttons[0].long_pressed) {
                                self->cb(BUTTON_EVENT_VOLUME_ADD, self->userdata);
                            }
                            buttons[0].long_pressed = false;
                        }
                        //更新按键状态
                        buttons[0].val = ev.value;
                        break;
                    case PHY_BUTTON_CODE_VOLUME_SUB:
                        if (buttons[1].val == 0 && ev.value == 1) {
                            //按键按下，开始计时
                            gettimeofday(&buttons[1].last_time, NULL);
                        } else if (buttons[1].val == 1 && ev.value == 0) {
                            //按键松开
                            if (!buttons[1].long_pressed) {
                                self->cb(BUTTON_EVENT_VOLUME_SUB, self->userdata);
                            }
                            buttons[1].long_pressed = false;
                        }
                        //更新按键状态
                        buttons[1].val = ev.value;
                        break;
                    case PHY_BUTTON_CODE_MUTE:
                        if (buttons[2].val == 0 && ev.value == 1) {
                            //按键按下，开始计时
                            gettimeofday(&buttons[2].last_time, NULL);
                        } else if (buttons[2].val == 1 && ev.value == 0) {
                            //按键松开
                            if (!buttons[2].long_pressed) {
                                self->cb(BUTTON_EVENT_MUTE_UNMUTE, self->userdata);
                            }
                            buttons[2].long_pressed = false;
                        }
                        //更新按键状态
                        buttons[2].val = ev.value;
                        break;
                    case PHY_BUTTON_CODE_PLAY_PAUSE:
                        if (buttons[3].val == 0 && ev.value == 1) {
                            //按键按下，开始计时
                            gettimeofday(&buttons[3].last_time, NULL);
                        } else if (buttons[3].val == 1 && ev.value == 0) {
                            //按键松开
                            if (!buttons[3].long_pressed) {
                                self->cb(BUTTON_EVENT_PLAY_PAUSE, self->userdata);
                            }
                            buttons[3].long_pressed = false;
                        }
                        //更新按键状态
                        buttons[3].val = ev.value;
                        break;
                    case PHY_BUTTON_CODE_MODE:
                        if (buttons[4].val == 0 && ev.value == 1) {
                            //按键按下，开始计时
                            gettimeofday(&buttons[4].last_time, NULL);
                        } else if (buttons[4].val == 1 && ev.value == 0) {
                            //按键松开
                            if (!buttons[4].long_pressed) {
                                self->cb(BUTTON_EVENT_MODE_NORMAL, self->userdata);
                            }
                            buttons[4].long_pressed = false;
                        }
                        //更新按键状态
                        buttons[4].val = ev.value;
                        break;
                    default:break;
                }
            }
        }
    }
    return 0;
}

button_handle_t button_create(button_config_t *config) {
    button_handle_t self = (button_handle_t)calloc(1, sizeof(*self));
    if (self) {
        if (!config->dev) {
#ifdef RK3308_BOARD_V10
            config->dev = "/dev/input/event1";
#elif defined(RK3308_BOARD_V11)
            config->dev = "/dev/input/event0";
#else
#error "RK3308 BOADRD VERSION IS ERROR"
#endif
        }
        self->fd = open(config->dev, O_RDONLY);
        assert(self->fd > 0);
        self->cb = config->cb;
        self->userdata = config->userdata;
    }
    return self;
}

void button_destroy(button_handle_t self) {
    if (!self) return;
    close(self->fd);
    free(self);
}

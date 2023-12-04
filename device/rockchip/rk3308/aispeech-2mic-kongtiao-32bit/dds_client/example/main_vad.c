#include "dui.h"
#include "dui_msg.h"
#include "dui_thread.h"
#include "dui_queue.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>

#define EV_RECODER_CLOSE (1 << 0)

static dui_event_group_handle_t event;

extern dui_queue_handle_t user_listen_queue;

static void listen_cb(dui_msg_t *msg) {
    dui_msg_t m;
    int ret;
    while (1) {
        ret = dui_queue_receive(user_listen_queue, &m); 
        if (ret == -1) break;
        if (m.type == RECORDER_CMD_STOP) {
            dui_event_group_set_bits(event, EV_RECODER_CLOSE);
        }
    }
}


int main(int argc, char **argv) {
    FILE *fd = fopen(argv[1], "rb");
    fseek(fd, 0L, SEEK_END);
    int len = ftell(fd);
    char *buf = (char *)malloc(len + 1);
    fseek(fd, 0L, SEEK_SET);
    fread(buf, 1, len, fd);

    event = dui_event_group_create();

    dui_library_init(buf, listen_cb);
    fclose(fd);
    dui_start_recorder();
    while (1) {
        int buf[64];
        long frames;
        while (1) {
            struct timeval tv = { 
                .tv_usec = 50000
            };  
            frames = -1; 
            fd = fopen("/sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames", "r");
            if (fd) {
                if (fgets(buf, sizeof(buf), fd)) {
                    frames = atol(buf);
                }   
                fclose(fd);
            }   
            if (frames > 80000) {
                dui_stop_recorder();
                dui_event_group_wait_bits(event, EV_RECODER_CLOSE, true, true);
                system("echo 0 > /sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames");
                system("echo mem > /sys/power/state");
                dui_start_recorder();
            }   
            select(0, NULL, NULL, NULL, &tv);
        }   
    }
    dui_library_cleanup();
    return 0;
}

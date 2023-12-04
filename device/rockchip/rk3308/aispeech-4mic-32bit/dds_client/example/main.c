#include "dui.h"
#include "dui_msg.h"
#include "os_thread.h"
#include "os_queue.h"
#include "os_event_group.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>

#define EV_RECODER_CLOSE (1 << 0)

static os_event_group_handle_t event;

extern os_queue_handle_t user_listen_queue;

static void listen_cb(dui_msg_t *msg) {
    dui_msg_t m;
    int ret;
    while (1) {
        ret = os_queue_receive(user_listen_queue, &m); 
        if (ret == -1) break;
        if (m.type == RECORDER_CMD_STOP) {
            os_event_group_set_bits(event, EV_RECODER_CLOSE);
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

    event = os_event_group_create();

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
            sleep(1);
            printf("\n frames = %d", frames);

            if (frames > 80000) {
                dui_stop_recorder();
                os_event_group_wait_bits(event, EV_RECODER_CLOSE, true, true);
                system("echo 0 > /sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames");
                system("echo mem > /sys/power/state");
               // usleep(30 * 1000);
                dui_start_recorder();
            }   
            select(0, NULL, NULL, NULL, &tv);
        }   
    }
    dui_library_cleanup();
    return 0;
}

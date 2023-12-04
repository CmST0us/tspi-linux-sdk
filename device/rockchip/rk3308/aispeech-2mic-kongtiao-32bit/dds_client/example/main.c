#include "dui.h"
#include "dui_msg.h"
#include "os_thread.h"
#include "os_queue.h"
#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



extern const char *dui_msg_table[];

extern os_queue_handle_t user_listen_queue;

static void listen_cb(dui_msg_t *msg) {
    dui_msg_t m;
    int ret;
    unsigned char tmp[10];

    while (1) {
        ret = os_queue_receive(user_listen_queue, &m);
        if (ret == -1) break;
        if (m.type == WAKEUP_INFO_WAKEUP_MINOR) {
            if (m.wakeup.index >= 0) {
                //TODO
                //此处的索引值就是在配置文件中wakeupWord的索引值，请注意要一一对应
                //另外此处尽量不要阻塞，若要处理很多消息，请将user_listen_queue的大小设置答谢
                printf("\n=====rockchip's log :m.wakeup.index = %d\n",m.wakeup.index);
                tmp[0] = m.wakeup.index;
                tmp[1] = m.wakeup.index;
                tmp[2] = m.wakeup.index;
                tmp[3] = m.wakeup.index;
                tmp[4] = m.wakeup.index;
                tmp[5] = m.wakeup.index;
                tmp[6] = 0;
                serial_send_data(ut.fd,tmp);
            }
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


    ut.fd = serial_openport(1500000);
    if(ut.fd  < 0) {
        printf("serial_openport failed\n");
    }
    printf("====ut.fd bandrate = %d\n",getbaud(ut.fd));
    //dui_library_init(buf, NULL);
    dui_library_init(buf, listen_cb);
    dui_start_recorder();
    while (1) {
        sleep(20);
    }
    dui_stop_recorder();
    dui_library_cleanup();
    close_port(/*m_serial_fd*/);
    return 0;
}

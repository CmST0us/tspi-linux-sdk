/*================================================================
*   Copyright (C) 2018 FREEDOM Ltd. All rights reserved.
*   
*   文件名称：main.c
*   创 建 者：chenjie.gu
*   创建日期：2018年05月23日
*   描    述：
*
================================================================*/

#include "busserver.h"
#include <stdio.h>
#include <sys/select.h>
#include <pthread.h>

void cb(const char *topic, const char *topic_data, void *user) {

    printf("%s: %s\n", topic, topic_data);
    
}

void *busserver_routine(void *user) {
    busserver_run("127.0.0.1:50001", cb, NULL);
    return (void *)0;
}

int main () {
    //
    //
    
    int ret = 0;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, busserver_routine, NULL);

    struct timeval tv = {10, 0};
    select(0, 0, 0, 0, &tv);
    
    busserver_send_msg("bus.event", "yyyyyyyyyyy");

    select(0, 0, 0, 0, 0);

    return 0;
}

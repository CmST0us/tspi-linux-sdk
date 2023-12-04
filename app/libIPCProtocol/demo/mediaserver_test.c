#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>

#include "json-c/json.h"
#include "mediaserver.h"
#include "IPCProtocol.h"
#include "dbserver.h"

void get_record_status()
{
    int ret = mediaserver_get_record_status(0);
    printf("record status is %d\n", ret);
}

void help_printf(void)
{
    printf("************************\n");
    printf("0.help\n");
    printf("1.take photo\n");
    printf("2.start record\n");
    printf("3.get record status\n");
    printf("4.stop record\n");
    printf("5.already change gray scale mode, reset pipes\n");
    printf("************************\n");
}

int main( int argc , char ** argv)
{
    help_printf();
    while (1) {
        char cmd = 0;
        printf("please enter:");
again:
        scanf("%c", &cmd);
        switch(cmd) {
            case '0':
                help_printf();
                break;
            case '1':
                mediaserver_take_photo(0, 1);
                break;
            case '2':
                mediaserver_start_record(0);
                break;
            case '3':
                get_record_status();
                break;
            case '4':
                mediaserver_stop_record(0);
                break;
            case '5':
                mediaserver_set(TABLE_VIDEO, 0, "{\"sGrayScaleMode\":\"[0-255]\"}");
                break;
            case '6':
            case 0xa:
                continue;
                break;
        }
        goto again;
    }

    return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "Upgrade.h"

enum { INSTALL_SUCCESS, INSTALL_ERROR, INSTALL_CORRUPT };

FILE *cmd_pipe = NULL;
int sdBootUpdate = 0;


void handle_upgrade_callback(char *szPrompt)
{
    if (cmd_pipe != NULL)
    {
        fprintf(cmd_pipe, "ui_print %s\n", szPrompt);
    }
}

void handle_upgrade_progress_callback(float portion, float seconds)
{
    if (cmd_pipe != NULL)
    {
        if (seconds == 0)
        {
            fprintf(cmd_pipe, "set_progress %f\n", portion);
        }
        else
        {
            fprintf(cmd_pipe, "progress %f %d\n", portion, seconds);
        }
    }
}

int main(int argc, char *argv[])
{
    int status;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    if (argc != 5)
    {
        printf("unexpected number of arguments (%d)\n", argc);
        fprintf(stderr, "unexpected number of arguments (%d)\n", argc);
        return 1;
    }
    int fd = atoi(argv[2]);
    cmd_pipe = fdopen(fd, "wb");
    setlinebuf(cmd_pipe);

    char *filepath = argv[3];
    sdBootUpdate = atoi(argv[4]);

    //call update
    bool bRet = do_rk_firmware_upgrade(filepath, (void *)handle_upgrade_callback,
                                       (void *)handle_upgrade_progress_callback);

    if (!bRet)
    {
        status = INSTALL_ERROR;
    }
    else
    {
        status = INSTALL_SUCCESS;
    }

    sleep(5);
    sync();
    return status;
}

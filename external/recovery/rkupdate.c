#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "install.h"
#include "bootloader.h"

#define URL_MAX_LENGTH 512
#define CMDLINE_LENGTH 2048
extern bool bSDBootUpdate;
static int start_main (const char *binary, char *args[], int* pipefd) {
    pid_t pid = fork();
    if(pid == 0){
        close(pipefd[0]);
        execv(binary, args);
        printf("E:Can't run %s (%s)\n", binary, strerror(errno));
        fprintf(stdout, "E:Can't run %s (%s)\n", binary, strerror(errno));
        _exit(-1);
    }
    close(pipefd[1]);

    char buffer[1024];
    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
        } else if (strcmp(command, "progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            char* seconds_s = strtok(NULL, " \n");

            float fraction = strtof(fraction_s, NULL);
            int seconds = strtol(seconds_s, NULL, 10);

            ui_show_progress(fraction * (1-VERIFICATION_PROGRESS_FRACTION), seconds);
        } else if (strcmp(command, "set_progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            float fraction = strtof(fraction_s, NULL);

            ui_set_progress(fraction);
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str) {
                printf(" >>>>>> %s <<<<<<\n", str);
                ui_print("%s", str);
            } else {
                ui_print("\n");
            }
        } else {
            LOGE("unknown command [%s]\n", command);
        }
    }

    fclose(from_child);

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", binary, WEXITSTATUS(status));
        return INSTALL_ERROR;
    }
    return INSTALL_SUCCESS;

}

int do_rk_updateEngine(const char *binary, const char *path) {
    LOGI("[%s] start with main.\n", __func__);
    char *update="--update";
    char *update_sdboot="--update=sdboot";
    int pipefd[2];
    pipe(pipefd);

    //updateEngine --update --image_url=path --partition=0x3a0000
    char *args[6];
    char args_1[20] = {0};
    char args_2[URL_MAX_LENGTH] = {0};
    char args_4[32] = {0};
    args[0] = (char* )binary;
    args[1] = args_1;
    sprintf(args[1], "--pipefd=%d", pipefd[1]);
    args[2] = args_2;
    sprintf(args[2], "--image_url=%s", path);

    if (bSDBootUpdate) {
        char path_second[64] = {0};
        char path_second_dir[64] = {0};

        sprintf(path_second_dir, "%s", path);
        dirname(path_second_dir);

        sprintf(path_second, "%s/update_ab.img", path_second_dir);
        if (access(path_second, F_OK) == 0) {
            sprintf(args[2], "--image_url=%s", path_second);
        } else {
            sprintf(path_second, "%s/update_ota.img", path_second_dir);
            if (access(path_second, F_OK) == 0) {
                sprintf(args[2], "--image_url=%s", path_second);
            }
        }
    }
    LOGI("SDcard update data: [%s]\n", args[2]);

    if (bSDBootUpdate) {
        args[3] = (char *)update_sdboot;
    } else {
        args[3] = (char *)update;
    }

    args[4] = args_4;
    args[5] = NULL;

    if (bSDBootUpdate) {
        // If SD boot, ignore --partition
        sprintf(args[4], "--partition=0x%s", "FFFF00");
    } else {
        struct bootloader_message boot;
        get_bootloader_message(&boot);
        sprintf(args[4], "--partition=0x%X", *((int *)(boot.needupdate)));
    }

    return start_main(binary, args, pipefd);

}
int do_rk_update(const char *binary, const char *path) {
    LOGI("[%s] start with main.\n", __func__);
    int pipefd[2];
    pipe(pipefd);

    char* args[6];
    args[0] = (char* )binary;
    args[1] = "Version 1.0";
    char args_2[10] = {0};
    args[2] = args_2;
    sprintf(args[2], "%d", pipefd[1]);
    args[3] = (char*)path;
    char args_4[8] = {0};
    args[4] = args_4;
    sprintf(args[4], "%d", (int)bSDBootUpdate);
    args[5] = NULL;
    return start_main(binary, args, pipefd);

#if 0
    pid_t pid = fork();
    if(pid == 0){
        close(pipefd[0]);
        execv(binary, args);
        printf("E:Can't run %s (%s)\n", binary, strerror(errno));
        fprintf(stdout, "E:Can't run %s (%s)\n", binary, strerror(errno));
        _exit(-1);
    }
    close(pipefd[1]);

    char buffer[1024];
    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
        } else if (strcmp(command, "progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            char* seconds_s = strtok(NULL, " \n");

            float fraction = strtof(fraction_s, NULL);
            int seconds = strtol(seconds_s, NULL, 10);

            ui_show_progress(fraction * (1-VERIFICATION_PROGRESS_FRACTION), seconds);
        } else if (strcmp(command, "set_progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            float fraction = strtof(fraction_s, NULL);
            ui_set_progress(fraction);
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str) {
                printf("ui_print = %s.\n", str);
                ui_print("%s", str);
            } else {
                ui_print("\n");
            }
        } else {
            LOGE("unknown command [%s]\n", command);
        }
    }

    fclose(from_child);

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", path, WEXITSTATUS(status));
        return INSTALL_ERROR;
    }
    return INSTALL_SUCCESS;
#endif
}

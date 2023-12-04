#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>
#include <pthread.h>

#include "json-c/json.h"
#include "system_manager.h"

static char command[128] = {""};
static char path[128] = {""};

void usage() {
    printf("Usage: %s [reboot|factory_reset|import_db|export_db|export_log|upgrade] file\n");
    exit(-1);
}

int main(int argc ,char **argv) {
    if (argc < 2)
        usage();

    if (!strcmp(argv[1], "reboot")) {
        system_reboot();
        return 0;
    }
    if (!strcmp(argv[1], "factory_reset")) {
        system_factory_reset();
        return 0;
    }

    if (argc < 3)
        usage();
    char *path = argv[2];

    if (!strcmp(argv[1], "import_db")) {
        system_import_db(path);
        return 0;
    }
    if (!strcmp(argv[1], "export_db")) {
        system_export_db(path);
        return 0;
    }
    if (!strcmp(argv[1], "export_log")) {
        system_export_log(path);
        return 0;
    }
    if (!strcmp(argv[1], "upgrade")) {
        system_upgrade(path);
        return 0;
    }

    return 0;
}

#ifndef __MAIN_H__
#define __MAIN_H__

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <png.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <lvgl/lvgl.h>

#include "lvgl/lv_port_file.h"
#include "lvgl/lv_port_indev.h"
#include "sys/timestamp.h"

#define ALIGN(x, a)     (((x) + (a - 1)) & ~(a - 1))
#define FAKE_FD         1234

enum {
    SCALE_MODE_FILL = 0x0,  // full screen stretch
    SCALE_MODE_CONTAIN,     // keep the scale, The side with the larger ratio
                            // is aligned with the container, and the other
                            // side is scaled equally
    SCALE_MODE_COVER,       // keep the scale, The side with the smaller ratio
                            // is aligned with the container, and the other
                            // side is scaled equally
    SCALE_MODE_NONE,
};

#define RK_LV_IMG_DSC_MAGIC     0x6996a55a

typedef struct
{
    lv_img_dsc_t img_dsc;
    uint32_t magic;
    void (*free)(void *para);
    void *para;
    int rot;
} rk_lv_img_dsc_t;

#endif


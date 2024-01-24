/**
 * @file evdev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "key.h"
#if USE_KEY != 0

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static int key_fd;
static int key_button;

static int key_val;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the evdev interface
 */
void key_init(void)
{
    key_fd = open(KEY_NAME, O_RDWR | O_NOCTTY | O_NDELAY);

    if(key_fd == -1) {
        perror("unable open evdev interface:");
        return;
    }

    fcntl(key_fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    key_val = 0;
    key_button = LV_INDEV_STATE_REL;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
void key_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct input_event in;

    while(read(key_fd, &in, sizeof(struct input_event)) > 0) {
        if(in.type == EV_KEY) {
            data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
            switch(in.code) {
            case 1:
                data->key = LV_KEY_ESC;
                break;
            case 139:
                data->key = LV_KEY_ENTER;
                break;
            //case KEY_UP:
            //    data->key = LV_KEY_UP;
            //    break;
            case 105:
                data->key = LV_KEY_PREV;
                break;
            case 106:
                data->key = LV_KEY_NEXT;
                break;
            //case KEY_DOWN:
            //    data->key = LV_KEY_DOWN;
            //    break;
            default:
                data->key = 0;
                break;
            }
            key_val = data->key;
            key_button = data->state;
            return ;
        }
    }

    if(drv->type == LV_INDEV_TYPE_KEYPAD) {
        /* No data retrieved */
        data->key = key_val;
        data->state = key_button;
        return ;
    }
}
#endif

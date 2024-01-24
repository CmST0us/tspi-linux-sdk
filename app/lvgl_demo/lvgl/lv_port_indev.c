/**
 * @file lv_port_indev_templ.c
 *
 */

 /* Copy this file as "lv_port_indev.c" and set this value to "1" to enable conten */

#include <stdlib.h>
#include "lvgl.h"
#include "lv_port_indev.h"

#include "hal/evdev.h"
#include "hal/key.h"

typedef struct _GROUP_NODE {
    struct _GROUP_NODE *next;
    lv_group_t *group;
} GROUP_NODE;

static int rot_indev;
lv_indev_t * indev_touchpad;
lv_indev_t * indev_key;

GROUP_NODE *group_list = NULL;

lv_group_t *lv_port_indev_group_create(void)
{
    struct _GROUP_NODE *group_node = NULL;
    lv_group_t *group = lv_group_create();

    lv_indev_set_group(indev_key, group);
    lv_group_set_default(group);

    group_node = (struct _GROUP_NODE *)malloc(sizeof(struct _GROUP_NODE));
    group_node->group = group;
    group_node->next = NULL;
    if (group_list) {
        group_node->next = group_list;
        group_list = group_node;
    } else {
        group_list = group_node;
    }

    return group;
}

void lv_port_indev_group_destroy(lv_group_t *group)
{
    if (group_list) {
        struct _GROUP_NODE *group_node = NULL;
        group_node = group_list;
        if (group_list->group == group) {
            group_list = group_list->next;
            if (group_list) {
                lv_indev_set_group(indev_key, group_list->group);
                lv_group_set_default(group_list->group);
            } else {
                lv_indev_set_group(indev_key, NULL);
                lv_group_set_default(NULL);
            }
            free(group_node);
        } else {
            while (group_node->next) {
                struct _GROUP_NODE *group_node_next = group_node->next;
                if (group_node_next->group == group) {
                    group_node->next = group_node_next->next;
                    free(group_node_next);
                    break;
                }
                group_node = group_node->next;
            }
        }
        lv_group_del(group);
    }
}

void lv_port_indev_init(int rot)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv;
    static lv_indev_drv_t key_drv;

    rot_indev = rot;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    evdev_init(rot);

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);

#if USE_KEY
    key_init();
    lv_indev_drv_init(&key_drv);
    key_drv.type = LV_INDEV_TYPE_KEYPAD;
    key_drv.read_cb = key_read;
    indev_key = lv_indev_drv_register(&key_drv);
    lv_port_indev_group_create();
#endif
}

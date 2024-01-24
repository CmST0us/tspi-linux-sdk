/**
 * @file key.h
 *
 */

#ifndef KEY_H
#define KEY_H

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------
 * Mouse or touchpad as evdev interface (for Linux based systems)
 *------------------------------------------------*/
#define USE_KEY           0

#if USE_KEY
#  define KEY_NAME   "/dev/input/event2"

#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the evdev
 */
void key_init(void);

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
void key_read(lv_indev_drv_t * drv, lv_indev_data_t * data);


/**********************
 *      MACROS
 **********************/

#endif /* USE_KEY */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KEY_H */

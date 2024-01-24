/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "minui/minui.h"
#include "recovery_ui.h"

#define MAX_COLS 96
#define MAX_ROWS 32

#define CHAR_WIDTH 10
#define CHAR_HEIGHT 18

extern int g_key_test ;


#define PROGRESSBAR_INDETERMINATE_STATES 6
#define PROGRESSBAR_INDETERMINATE_FPS 15

static pthread_mutex_t gUpdateMutex = PTHREAD_MUTEX_INITIALIZER;
static gr_surface gBackgroundIcon[NUM_BACKGROUND_ICONS];
static gr_surface gProgressBarIndeterminate[PROGRESSBAR_INDETERMINATE_STATES];
static gr_surface gProgressBarEmpty;
static gr_surface gProgressBarFill;

static const struct { gr_surface* surface; const char *name; } BITMAPS[] = {
    { &gBackgroundIcon[BACKGROUND_ICON_INSTALLING], "icon_installing" },
    { &gBackgroundIcon[BACKGROUND_ICON_ERROR],      "icon_error" },
    { &gProgressBarIndeterminate[0],    "indeterminate1" },
    { &gProgressBarIndeterminate[1],    "indeterminate2" },
    { &gProgressBarIndeterminate[2],    "indeterminate3" },
    { &gProgressBarIndeterminate[3],    "indeterminate4" },
    { &gProgressBarIndeterminate[4],    "indeterminate5" },
    { &gProgressBarIndeterminate[5],    "indeterminate6" },
    { &gProgressBarEmpty,               "progress_empty" },
    { &gProgressBarFill,                "progress_fill" },
    { NULL,                             NULL },
};

static gr_surface gCurrentIcon = NULL;

static enum ProgressBarType {
    PROGRESSBAR_TYPE_NONE,
    PROGRESSBAR_TYPE_INDETERMINATE,
    PROGRESSBAR_TYPE_NORMAL,
} gProgressBarType = PROGRESSBAR_TYPE_NONE;

// Progress bar scope of current operation
static float gProgressScopeStart = 0, gProgressScopeSize = 0, gProgress = 0;
static time_t gProgressScopeTime, gProgressScopeDuration;

// Set to 1 when both graphics pages are the same (except for the progress bar)
static int gPagesIdentical = 0;

// Log text overlay, displayed when a magic key is pressed
static char text[MAX_ROWS][MAX_COLS];
static int text_cols = 0, text_rows = 0;
static int text_col = 0, text_row = 0, text_top = 0;
static int show_text = 1;

static char menu[MAX_ROWS][MAX_COLS];
static int show_menu = 0;
static int menu_top = 0, menu_items = 0, menu_sel = 0;

// Key event input queue
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;
static int key_queue[256], key_queue_len = 0;
static volatile char key_pressed[KEY_MAX + 1];


//-----------------------------------------------------------------------------
//#define _EVENT_LOGGING 0

static int touch_tp_state = 0;

typedef struct{
	int t_col;
	int t_row;
	int r;
	int g;
	int b;
	int a;
} textInfo;

textInfo itemsInfo[MAX_ROWS];

typedef struct{
	   int left;
	   int top;
	   int right;
	   int bottom;
	   int r;
	   int g;
	   int b;
	   int a;
} FillColorTile;
FillColorTile  tiles[kMaxTiles];
int tiles_count = 0;

typedef struct{
	   int x0;
	   int y0;
	   int x1;
	   int y1;
	   int linewidth;
	   int r;
	   int g;
	   int b;
	   int a;
} LineInfo;
LineInfo  lines[kMaxTiles];
int lines_count = 0;


typedef enum {
	TOUCH_START = 0,
	TOUCH_DRAG = 1,
	TOUCH_RELEASE = 2,
	TOUCH_HOLD = 3,
	TOUCH_REPEAT = 4
} TOUCH_STATE;

static int gUiInitialized = 0;

extern int NotifyTouch(int action, int x, int y);

//-----------------------------------------------------------------------------



// Clear the screen and draw the currently selected background icon (if any).
// Should only be called with gUpdateMutex locked.
static void draw_background_locked(gr_surface icon)
{
    gPagesIdentical = 0;
    gr_color(0, 0, 0, 255);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    if (icon) {
        int iconWidth = gr_get_width(icon);
        int iconHeight = gr_get_height(icon);
        int iconX = (gr_fb_width() - iconWidth) / 2;
        int iconY = (gr_fb_height() - iconHeight) / 2;
        gr_blit(icon, 0, 0, iconWidth, iconHeight, iconX, iconY);
    }
}

// Draw the progress bar (if any) on the screen.  Does not flip pages.
// Should only be called with gUpdateMutex locked.
static void draw_progress_locked()
{
    if (gProgressBarType == PROGRESSBAR_TYPE_NONE) return;

    int iconHeight = gr_get_height(gBackgroundIcon[BACKGROUND_ICON_INSTALLING]);
    int width = gr_get_width(gProgressBarEmpty);
    int height = gr_get_height(gProgressBarEmpty);

    int dx = (gr_fb_width() - width)/2;
    int dy = (3*gr_fb_height() + iconHeight - 2*height)/4;

    // Erase behind the progress bar (in case this was a progress-only update)
    gr_color(0, 0, 0, 255);
    gr_fill(dx, dy, width, height);

    if (gProgressBarType == PROGRESSBAR_TYPE_NORMAL) {
        float progress = gProgressScopeStart + gProgress * gProgressScopeSize;
        int pos = (int) (progress * width);

        if (pos > 0) {
          gr_blit(gProgressBarFill, 0, 0, pos, height, dx, dy);
        }
        if (pos < width-1) {
          gr_blit(gProgressBarEmpty, pos, 0, width-pos, height, dx+pos, dy);
        }
    }

    if (gProgressBarType == PROGRESSBAR_TYPE_INDETERMINATE) {
        static int frame = 0;
        gr_blit(gProgressBarIndeterminate[frame], 0, 0, width, height, dx, dy);
        frame = (frame + 1) % PROGRESSBAR_INDETERMINATE_STATES;
    }
}

static void  draw_text_line(int left, int top, const char* t) {
  if (t[0] != '\0') {
    gr_text(left,top , t,0);
  }
}


// Redraw everything on the screen.  Does not flip pages.
// Should only be called with gUpdateMutex locked.
static void draw_screen_locked(void)
{
    draw_background_locked(gCurrentIcon);
    draw_progress_locked();

    if (show_text)
	{
		gr_color(0, 0, 0, 160);
		gr_fill(0, 0, gr_fb_width(), gr_fb_height());

		int k = 0;
		for (k = 0; k < text_row; ++k) {
			gr_color(itemsInfo[k].r, itemsInfo[k].g, itemsInfo[k].b,
				itemsInfo[k].a);
			draw_text_line((itemsInfo[k].t_col + 1)*CHAR_WIDTH + 1,
				(itemsInfo[k].t_row)*CHAR_HEIGHT, text[k]);
		}

		for(k = 0; k < lines_count;k++){
        	gr_color(lines[k].r, lines[k].g, lines[k].b, lines[k].a);
        	//gr_line(lines[k].x0, lines[k].y0, lines[k].x1, lines[k].y1,lines[k].linewidth);
		}

		for(k = 0; k < tiles_count;k++){
			gr_color(tiles[k].r, tiles[k].g, tiles[k].b, tiles[k].a);
			gr_fill(tiles[k].left, tiles[k].top, tiles[k].right, tiles[k].bottom);
		}
	}
}

// Redraw everything on the screen and flip the screen (make it visible).
// Should only be called with gUpdateMutex locked.
static void update_screen_locked(void)
{
    draw_screen_locked();
    gr_flip();
}

// Updates only the progress bar, if possible, otherwise redraws the screen.
// Should only be called with gUpdateMutex locked.
static void update_progress_locked(void)
{
    printf("%s in >>>>>> \n",__func__);
    if (show_text || !gPagesIdentical) {
        draw_screen_locked();    // Must redraw the whole screen
        gPagesIdentical = 1;
    } else {
        draw_progress_locked();  // Draw only the progress bar
    }
    gr_flip();
}

// Keeps the progress bar updated, even when the process is otherwise busy.
static void *progress_thread(void *cookie)
{
    printf("%s in >>>>>> \n",__func__);
    for (;;) {
        usleep(1000000 / PROGRESSBAR_INDETERMINATE_FPS);
        pthread_mutex_lock(&gUpdateMutex);

        // update the progress bar animation, if active
        // skip this if we have a text overlay (too expensive to update)
        if (gProgressBarType == PROGRESSBAR_TYPE_INDETERMINATE && !show_text) {
            update_progress_locked();
        }

        // move the progress bar forward on timed intervals, if configured
        int duration = gProgressScopeDuration;
        if (gProgressBarType == PROGRESSBAR_TYPE_NORMAL && duration > 0) {
            int elapsed = time(NULL) - gProgressScopeTime;
            float progress = 1.0 * elapsed / duration;
            if (progress > 1.0) progress = 1.0;
            if (progress > gProgress) {
                gProgress = progress;
                update_progress_locked();
            }
        }

        pthread_mutex_unlock(&gUpdateMutex);
    }
    return NULL;
}

static int rel_sum = 0;

static int input_callback(int fd, short revents, void *data)
{
    struct input_event ev;
    int ret;
    int fake_key = 0;

    ret = ev_get_input(fd, revents, &ev);
    if (ret)
        return -1;

    if (ev.type == EV_SYN) {
        return 0;
    } else if (ev.type == EV_REL) {
        if (ev.code == REL_Y) {
            // accumulate the up or down motion reported by
            // the trackball.  When it exceeds a threshold
            // (positive or negative), fake an up/down
            // key event.
            rel_sum += ev.value;
            if (rel_sum > 3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_DOWN;
                ev.value = 1;
                rel_sum = 0;
            } else if (rel_sum < -3) {
                fake_key = 1;
                ev.type = EV_KEY;
                ev.code = KEY_UP;
                ev.value = 1;
                rel_sum = 0;
            }
        }
    } else {
        rel_sum = 0;
    }

    if (ev.type != EV_KEY || ev.code > KEY_MAX)
        return 0;

    pthread_mutex_lock(&key_queue_mutex);
    if (!fake_key) {
        // our "fake" keys only report a key-down event (no
        // key-up), so don't record them in the key_pressed
        // table.
        key_pressed[ev.code] = ev.value;
    }
    const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
    if (ev.value > 0 && key_queue_len < queue_max) {
        key_queue[key_queue_len++] = ev.code;
        pthread_cond_signal(&key_queue_cond);
    }
    pthread_mutex_unlock(&key_queue_mutex);

    if (ev.value > 0 && device_toggle_display(key_pressed, ev.code)) {
        pthread_mutex_lock(&gUpdateMutex);
        show_text = !show_text;
        update_screen_locked();
        pthread_mutex_unlock(&gUpdateMutex);
    }

    if (ev.value > 0 && device_reboot_now(key_pressed, ev.code)) {
            reboot(RB_AUTOBOOT);
    }

    return 0;
}


// Reads input events, handles special hot keys, and adds to the key queue.
static void *input_thread(void *cookie)
{
    printf("%s in >>>>>> \n",__func__);

        int drag = 0;
        static int touch_and_hold = 0, dontwait = 0, touch_repeat = 0, x = 0, y = 0, lshift = 0, rshift = 0, key_repeat = 0;
        static struct timeval touchStart;
        static struct timeval keyStart, keyEnd;
        //LOGE("start input thread!\n");
        printf("start input thread! \n");

        memset(&touchStart, 0, sizeof(struct timeval));
        memset(&keyStart, 0, sizeof(struct timeval));
        memset(&keyEnd, 0, sizeof(struct timeval));

        for (;;)
        {
            // wait for the next event
            struct input_event ev;
            int state = 0, ret = 0;

            ret = ev_get(&ev, dontwait);
            //LOGE("input_thread::type:%d>>code:%d>>value:%d>>EV_ABS:%d>>EV_KEY:%d\n",ev.type,ev.code,ev.value,EV_ABS,EV_KEY);
            if (ret < 0)
            {
                struct timeval curTime;
                gettimeofday(&curTime, NULL);
                long mtime, seconds, useconds;

                seconds  = curTime.tv_sec  - touchStart.tv_sec;
                useconds = curTime.tv_usec - touchStart.tv_usec;

                mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
                if (touch_and_hold && mtime > 500)
                {
                    touch_and_hold = 0;
                    touch_repeat = 1;
                    gettimeofday(&touchStart, NULL);
#ifdef _EVENT_LOGGING
                    LOGE("TOUCH_HOLD: %d,%d\n", x, y);
#endif
                    NotifyTouch(TOUCH_HOLD, x, y);
                }
                else if (touch_repeat && mtime > 100)
                {
#ifdef _EVENT_LOGGING
                    LOGE("TOUCH_REPEAT: %d,%d\n", x, y);
#endif
                    gettimeofday(&touchStart, NULL);
                    NotifyTouch(TOUCH_REPEAT, x, y);
                }
                else if (key_repeat == 1 && mtime > 500)
                {
#ifdef _EVENT_LOGGING
                    LOGE("KEY_HOLD: %d,%d\n", x, y);
#endif
                    gettimeofday(&touchStart, NULL);
                    key_repeat = 2;
                }
                else if (key_repeat == 2 && mtime > 100)
                {
#ifdef _EVENT_LOGGING
                    LOGE("KEY_REPEAT: %d,%d\n", x, y);
#endif
                    gettimeofday(&touchStart, NULL);
                }
            }
            else if (ev.type == EV_ABS)
            {
                if (ev.value==-1)
                {
                    continue;
                }

                x = ev.value >> 16;
                y = ev.value & 0xFFFF;

                if (ev.code == 0)
                {
                    if (state == 0)
                    {
#ifdef _EVENT_LOGGING
                        LOGE("TOUCH_RELEASE: %d,%d\n", x, y);
#endif
                        touch_tp_state = 0;

#if 0 // chad.ma close
                        start_manual_test_item(x,y);
#endif
                        NotifyTouch(TOUCH_RELEASE, x, y);
                        touch_and_hold = 0;
                        touch_repeat = 0;
                        if (!key_repeat)
                            dontwait = 0;
                    }
                    state = 0;
                    drag = 0;
                }
                else
                {
                    if (!drag)
                    {
#ifdef _EVENT_LOGGING
                        LOGE("TOUCH_START: %d,%d\n", x, y);
#endif
                        NotifyTouch(TOUCH_START, x, y);
                        state = 1;
                        drag = 1;
                        touch_and_hold = 1;
                        dontwait = 1;
                        key_repeat = 0;
                        gettimeofday(&touchStart, NULL);
                    }
                    else
                    {
                        if (state == 0)
                        {
                            touch_tp_state = 1;
#ifdef _EVENT_LOGGING
                            LOGE("TOUCH_DRAG: %d,%d\n", x, y);
#endif
                            NotifyTouch(TOUCH_DRAG, x, y);
                            state = 1;
                            key_repeat = 0;
                        }
                    }
                }
            }
            else if (ev.type == EV_KEY)
            {
                // Handle key-press here
			#ifdef _EVENT_LOGGING
                    LOGE("TOUCH_KEY: %d\n", ev.code);
			#endif

                if (ev.code==330)
                {
                    continue;
                }

                if (ev.value != 0 && ev.code != 143)
                {
                    gettimeofday(&keyStart, NULL);
                    printf("key down:%lu\n",keyStart.tv_sec);

                    key_repeat = 0;
                    touch_and_hold = 0;
                    touch_repeat = 0;
                    dontwait = 0;
                }
                else if (ev.code != 143)
                {
                    // This is a key release
                    gettimeofday(&keyEnd, NULL);
#if 1   //chad.ma close
                    printf("key hold time:%lu, g_key_test=%d\n",keyEnd.tv_sec-keyStart.tv_sec, g_key_test);
                    if(g_key_test)
                        set_gkey(ev.code);
#endif
                    key_repeat = 0;
                    touch_and_hold = 0;
                    touch_repeat = 0;
                    dontwait = 0;
                    if((keyEnd.tv_sec-keyStart.tv_sec) >= 5)
                    {
                        break;
                    }
                }
            }
        //printf("tp touch : state : %d\r\n",state);
        //touch_tp_state = state;
        }

    return NULL;
}

void start_input_thread(void)
{
    pthread_t t;
    pthread_create(&t, NULL, input_thread, NULL);
	pthread_join(t,NULL);
}

void ui_init(void)
{
    printf("%s in >>>>>> \n",__func__);

    gr_init();
    ev_init();

    text_col = text_row = 0;
    text_rows = gr_fb_height() / CHAR_HEIGHT;
    if (text_rows > MAX_ROWS) text_rows = MAX_ROWS;
    text_top = 1;
    text_cols = gr_fb_width() / CHAR_WIDTH;
    if (text_cols > MAX_COLS - 1) text_cols = MAX_COLS - 1;

    pthread_t t;
    pthread_create(&t, NULL, progress_thread, NULL);
    pthread_create(&t, NULL, input_thread, NULL);

    gUiInitialized = 1;
}

void ui_set_background(int icon)
{
    pthread_mutex_lock(&gUpdateMutex);
    gCurrentIcon = gBackgroundIcon[icon];
    update_screen_locked();
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_show_indeterminate_progress()
{
    pthread_mutex_lock(&gUpdateMutex);
    if (gProgressBarType != PROGRESSBAR_TYPE_INDETERMINATE) {
        gProgressBarType = PROGRESSBAR_TYPE_INDETERMINATE;
        update_progress_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_show_progress(float portion, int seconds)
{
    pthread_mutex_lock(&gUpdateMutex);
    gProgressBarType = PROGRESSBAR_TYPE_NORMAL;
    gProgressScopeStart += gProgressScopeSize;
    gProgressScopeSize = portion;
    gProgressScopeTime = time(NULL);
    gProgressScopeDuration = seconds;
    gProgress = 0;
    update_progress_locked();
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_set_progress(float fraction)
{
    pthread_mutex_lock(&gUpdateMutex);
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    if (gProgressBarType == PROGRESSBAR_TYPE_NORMAL && fraction > gProgress) {
        // Skip updates that aren't visibly different.
        int width = gr_get_width(gProgressBarIndeterminate[0]);
        float scale = width * gProgressScopeSize;
        if ((int) (gProgress * scale) != (int) (fraction * scale)) {
            gProgress = fraction;
            update_progress_locked();
        }
    }
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_reset_progress()
{
    pthread_mutex_lock(&gUpdateMutex);
    gProgressBarType = PROGRESSBAR_TYPE_NONE;
    gProgressScopeStart = gProgressScopeSize = 0;
    gProgressScopeTime = gProgressScopeDuration = 0;
    gProgress = 0;
    update_screen_locked();
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_print(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);

    fputs(buf, stdout);

    // This can get called before ui_init(), so be careful.
    pthread_mutex_lock(&gUpdateMutex);
printf("vvvvv %s in\n",__func__);
    if (text_rows > 0 && text_cols > 0) {
        char *ptr;
        for (ptr = buf; *ptr != '\0'; ++ptr) {
            if (*ptr == '\n' || text_col >= text_cols) {
                text[text_row][text_col] = '\0';
                text_col = 0;
                text_row = (text_row + 1) % text_rows;
                if (text_row == text_top) text_top = (text_top + 1) % text_rows;
            }
            if (*ptr != '\n') text[text_row][text_col++] = *ptr;
        }
        text[text_row][text_col] = '\0';
        update_screen_locked();
    }
printf("vvvvv %s out\n",__func__);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_start_menu(char** headers, char** items, int initial_selection) {
    int i;
    pthread_mutex_lock(&gUpdateMutex);
    if (text_rows > 0 && text_cols > 0) {
        for (i = 0; i < text_rows; ++i) {
            if (headers[i] == NULL) break;
            strncpy(menu[i], headers[i], text_cols-1);
            menu[i][text_cols-1] = '\0';
        }
        menu_top = i;
        for (; i < text_rows; ++i) {
            if (items[i-menu_top] == NULL) break;
            strncpy(menu[i], items[i-menu_top], text_cols-1);
            menu[i][text_cols-1] = '\0';
        }
        menu_items = i - menu_top;
        show_menu = 1;
        menu_sel = initial_selection;
        update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}

int ui_menu_select(int sel) {
    int old_sel;
    pthread_mutex_lock(&gUpdateMutex);
    if (show_menu > 0) {
        old_sel = menu_sel;
        menu_sel = sel;
    #if 0
        if (menu_sel < 0) menu_sel = 0;
        if (menu_sel >= menu_items) menu_sel = menu_items-1;
    #else
        if (menu_sel < 0) {
            menu_sel = menu_items-1;//这里表示如果当前项为首项，按键操作为向上切换时，切换至尾项
        } else if (menu_sel >= menu_items) {
            menu_sel = 0;//这里表示如果当前项为尾项，按键操作为向下切换时，切换至首项
        }
    #endif
        sel = menu_sel;
        if (menu_sel != old_sel) update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
    return sel;
}

void ui_end_menu() {
    int i;
    pthread_mutex_lock(&gUpdateMutex);
    if (show_menu > 0 && text_rows > 0 && text_cols > 0) {
        show_menu = 0;
        update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}

int ui_text_visible()
{
    pthread_mutex_lock(&gUpdateMutex);
    int visible = show_text;
    pthread_mutex_unlock(&gUpdateMutex);
    return visible;
}

void ui_show_text(int visible)
{
    pthread_mutex_lock(&gUpdateMutex);
    show_text = visible;
    update_screen_locked();
    pthread_mutex_unlock(&gUpdateMutex);
}

int ui_wait_key()
{
    pthread_mutex_lock(&key_queue_mutex);
    while (key_queue_len == 0) {
        pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
    }

    int key = key_queue[0];
    memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    pthread_mutex_unlock(&key_queue_mutex);
    return key;
}

int ui_key_pressed(int key)
{
    // This is a volatile static array, don't bother locking
    return key_pressed[key];
}

void ui_clear_key_queue() {
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}

void ui_print_init(void)  //add by yxj
{
	int i = 0;

	text_col = text_row = 0;
	text_rows = gr_fb_height() / CHAR_HEIGHT;
	if (text_rows > MAX_ROWS)
		text_rows = MAX_ROWS;
	text_top = 1;
    text_cols = gr_fb_width() / CHAR_WIDTH;
	if (text_cols > MAX_COLS - 1)
		text_cols = MAX_COLS - 1;

	//set_theme("0"); //set r g b a

	LOGI("text_col:%d>>text_row:%d>>text_cols:%d>>text_rows:%d\n",text_col,
		text_row,text_cols,text_rows);

	for(i=0 ;i< text_rows;i++)
	{
		itemsInfo[i].t_row = i;
	}

	gUiInitialized = 1;
}

void drawline(int r,int g,int b,int a,int x0,int y0,int x1,int y1,int linewidth)
{
	lines_count += 1;
	if(lines_count > kMaxTiles)
	{
		lines_count = kMaxTiles;
	}
	lines[lines_count-1].x0 = x0;
	lines[lines_count-1].y0 = y0;
	lines[lines_count-1].x1 = x1;
	lines[lines_count-1].y1 = y1;
	lines[lines_count-1].linewidth = linewidth;
	lines[lines_count-1].r = r;
	lines[lines_count-1].g = g;
	lines[lines_count-1].b = b;
	lines[lines_count-1].a = a;
}
void drawline_4(int r,int g,int b,int a,int left,int top,int width,int height,int linewidth)
{
	drawline(r,g,b,a,left,top,left+width,top,linewidth);
	drawline(r,g,b,a,left+width,top,left+width,top+height,linewidth);
	drawline(r,g,b,a,left+width,top+height,left,top+height,linewidth);
	drawline(r,g,b,a,left,top+height,left,top,linewidth);
	pthread_mutex_lock(&gUpdateMutex);
	update_screen_locked();
	pthread_mutex_unlock(&gUpdateMutex);
}

void ui_print_xy_rgba(int t_col,int t_row,int r,int g,int b,int a,const char* fmt, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 512, fmt, ap);
	va_end(ap);

	/*fputs(buf, stdout);*/

    /*This can get called before ui_init(), so be careful.*/
    int temp_row = t_row;
    pthread_mutex_lock(&gUpdateMutex);
    t_col+=2;

	/*1080P screen : text_rows is:18 , text_cols is: 64*/
    if (text_rows > 0 && text_cols > 0)
    {
		char *ptr;
		for (ptr = buf; *ptr != '\0'; ++ptr)
		{
		    /*if (*ptr == '\n' || text_col >= text_cols)*/
		    if (*ptr == '\n' || text_col >= text_cols*2) {
				text[temp_row][text_col] = '\0';
				text_col = 0;

				if (temp_row < text_rows) {
					itemsInfo[temp_row].t_col = t_col;
					itemsInfo[temp_row].t_row = temp_row;

				} else {
					itemsInfo[temp_row].t_col =
						t_col+text_cols/2;
					itemsInfo[temp_row].t_row =
						(temp_row+2)%text_rows;
				}
				itemsInfo[temp_row].r = r;
				itemsInfo[temp_row].g = g;
				itemsInfo[temp_row].b = b;
				itemsInfo[temp_row].a = a;

				temp_row = temp_row + 1;
				if (temp_row >= (2*text_rows))
					temp_row = 0;

				if (temp_row > text_row)
					text_row = temp_row;
		    }
		    if (*ptr != '\n') {
				text[temp_row][text_col++] = *ptr;
		    }
		}
	        text[text_row][text_col] = '\0';
	        update_screen_locked();
    }

    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_print_xy_rgba_multi(struct display_info *info, int count)
{
	int i = 0;

	/*This can get called before ui_init(), so be careful.*/
	pthread_mutex_lock(&gUpdateMutex);
	if (text_rows > 0 && text_cols > 0)	{
		for (i = 0; i < count; i++) {
			int temp_row = info[i].row;
			int t_col = info[i].col+2;
			char *buf = info[i].string;
			char *ptr;

			for (ptr = buf; *ptr != '\0'; ++ptr) {
				if (*ptr == '\n' || text_col >= text_cols*2) {
					text[temp_row][text_col] = '\0';
					text_col = 0;
					if (temp_row < text_rows) {
						itemsInfo[temp_row].t_col =
							t_col;
						itemsInfo[temp_row].t_row =
							temp_row;
					} else {
						itemsInfo[temp_row].t_col =
							t_col+text_cols/2;
						itemsInfo[temp_row].t_row =
							(temp_row+2)%text_rows;
					}
					itemsInfo[temp_row].r = info[i].r;
					itemsInfo[temp_row].g = info[i].g;
					itemsInfo[temp_row].b = info[i].b;
					itemsInfo[temp_row].a = info[i].a;

					temp_row = temp_row + 1;
					if (temp_row >= (2*text_rows))
						temp_row = 0;
					if (temp_row > text_row)
						text_row = temp_row;
				}
			    if (*ptr != '\n')
					text[temp_row][text_col++] = *ptr;
			}
			/*text[text_row][text_col] = '\0';*/
		}
	    update_screen_locked();
	}
	pthread_mutex_unlock(&gUpdateMutex);
}

void ui_display_sync(int t_col,int t_row,int r,int g,int b,int a,const char* fmt, ...)
{
	char buf[512];
	va_list ap;

	if(touch_tp_state == 0)
	{
		va_start(ap, fmt);
		vsnprintf(buf, 512, fmt, ap);
		va_end(ap);
		ui_print_xy_rgba(t_col,t_row,r,g,b,a,buf);
	}
}

void FillColor(int r,int g,int b,int a,int left,int top,int width,int height)
{
	tiles_count += 1;
	if(tiles_count > kMaxTiles)
	{
		tiles_count = kMaxTiles;
	}
	tiles[tiles_count-1].left = left;
	tiles[tiles_count-1].top = top;
	tiles[tiles_count-1].right = width+left;
	tiles[tiles_count-1].bottom = height+top;
	tiles[tiles_count-1].r = r;
	tiles[tiles_count-1].g = g;
	tiles[tiles_count-1].b = b;
	tiles[tiles_count-1].a = a;
	pthread_mutex_lock(&gUpdateMutex);
	update_screen_locked();
	pthread_mutex_unlock(&gUpdateMutex);
}



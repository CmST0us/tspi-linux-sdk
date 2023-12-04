/*
 *  Copyright (c) 2005-2017 Fuzhou Rockchip Electronics Co.Ltd
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "InfoLed.h"

#if 1
struct led_ctrl_data {
    uint32_t trigger;
    /* the delay time(ms) of triggering a trigger */
    uint32_t delayed_trigger_ms;
    uint32_t brightness;
    uint32_t delay_on;
    uint32_t delay_off;
};

struct led_ctrl_scroll_data {
    uint64_t init_bitmap;
    /* the shift bits on every scrolling time*/
    uint32_t shifts;
    uint32_t shift_delay_ms;
};

struct led_ctrl_breath_data {
    uint64_t background_bitmap;
    uint64_t breath_bitmap;
    uint32_t change_delay_ms;
    uint32_t breath_steps;
};

#define bits(nr)	((uint64_t)1 << nr)
static int leds_num = 0;
static int multi_ctrl_fd = -1;
static struct led_ctrl_data *led_multi_data;
static struct led_ctrl_scroll_data scroll_data;
static struct led_ctrl_breath_data breath_data;
pthread_mutex_t led_mutex;

//one led contain 3 color, each of color has a independent led_ctrl_data
struct led_ctrl_data led_multi_mode[LED_MULTI_PURE_COLOR_MAX][3] =
{
    //green color
    {
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    //red color
    {
        {0, 0, 0, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {0, 0, 0, 0, 0},
    },
    //blue color
    {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
    },
    //white color
    {
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
    },
    //black color
    {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
    //non-green color
    {
        {0, 0, 0, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
    },
    //non-red color
    {
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {0, 0, 0, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
    },
    //non-blue color
    {
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {MULTI_TRIGGER_DEFAULT_ON, 0, LED_BRIGHTNESS_FULL, 0, 0},
        {0, 0, 0, 0, 0},
    },
};

static uint64_t leds_color_bitmap[LED_MULTI_PURE_COLOR_MAX] = {
    //green color
    1<<0,
    //red color
    1<<1,
    //blue color
    1<<2,
    //white color
    1<<0 | 1<<1 | 1<<2,
    //black color
    0,
    //non-green color
    1<<1 | 1<<2,
    //non-red color
    1<<0 | 1<<2,
    //non-blue color
    1<<0 | 1<<1,
};

std::unique_ptr<std::thread> m_led_thread;
std::unique_ptr<std::thread> m_led_system_start_t;

#define ONE_LED_ON(multi_data)		\
    multi_data->trigger = MULTI_TRIGGER_DEFAULT_ON; \
multi_data->delayed_trigger_ms = 0; \
multi_data->brightness = LED_BRIGHTNESS_FULL; \
multi_data->delay_on = 0;		\
multi_data->delay_off = 0;
#define ONE_LED_OFF(multi_data)		\
    multi_data->trigger = MULTI_TRIGGER_NONE; \
multi_data->delayed_trigger_ms = 0; \
multi_data->brightness = 0; \
multi_data->delay_on = 0;		\
multi_data->delay_off = 0;

#define ONE_LED_TIMER(multi_data, delayed_trig, delay_on, delay_off)		            \
    struct led_ctrl_data multi_data[LED_MULTI_PURE_COLOR_MAX][3] = 	                    \
{															                            \
    {														                            \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {0, 0, 0, 0, 0},											                    \
        {0, 0, 0, 0, 0},											                    \
    },	                                                                                \
    {	                                                                                \
        {0, 0, 0, 0, 0},	                                                            \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {0, 0, 0, 0, 0},	                                                            \
    },	                                                                                \
    {	                                                                                \
        {0, 0, 0, 0, 0},	                                                            \
        {0, 0, 0, 0, 0},	                                                            \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
    },	                                                                                \
    {	                                                                                \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
    },	                                                                                \
    {	                                                                                \
        {0, 0, 0, 0, 0},	                                                            \
        {0, 0, 0, 0, 0},	                                                            \
        {0, 0, 0, 0, 0},	                                                            \
    },	                                                                                \
    {	                                                                                \
        {0, 0, 0, 0, 0},											                    \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
    },	                                                                                \
    {	                                                                                \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {0, 0, 0, 0, 0},											                    \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
    },	                                                                                \
    {	                                                                                \
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {MULTI_TRIGGER_TIMER, delayed_trig, LED_BRIGHTNESS_FULL, delay_on, delay_off},	\
        {0, 0, 0, 0, 0},											                    \
    },	                                                                                \
};


int dbg_print_ctrl_data(int index)
{
    int i;
    struct led_ctrl_data *d = led_multi_data;

    for (i=index*3;i < (index+1)*3; i++)
        fprintf(stderr,"index=%d,trig=%d,brightness=%d,delay_trig=%d,delay-off=%d,delay-on=%d\n",
                i,d[i].trigger,d[i].brightness,d[i].delayed_trigger_ms,d[i].delay_off,d[i].delay_on);
    return 0;
}

int InfoLed::leds_multi_init(void)
{
    multi_ctrl_fd = open(LEDS_MULTI_CTRL_FILE, O_RDONLY);
    if (multi_ctrl_fd < 0) {
        fprintf(stderr,"%s,can't open file %s\n",__func__, LEDS_MULTI_CTRL_FILE);
        return -1;
    }

    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_GET_LED_NUMBER, &leds_num);
    if (leds_num <= 0){
        fprintf(stderr,"%s,the led number get from kernel is invalid\n",__func__);
        return -1;
    }

    led_multi_data = (struct led_ctrl_data *)malloc(sizeof(struct led_ctrl_data)*leds_num);
    if (led_multi_data ==NULL) {
        fprintf(stderr,"%s,out of memory\n",__func__);
        return -1;
    }
    fprintf(stderr,"%s,the led number is %d\n",__func__,leds_num);

    return 0;
}

int InfoLed::leds_multi_set_one_led(int index, int color)
{
    memcpy(&led_multi_data[index*3], led_multi_mode[color], sizeof(struct led_ctrl_data) * 3);

    return 0;
}

int InfoLed::leds_multi_all_on(int color)
{
    int i;

    for (i = 0; i < leds_num / 3; i++) {
        leds_multi_set_one_led(i, color);
    }

    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);

    return 0;
}

int InfoLed::leds_multi_all_off(void)
{
    memset(led_multi_data, 0, sizeof(struct led_ctrl_data)*leds_num);
    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);

    return 0;
}

/*int InfoLed::leds_multi_scroll(int bg_color, int color, )
  {
  memset(led_multi_data, 0, sizeof(struct led_ctrl_data)*leds_num);
  ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);
  }
  */
int InfoLed::leds_multi_certain_on(int idx_bitmap, int color)
{
    int i;

    memset(led_multi_data, 0, sizeof(struct led_ctrl_data)*leds_num);

    for (i = 0; i < leds_num / 3; i++) {
        if (idx_bitmap & 1 << i)
            leds_multi_set_one_led(i, color);
    }

    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);

    return 0;
}

int InfoLed::leds_multi_certain_on_with_bgcolor(int idx_bitmap, int bg_color, int color)
{
    int i;

    memset(led_multi_data, 0, sizeof(struct led_ctrl_data)*leds_num);
    for (i = 0; i < leds_num / 3; i++) {
        if (idx_bitmap & 1 << i)
            leds_multi_set_one_led(i, color);
        else
            leds_multi_set_one_led(i, bg_color);
    }
    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);

    return 0;
}

int InfoLed::leds_multi_all_blink(unsigned int color, unsigned int delay_on, unsigned int delay_off)
{
    int i;

    ONE_LED_TIMER(multi_data, 0, delay_on, delay_off);

    for (i = 0; i < leds_num / 3; i++) {
        memcpy(&led_multi_data[i*3], multi_data[color], sizeof(struct led_ctrl_data) * 3);
    }

    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET, led_multi_data);

    return 0;
}

int InfoLed::leds_multi_set_scroll(uint64_t bitmap_color, uint32_t color, uint32_t  bg_color, uint32_t shift, uint32_t delay_ms)
{
    int i;

    memset(&scroll_data, 0, sizeof(scroll_data));

    for (i = 0 ;i < leds_num/3;i++) {
        if (bitmap_color & bits(i))
            scroll_data.init_bitmap |= leds_color_bitmap[color] << (i*shift);
        else
            scroll_data.init_bitmap |= leds_color_bitmap[bg_color] << (i*shift);
    }

    scroll_data.shifts = shift;
    scroll_data.shift_delay_ms = delay_ms;
    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET_SCROLL, &scroll_data);

    return 0;
}

int InfoLed::leds_multi_set_breath(uint32_t breath_color, uint32_t bg_color, uint32_t delay_ms, uint32_t steps)
{
    int i;

    memset(&breath_data, 0, sizeof(breath_data));
    for (i = 0 ;i < leds_num/3; i++) {
        breath_data.breath_bitmap |= leds_color_bitmap[breath_color] << i*3;
        breath_data.background_bitmap |= leds_color_bitmap[bg_color] << i*3;
    }
    breath_data.change_delay_ms = delay_ms;
    breath_data.breath_steps = steps;
    ioctl(multi_ctrl_fd, LEDS_MULTI_CTRL_IOCTL_MULTI_SET_BREATH, &breath_data);

    return 0;
}

#endif

InfoLed::InfoLed() :
    m_isRunning{false},
    m_isLedInitialized{false},
    m_currentState{State::IDLE},
    m_oldState{State::IDLE},
    m_currentVolume{0},
    m_oldVolume{0},
    m_isVolAdjust{false},
    m_isWifiError{false},
    m_needSaveVol{false} {

    }

InfoLed::~InfoLed() {
    m_isRunning = false;
    leds_multi_all_off();
}

void InfoLed::init() {
    m_isRunning = true;
    pthread_mutex_init (&led_mutex,NULL);
    m_led_thread = std::make_unique<std::thread>(&InfoLed::mainloop,this);
    m_led_thread->detach();
    m_led_system_start_t = std::make_unique<std::thread>(&InfoLed::led_system_start_t,this);
    m_led_system_start_t->detach();
}

void InfoLed::led_system_start_t() {
    struct timeval tv_now;

    while(m_isRunning) {
        pthread_mutex_lock(&led_mutex);

        if (m_isVolAdjust) {
            gettimeofday(&tv_now,0);
            if (tv_now.tv_sec - m_tvVolAdjust.tv_sec >= 5) { //if time is 5 seconds after set volume, then close the led
                setState(m_ledState);//State::OFF);
                mStateChange.notify_one();
                fprintf(stderr,"save volume m_currentVolume:%d,line(%d)\n",m_currentVolume,__LINE__);
                //save volume
                FILE *devInfo = fopen("/data/cfg/device_info.txt","rb+");
                char tmpbuf[128] = {0};
                if(devInfo) {
                    sprintf(tmpbuf, "%d", m_currentVolume);
                    fseek(devInfo, 25, SEEK_SET);
                    fprintf(stderr,"tmpbuf:%ld\n", strlen(tmpbuf)+1);
                    fwrite(tmpbuf,1,strlen(tmpbuf)+1,devInfo);
                    fprintf(stderr,"tmpbuf:%s\n",tmpbuf);
                    fflush(devInfo);
                    fsync(fileno(devInfo));
                    fclose(devInfo);
                }
                system("cp /data/cfg/device_info.txt /data/cfg/device_info");
                m_isVolAdjust = false;
            }
        }

        if (m_needSaveVol) {
            fprintf(stderr,"save volume m_currentVolume:%d,line(%d)\n",m_currentVolume,__LINE__);
            //save volume
            FILE *devInfo = fopen("/data/cfg/device_info.txt","rb+");
            char tmpbuf[128] = {0};
            if(devInfo) {
                sprintf(tmpbuf,"%d",m_currentVolume);
                fseek(devInfo,25,SEEK_SET);
                fprintf(stderr,"tmpbuf:%ld\n",strlen(tmpbuf)+1);
                fwrite(tmpbuf,1,strlen(tmpbuf)+1,devInfo);
                fprintf(stderr,"tmpbuf:%s\n",tmpbuf);
                fflush(devInfo);
                fsync(fileno(devInfo));
                fclose(devInfo);
            }
            system("cp /data/cfg/device_info.txt /data/cfg/device_info");
            m_needSaveVol = false;
        }

        if (m_isWifiError) {
            gettimeofday(&tv_now,0);
            if (tv_now.tv_sec - m_tvWifiError.tv_sec >= 5) { //if time is 10 seconds after set wifi error, then close the led
                fprintf(stderr,"-----wifi error leds OFF-------\n");
                setState(State::OFF);//State::OFF);
                mStateChange.notify_one();
            }
        }
        pthread_mutex_unlock(&led_mutex);
        usleep(100*1000);
    }
}

void InfoLed::mainloop() {
    std::unique_lock<std::mutex> lck(m_mtx);

    m_oldState = State::OFF;
    m_ledState = State::NORMAL_MODE;
    m_currentState = State::OFF;//State::LED_SCROLLING;
    m_scrollLedNum = 3;

    auto checkState = [this] {
        return m_currentState != m_oldState;

    };

    if(leds_multi_init()) {
        m_isLedInitialized = false;
        return;
    }

    m_isLedInitialized = true;
    while(m_isRunning) {
        uint32_t bitmap = 0;
        mStateChange.wait(lck, checkState);

        pthread_mutex_lock(&led_mutex);
        if (m_currentState != State::ADJUST_VOLUME && m_currentState != State::IDLE && m_currentState != State::OFF && m_isVolAdjust) {
            m_isVolAdjust = false;
            m_needSaveVol = true;
        }

        if (m_currentState != State::WIFI_ERR && m_currentState != State::IDLE) {
            m_isWifiError = false;
        }

        //to avoid led scrolling after system is ok
        if (m_scrollLedNum == 0 && m_currentState == State::LED_SCROLLING) {
            m_currentState = State::IDLE;
        }

        m_oldState = m_currentState;
        switch (m_currentState) {
        case State::LED_SCROLLING:
            gettimeofday(&m_tvLedScroll,0);
            bitmap = (1 << m_scrollLedNum) -1;
            fprintf(stderr,"-----mode LED_SCROLLING,bitmap =0x%x-------\n",bitmap);
            leds_multi_set_scroll(bitmap,LED_MULTI_PURE_COLOR_WHITE,LED_MULTI_PURE_COLOR_BLUE, 3, 80);
            setState(State::IDLE);
            break;
        case State::SYSTEM_OK:
            fprintf(stderr,"-----mode SYSTEM_OK\n");
            leds_multi_all_on(LED_MULTI_PURE_COLOR_WHITE);
            setState(State::IDLE);
            break;
        case State::SYSTEM_START:
            fprintf(stderr,"-----mode SYSTEM_START\n");
            leds_multi_all_off();
            setState(State::IDLE);
            break;
        case State::WIFI_MODE:
            fprintf(stderr,"-----mode WIFI_MODE\n");
            m_ledState = State::WIFI_MODE;
            bitmap = 0x1f;
            leds_multi_set_scroll(bitmap,LED_MULTI_PURE_COLOR_NON_GREEN,LED_MULTI_PURE_COLOR_BLACK, 3, 100);
            setState(State::IDLE);
            break;
        case State::BT_MODE:
            fprintf(stderr,"-----mode BT_MODE\n");
            m_ledState = State::BT_MODE;
            bitmap = 0x1f;
            leds_multi_set_scroll(bitmap,LED_MULTI_PURE_COLOR_GREEN,LED_MULTI_PURE_COLOR_BLACK, 3, 100);
            setState(State::IDLE);
            break;
        case State::NORMAL_MODE:
            fprintf(stderr,"-----mode NORMAL_MODE\n");
            m_ledState = State::NORMAL_MODE;
            leds_multi_all_off();
            setState(State::IDLE);
            break;
        case State::WIFI_ERR:
            fprintf(stderr,"-----mode WIFI_ERROR\n");
            gettimeofday(&m_tvWifiError,0);
            m_isWifiError = true;
            bitmap = 0x1f;
            leds_multi_set_scroll(bitmap,LED_MULTI_PURE_COLOR_RED,LED_MULTI_PURE_COLOR_BLACK, 3, 100);
            setState(State::IDLE);
            break;
        case State::VP_WAKEUP:
            fprintf(stderr,"-----mode VP_WAKEUP\n");
            leds_multi_set_breath(LED_MULTI_PURE_COLOR_WHITE ,LED_MULTI_PURE_COLOR_BLACK, 200, 8);
            setState(State::IDLE);
            break;
        case State::VP_PROCESS:
            fprintf(stderr,"-----mode VP_PROCESS\n");
            bitmap = 0x333;
            leds_multi_set_scroll(bitmap,LED_MULTI_PURE_COLOR_WHITE,LED_MULTI_PURE_COLOR_BLUE, 3, 50);
            setState(State::IDLE);
            break;
        case State::ADJUST_VOLUME:
            fprintf(stderr,"-----mode ADJUST_VOLUME:%d-------\n",m_currentVolume);
            gettimeofday(&m_tvVolAdjust,0);
            m_isVolAdjust = true;
            leds_multi_certain_on((1 << m_currentVolume) -1, LED_MULTI_PURE_COLOR_WHITE);
            setState(State::IDLE);
            m_oldVolume = m_currentVolume;
            break;
        case State::MIC_MUTE:
            fprintf(stderr,"-----mode MIC_MUTE-------\n");
            m_ledState = State::MIC_MUTE;
            leds_multi_all_on(LED_MULTI_PURE_COLOR_RED);
            setState(State::IDLE);
            break;
        case State::OFF:
            fprintf(stderr,"-----mode OFF-------\n");
            if (m_isVolAdjust) {
                fprintf(stderr,"-----mode OFF while adjust volume-------\n");
                break;
            }

            leds_multi_all_off();
            setState(State::IDLE);
            break;
        case State::IDLE:
            break;

        default:
            break;
        }
        pthread_mutex_unlock(&led_mutex);
    }
}

void InfoLed::setState(State state) {
    m_currentState = state;
}

void InfoLed::led_open(int mode, int val) {
    if (!m_isLedInitialized){
        return;
    }
    pthread_mutex_lock(&led_mutex);
    switch(mode) {
    case MODE_BOOTED:
        m_scrollLedNum = 3;
        setState(State::LED_SCROLLING);
        mStateChange.notify_one();
        break;
    case MODE_NETWORK_STARTED:
        m_scrollLedNum = 8;
        setState(State::LED_SCROLLING);
        mStateChange.notify_one();
        break;
    case MODE_JAVACLIENT_STARTED:
        m_scrollLedNum = 4;
        setState(State::LED_SCROLLING);
        mStateChange.notify_one();
        break;
    case MODE_SENSORY_STARTED:
        m_scrollLedNum = 6;
        setState(State::LED_SCROLLING);
        mStateChange.notify_one();
        break;
    case MODE_SYSTEM_OK:
        m_scrollLedNum = 0;
        setState(State::SYSTEM_OK);
        mStateChange.notify_one();
        break;
    case MODE_START:
        setState(State::SYSTEM_START);
        mStateChange.notify_one();
        break;
    case MODE_OFF:
        setState(State::OFF);
        mStateChange.notify_one();
        //led_all_off();
        break;
    case MODE_VP:
        setState(State::VP_PROCESS);
        mStateChange.notify_one();
        break;
    case MODE_VP_WAKEUP:
        setState(State::VP_WAKEUP);
        mStateChange.notify_one();
        break;
    case MODE_WIFI_CONNECT:
        //led_all_on(COLOR_GREEN);
        setState(State::WIFI_MODE);
        mStateChange.notify_one();
        break;
    case MODE_BLUETOOTH:
        //led_all_on(COLOR_BLUE);
        setState(State::BT_MODE);
        mStateChange.notify_one();
        break;
    case MODE_NORMAL:
        //led_all_on(COLOR_BLUE);
        setState(State::NORMAL_MODE);
        mStateChange.notify_one();
        break;
    case MODE_VOLUME:
        m_currentVolume = val;
        setState(State::ADJUST_VOLUME);
        mStateChange.notify_one();
        break;
    case MODE_WIFI_ERR:
        setState(State::WIFI_ERR);
        mStateChange.notify_one();
        break;
    case MODE_MIC_MUTE:
        setState(State::MIC_MUTE);
        mStateChange.notify_one();
        break;
    case MODE_TEST:
        break;
    default:
        fprintf(stderr,"mode is not support");
        break;
    }
    pthread_mutex_unlock(&led_mutex);
}

static int pcba_cx20921_upgrade()
{
    int ret = -1;
    char buf[256] = {0};
    char name[256] = "/data/cx20921/i2c_flash -d /dev/i2c-2 -g 28 -f /data/cx20921/evk-nebula-generic.sfs /data/cx20921/iflash.bin";
    FILE *pFile = NULL;

    printf("cmd = %s\n",name);
    pFile = popen(name, "r");
    if (pFile != NULL)  {
        while (fgets(buf, sizeof(buf), pFile)) {
            if (strstr(buf, "Firmware Downloaded Successfully")){
                printf("cx20921 fw downloading is finished\n");
                ret = 0;
            }
            printf("%s\n",buf);
        }
        pclose(pFile);
    }
    return ret;
}

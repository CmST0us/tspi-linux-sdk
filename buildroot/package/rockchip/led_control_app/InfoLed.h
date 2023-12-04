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

#ifndef __RK_LEDS_H__
#define __RK_LEDS_H__

#ifdef __cplusplus
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <time.h>
#endif

#if 1
#define LEDS_MULTI_CTRL_FILE	"/dev/led_multi_ctrl"

#define LEDS_MULTI_CTRL_IOCTL_MAGIC			'z'

#define LEDS_MULTI_CTRL_IOCTL_MULTI_SET	\
    _IOW(LEDS_MULTI_CTRL_IOCTL_MAGIC, 0x01, struct led_ctrl_data*)
#define LEDS_MULTI_CTRL_IOCTL_GET_LED_NUMBER	\
    _IOR(LEDS_MULTI_CTRL_IOCTL_MAGIC, 0x02, int)
#define LEDS_MULTI_CTRL_IOCTL_MULTI_SET_SCROLL	\
    _IOW(LEDS_MULTI_CTRL_IOCTL_MAGIC, 0x03, struct led_ctrl_scroll_data*)
#define LEDS_MULTI_CTRL_IOCTL_MULTI_SET_BREATH	\
    _IOW(LEDS_MULTI_CTRL_IOCTL_MAGIC, 0x04, struct led_ctrl_breath_data*)

#define LED_BRIGHTNESS_FULL 255
#define LED_BRIGHTNESS_HALF 127
#define LED_BRIGHTNESS_OFF 0

enum {
    MULTI_TRIGGER_NONE = 0,
    MULTI_TRIGGER_DEFAULT_ON,
    MULTI_TRIGGER_TIMER,
    MULTI_TRIGGER_ONESHOT,
    MULTI_TRIGGER_MAX,
};

enum {
    LED_MULTI_PURE_COLOR_GREEN = 0,
    LED_MULTI_PURE_COLOR_RED,
    LED_MULTI_PURE_COLOR_BLUE,
    LED_MULTI_PURE_COLOR_WHITE,
    LED_MULTI_PURE_COLOR_BLACK,
    LED_MULTI_PURE_COLOR_NON_GREEN,
    LED_MULTI_PURE_COLOR_NON_RED,
    LED_MULTI_PURE_COLOR_NON_BLUE,
    LED_MULTI_PURE_COLOR_MAX,
};
#endif

#define NUMBER_OF_LEDS		12
typedef enum {
    MODE_START=1,
    MODE_OFF,
    MODE_VP,
    MODE_VP_WAKEUP,
    MODE_NORMAL,
    MODE_WIFI_CONNECT,
    MODE_BLUETOOTH,
    MODE_AUXILIARY,
    MODE_VOLUME,
    MODE_WIFI_ERR,
    MODE_MIC_MUTE,
    MODE_TEST,
    MODE_BOOTED,
    MODE_NETWORK_STARTED,
    MODE_JAVACLIENT_STARTED,
    MODE_SENSORY_STARTED,
    MODE_SYSTEM_OK,
    MODE_MAX,
} L_color;


class InfoLed {
public:
    InfoLed();
    ~InfoLed();
    void init();
    void mainloop();
    void led_open(int mode,int val);
    int led_all_on(int color);
    void led_all_off();
//private:
    enum class State {
        UNINITIALIZED = 1,            // agent is not initialized.
        OFF,
        IDLE,                         // initialized, but idle.
        SYSTEM_START,                 // The agent has detected the wake word.
        NORMAL_MODE,
        BT_MODE,
        WIFI_MODE,
        WIFI_ERR,
        VP_WAKEUP,                    // wake word has been detected, and an IPC
        VP_PROCESS,                   // we have received a request to resume.
        ADJUST_VOLUME,
        LED_SCROLLING,
        MIC_MUTE,
        SYSTEM_OK,
    };

    void setState(State state);
    void led_system_start_t();
    void set_scroll_duration(int ms);
    int get_scroll_duration();

    int leds_multi_init(void);
    int leds_multi_all_on(int color);
    int leds_multi_set_one_led(int index, int color);
    int leds_multi_all_off(void);
    int leds_multi_certain_on(int idx_bitmap, int color);
    int leds_multi_certain_on_with_bgcolor(int idx_bitmap, int bg_color, int color);
    int leds_multi_all_blink(unsigned int color,unsigned int delay_on, unsigned int delay_off);
    int leds_multi_set_scroll(uint64_t bitmap_color, uint32_t color, uint32_t  bg_color, uint32_t shift, uint32_t delay_ms);
    int leds_multi_set_breath(uint32_t breath_color, uint32_t bg_color, uint32_t delay_ms, uint32_t steps);

    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_isLedInitialized;
    std::mutex m_mtx;
    std::condition_variable mStateChange;
    State m_currentState;
    State m_oldState;
    State m_ledState;
    int m_currentVolume;
    int m_oldVolume;
    volatile int m_scrollLedNum;
    std::atomic<bool> m_isVolAdjust;
    std::atomic<bool> m_isWifiError;
    std::atomic<bool> m_needSaveVol;

    struct timeval m_tvVolAdjust;
    struct timeval m_tvLedScroll;
    struct timeval m_tvWifiError;
};

#endif

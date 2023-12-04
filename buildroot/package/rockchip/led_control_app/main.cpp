#include <iostream>
#include <memory>
#include <thread>
#include <string>

#include <stdio.h>
#include <unistd.h>

#include "InfoLed.h"

void printf_mode(int mode) {
    printf("mode:\n");
    switch(mode) {
        case 1:
            printf("all on");
            break;
        case 2:
            printf("all off");
            break;
        case 3:
            printf("scroll_wifi");
            break;
        case 4:
            printf("scroll_bt");
            break;
        case 5:
            printf("scroll_3");
            break;
        case 6:
            printf("breath");
            break;
        case 7:
            printf("blink");
            break;
        case 8:
            printf("volume");
            break;
        default:
            printf("the mode is not supported");
    }
    printf("\n");
}
int main(int argc, char** argv)
{
    int ledNumber;
    auto infoled = std::make_shared<InfoLed>();

    if(argc != 2) {
        printf("************************************\n");
        printf("please input LedTest + mode number:\n");
        printf("    support mode(1~8):\n");
        printf("    all on,all off\n");
        printf("    scroll_wifi,scroll_bt,scroll_3,breath,blink,volume\n");
        printf("************************************\n");
        return -1;
    }
    /**
     *
     * enum {
     *     LED_MULTI_PURE_COLOR_GREEN = 0,
     *     LED_MULTI_PURE_COLOR_RED,
     *     LED_MULTI_PURE_COLOR_BLUE,
     *     LED_MULTI_PURE_COLOR_WHITE,
     *     LED_MULTI_PURE_COLOR_BLACK,
     *     LED_MULTI_PURE_COLOR_NON_GREEN,
     *     LED_MULTI_PURE_COLOR_NON_RED,
     *     LED_MULTI_PURE_COLOR_NON_BLUE,
     *     LED_MULTI_PURE_COLOR_MAX,
     * };
     *
     */
    ledNumber = infoled->leds_multi_init();
    if(ledNumber < 0) {
        printf("led init failed\n");
        return -1;
    }

    printf("Led on...\n");
    infoled->leds_multi_all_on(LED_MULTI_PURE_COLOR_WHITE);
    sleep(1);

    int mode = atoi(argv[1]);
    int bitmap;
    int m_scrollLedNum,m_currentVolume;
    printf_mode(mode);

    switch(mode) {
        case 1:
            infoled->leds_multi_all_on(LED_MULTI_PURE_COLOR_RED);
            break;
        case 2:
            infoled->leds_multi_all_off();
            break;
        case 3:
            bitmap = 0x1f;
            infoled->leds_multi_set_scroll(
                    bitmap,LED_MULTI_PURE_COLOR_NON_GREEN,LED_MULTI_PURE_COLOR_BLACK, 3, 100);
            break;
        case 4:
            bitmap = 0x1f;
            infoled->leds_multi_set_scroll(
                    bitmap,LED_MULTI_PURE_COLOR_GREEN,LED_MULTI_PURE_COLOR_BLACK, 3, 100);
            break;
        case 5:
            m_scrollLedNum = 3;
            bitmap = (1 << m_scrollLedNum) -1;
            infoled->leds_multi_set_scroll(
                    bitmap, LED_MULTI_PURE_COLOR_WHITE, LED_MULTI_PURE_COLOR_BLUE, 3, 80);
            break;
        case 6:
            infoled->leds_multi_set_breath(
                    LED_MULTI_PURE_COLOR_WHITE ,LED_MULTI_PURE_COLOR_BLACK, 200, 8);
            break;
        case 7:
            bitmap = 0x333;
            infoled->leds_multi_set_scroll(
                    bitmap,LED_MULTI_PURE_COLOR_WHITE,LED_MULTI_PURE_COLOR_BLUE, 3, 50);
            break;
        case 8:
            m_currentVolume = 3;
            infoled->leds_multi_certain_on(
                    (1 << m_currentVolume) -1, LED_MULTI_PURE_COLOR_WHITE);
            break;
        default:
            break;
    }
    sleep(1);

    printf("Led off...\n");
    infoled->leds_multi_all_off();

    return 0;
}

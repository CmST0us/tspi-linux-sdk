led demo:

please use the following command to compile the demo:
    ./../../../buildroot/output/host/usr/bin/aarch64-rockchip-linux-gnu-g++ main.cpp InfoLed.cpp -pthread -std=c++14 -o LedTest

please input LedTest + mode number to exec the demo
    support mode(1~8):
    all on,all off,scroll_wifi,scroll_bt,scroll_3,breath,blink,volume

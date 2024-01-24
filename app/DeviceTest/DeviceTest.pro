QT       += core gui serialport network multimedia multimediawidgets serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    4G/mobilenet.cpp \
    4G/mobilenetthread.cpp \
    adc/adckey.cpp \
    adc/adcthread.cpp \
    bluetooth/bluetooth.cpp \
    bluetooth/bluetooththread.cpp \
    camera/camera.cpp \
    camera/thread/camerathread1.cpp \
    camera/thread/camerathread2.cpp \
    camera/v4l2/v4l2.c \
    can/cantest.cpp \
    common/common.cpp \
    device/deviceinfo.cpp \
    device/devicethread.cpp \
    ethernet/ethernet.cpp \
    ethernet/ethnetautotest.cpp \
    ethernet/eththread.cpp \
    gpio/gpiotest.cpp \
    led/led.cpp \
    main.cpp \
    mainUI/mainwindow.cpp \
    npu/nputhread.cpp \
    npu/nputest.cpp \
    pwm/pwm.cpp \
    pwm/pwmthread.cpp \
    rgb/lcdrgb.cpp \
    rtc/rtc.cpp \
    rtc/rtcthread.cpp \
    sound/playtimethread.cpp \
    sound/recortTimeThread.cpp \
    sound/sound.cpp \
    stress/stresstest.cpp \
    stress/stressthread.cpp \
    tfcard/tfcard.cpp \
    tfcard/tfcardthread.cpp \
    touch/touch.cpp \
    uart/uart.cpp \
    usb/usbdevices.cpp \
    usb/usbthread.cpp \
    wifi/connectthread.cpp \
    wifi/wifi.cpp \
    wifi/wifithread.cpp \
    wifiStress/FileTransferThread.cpp \
    wifiStress/NetCheckThread.cpp \
    wifiStress/ScanThread.cpp \
    wifiStress/WifiConnectThread.cpp \
    wifiStress/wifistress.cpp

HEADERS += \
    4G/mobilenet.h \
    4G/mobilenetthread.h \
    adc/adckey.h \
    adc/adcthread.h \
    bluetooth/bluetooth.h \
    bluetooth/bluetooththread.h \
    camera/camera.h \
    camera/thread/camerathread1.h \
    camera/thread/camerathread2.h \
    camera/v4l2/config.h \
    camera/v4l2/v4l2.h \
    can/cantest.h \
    common/common.h \
    device/deviceinfo.h \
    device/devicethread.h \
    ethernet/ethernet.h \
    ethernet/ethnetautotest.h \
    ethernet/eththread.h \
    gpio/gpiotest.h \
    led/led.h \
    mainUI/mainwindow.h \
    npu/nputhread.h \
    npu/nputest.h \
    pwm/pwm.h \
    pwm/pwmthread.h \
    rgb/lcdrgb.h \
    rtc/rtc.h \
    rtc/rtcthread.h \
    sound/playtimethread.h \
    sound/recortTimeThread.h \
    sound/sound.h \
    stress/stresstest.h \
    stress/stressthread.h \
    tfcard/tfcard.h \
    tfcard/tfcardthread.h \
    touch/touch.h \
    uart/uart.h \
    usb/usbdevices.h \
    usb/usbthread.h \
    wifi/connectthread.h \
    wifi/wifi.h \
    wifi/wifithread.h \
    wifiStress/FileTransferThread.h \
    wifiStress/NetCheckThread.h \
    wifiStress/ScanThread.h \
    wifiStress/WifiConnectThread.h \
    wifiStress/wifistress.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

#QMAKE_LFLAGS += -no-pie

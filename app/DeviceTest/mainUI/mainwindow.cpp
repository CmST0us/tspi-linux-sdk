#include "mainwindow.h"
#include "rtc/rtc.h"
#include "uart/uart.h"
#include "device/deviceinfo.h"
#include "touch/touch.h"
#include "adc/adckey.h"
#include "pwm/pwm.h"
#include "ethernet/ethernet.h"
#include "wifi/wifi.h"
#include "bluetooth/bluetooth.h"
#include "camera/camera.h"
#include "led/led.h"
#include "sound/sound.h"
#include "4G/mobilenet.h"
#include "tfcard/tfcard.h"
#include "rgb/lcdrgb.h"
#include "usb/usbdevices.h"
#include "gpio/gpiotest.h"
#include "npu/nputest.h"
#include "can/cantest.h"
#include "stress/stresstest.h"
#include "wifiStress/wifistress.h"

QWidget *widget;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //自适应大小
    QScreen *screen = QGuiApplication::primaryScreen();
    windSize = screen->virtualGeometry();

    //default full screen
    resize(windSize.width(), windSize.height());


    //QDesktopWidget* desktop = QApplication::desktop();
    //this->setGeometry(desktop->screenGeometry(0));
    //this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint); // 去掉标题栏,去掉任务栏显示，窗口置顶
    this->initWindow();
}


void MainWindow::initWindow()
{
    QGridLayout *mainLayout = new QGridLayout;
    widget = new QWidget();
    QSettings  *setting = new QSettings(CONFIG_FILE_PATH,QSettings::IniFormat);
    setting->setIniCodec("UTF-8");

    QStringList groups = setting->childGroups();
    int row = 1;
    int index = 0;
    for(int i = 0;i < groups.length(); i++){
        QString enable = setting->value("/"+groups.at(i)+"/enable").toString();
        QString name = setting->value("/"+groups.at(i)+"/name").toString();
        QString state = setting->value("/"+groups.at(i)+"/state").toString();

        if(enable == "1"){
            QPushButton *funBtn = common.getButton(name);
            funBtn->setStyleSheet(funBtn->styleSheet()+"QPushButton{width:200;height:40}");

            if(state == "1")
                funBtn->setStyleSheet(funBtn->styleSheet()+"QPushButton{border-image:url(:/icon/pass.png)}");
            if(state == "0")
                funBtn->setStyleSheet(funBtn->styleSheet()+"QPushButton{border-image:url(:/icon/faild.png)}");

            connect(funBtn, &QPushButton::clicked, [=] { handleClick(funBtn->text()); });

            if(index % 2 == 0){
                mainLayout->addWidget(funBtn, row, 1, Qt::AlignRight);

            }else{
                mainLayout->addWidget(funBtn, row, 2, Qt::AlignLeft);
                row++;
            }

            index++;
        }
    }

    QPushButton *auTestBtn = common.getButton("自动测试");
    connect(auTestBtn, &QPushButton::clicked, [=] { handleAutoTest(); });
    auTestBtn->setStyleSheet(auTestBtn->styleSheet()+"QPushButton{width:460;height:40}");
    mainLayout->addWidget(auTestBtn, ++row, 1, 1, 2, Qt::AlignHCenter);

    mainLayout->setHorizontalSpacing(60);
    widget->resize(windSize.width(), windSize.height());
    widget->setLayout(mainLayout);
    this->setCentralWidget(widget);
}

void MainWindow::updateBtnStyle(QString btnTxt, bool result)
{
    QList<QPushButton *> buttons = centralWidget()->findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        if(btn->text() == btnTxt){
            if (result)
                btn->setStyleSheet(btn->styleSheet()+"QPushButton{border-image:url(:/icon/pass.png)}");
            else
                btn->setStyleSheet(btn->styleSheet()+"QPushButton{border-image:url(:/icon/faild.png)}");
        }
        btn->update();
    }

    // for auto test
    if(autoTest){
        if(queue.isEmpty()){
            autoTest = 0;
            timer->stop();
            return;
        }
        timer->start();
    }
}

void MainWindow::handleAutoTest()
{
    qDebug()<<autoTest;
    autoTest = 1;
    qDebug()<<autoTest;
    QSettings  *setting = new QSettings(CONFIG_FILE_PATH,QSettings::NativeFormat);
    QStringList groups = setting->childGroups();

    for(int i = 0; i < groups.length(); i++){
        for(int j = 0; j < groups.length(); j++){
            int id = setting->value("/"+groups.at(j)+"/id").toInt();
            if ((id - 1) == i){
                QString enable = setting->value("/"+groups.at(j)+"/enable").toString();
                if(enable == "1"){
                    QString name = setting->value("/"+groups.at(j)+"/name").toString();
                    queue.enqueue(name);
                }
            }
        }
    }

    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, [=] {handleClick(queue.dequeue());});
    timer->start();
}

void MainWindow::handleClick(QString str)
{
    if(autoTest){
        timer->stop();
    }

    if(str == "RTC"){
        RTC *rtc = new RTC(this);
        connect(rtc, &RTC::testFinish, [=] { updateBtnStyle(str,rtc->testResult); });
        rtc->initWindow(windSize.width(),windSize.height());
        rtc->showFullScreen();

    }else if (str == "串口") {
        Uart *uart = new Uart(this);
        connect(uart, &Uart::testFinish, [=] { updateBtnStyle(str,uart->testResult); });
        uart->initWindow(windSize.width(),windSize.height());

    }else if (str == "设备信息"){
        DeviceInfo *dev = new DeviceInfo(this);
        connect(dev, &DeviceInfo::testFinish, [=] { updateBtnStyle(str,dev->testResult); });
        dev->initWindow(windSize.width(),windSize.height());

    }else if (str == "触摸"){
        Touch *touch = new Touch(this);
        touch->initWindow(windSize.width(),windSize.height());
        touch->showFullScreen();
        connect(touch, &Touch::testFinish, [=] { updateBtnStyle(str,touch->testResult); });

    }else if(str == "ADC按键"){
        AdcKey *adc = new AdcKey(this);
        adc->initWindow(windSize.width(),windSize.height());
        adc->showFullScreen();
        connect(adc, &AdcKey::testFinish, [=] { updateBtnStyle(str,adc->testResult); });

    }else if(str == "PWM/背光"){
        Pwm *pwm = new Pwm(this);
        pwm->initWindow(windSize.width(),windSize.height());
        pwm->showFullScreen();
        connect(pwm, &Pwm::testFinish, [=] { updateBtnStyle(str,pwm->testResult); });

    }else if(str == "以太网"){
        EtherNet *etherNet = new EtherNet(this);
        etherNet->initWindow(windSize.width(),windSize.height());
        etherNet->showFullScreen();
        connect(etherNet, &EtherNet::testFinish, [=] { updateBtnStyle(str,etherNet->testResult); });

    }else if(str == "WIFI"){
        Wifi *wifi = new Wifi(this);
        wifi->initWindow(windSize.width(),windSize.height());
        wifi->showFullScreen();
        connect(wifi, &Wifi::testFinish, [=] { updateBtnStyle(str,wifi->testResult); });

    }else if(str == "蓝牙"){
        Bluetooth *bt = new Bluetooth(this);
        bt->initWindow(windSize.width(),windSize.height());
        bt->showFullScreen();
        connect(bt, &Bluetooth::testFinish, [=] { updateBtnStyle(str,bt->testResult); });

    }else if(str == "摄像头"){
        Camera *cam = new Camera(this);
        cam->initWindow(windSize.width(),windSize.height());
        connect(cam, &Camera::testFinish, [=] { updateBtnStyle(str,cam->testResult); });
        cam->showFullScreen();

    }else if(str == "补光灯"){
        Led *led = new Led(this);
        led->initWindow(windSize.width(),windSize.height());
        connect(led, &Led::testFinish, [=] { updateBtnStyle(str,led->testResult); });
        led->showFullScreen();

    }else if(str == "声卡"){
        Sound *sound = new Sound(this);
        sound->initWindow(windSize.width(),windSize.height());
        connect(sound, &Sound::testFinish, [=] { updateBtnStyle(str,sound->testResult); });
        sound->showFullScreen();

    }else if(str == "4G"){
        MobileNet *mobileNet = new MobileNet(this);
        mobileNet->initWindow(windSize.width(),windSize.height());
        connect(mobileNet, &MobileNet::testFinish, [=] { updateBtnStyle(str,mobileNet->testResult); });
        mobileNet->showFullScreen();

    }else if(str == "TF卡"){
        TFCard *tfcard = new TFCard(this);
        tfcard->initWindow(windSize.width(),windSize.height());
        connect(tfcard, &TFCard::testFinish, [=] { updateBtnStyle(str,tfcard->testResult); });
        tfcard->showFullScreen();

    }else if(str == "RGB"){
        LcdRGB *rgb = new LcdRGB(this);
        rgb->initWindow(windSize.width(),windSize.height());
        connect(rgb, &LcdRGB::testFinish, [=] { updateBtnStyle(str,rgb->testResult); });
        rgb->showFullScreen();

    }else if(str == "USB"){
        USBDevices *usb = new USBDevices(this);
        usb->initWindow(windSize.width(),windSize.height());
        connect(usb, &USBDevices::testFinish, [=] { updateBtnStyle(str,usb->testResult); });
        usb->showFullScreen();

    }else if(str == "GPIO"){
        GPIOTest *gpio = new GPIOTest(this);
        gpio->initWindow(windSize.width(),windSize.height());
        connect(gpio, &GPIOTest::testFinish, [=] { updateBtnStyle(str,gpio->testResult); });
        gpio->showFullScreen();

    }else if(str == "NPU"){
        NPUTest *npu = new NPUTest(this);
        npu->initWindow(windSize.width(),windSize.height());
        connect(npu, &NPUTest::testFinish, [=] { updateBtnStyle(str,npu->testResult); });
        npu->showFullScreen();

    }else if(str == "CAN"){
        CANTest *can = new CANTest(this);
        can->initWindow(windSize.width(),windSize.height());
        connect(can, &CANTest::testFinish, [=] { updateBtnStyle(str,can->testResult); });
        can->showFullScreen();

    }else if(str == "stress"){
        StressTest *stress = new StressTest(this);
        stress->initWindow(windSize.width(),windSize.height());
        stress->showFullScreen();

    }else if(str == "WIFI压力"){
        WifiStress *wifiStress = new WifiStress(this);
        wifiStress->initWindow(windSize.width(),windSize.height());
        wifiStress->showFullScreen();
    }
}

MainWindow::~MainWindow()
{
}


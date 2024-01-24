#include "usbdevices.h"

USBDevices::USBDevices(QWidget *parent) : QWidget(parent)
{

}

void USBDevices::initWindow(int w, int h)
{
    tips = common.getButton("检测到USB设备个数:");
    hubCnt = common.getButton("检测到HUB设备个数:");
    hubCnt->setStyleSheet(hubCnt->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(faildBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    QGridLayout *layout = new QGridLayout();

    QString powerGPIOs = common.getConfig("USB","power_gpios");
    if(powerGPIOs != ""){
        powerBtn = common.getButton("电源:开");
        connect(powerBtn, &QPushButton::clicked, [=] { this->powerCtrl(); });

        layout->addWidget(tips, 1, 1, 1, 6, Qt::AlignBottom);
        layout->addWidget(hubCnt, 2, 1, 1, 6, Qt::AlignBottom);
        layout->addWidget(powerBtn, 3, 1, 1, 6, Qt::AlignBottom | Qt::AlignHCenter);
        layout->addWidget(passBtn, 4, 1, 1, 3, Qt::AlignBottom);
        layout->addWidget(faildBtn, 4, 4, 1, 3, Qt::AlignBottom);
    }else{
        layout->addWidget(tips, 1, 1, 1, 6, Qt::AlignBottom);
        layout->addWidget(hubCnt, 2, 1, 1, 6, Qt::AlignBottom);
        layout->addWidget(passBtn, 3, 1, 1, 3, Qt::AlignBottom);
        layout->addWidget(faildBtn, 3, 4, 1, 3, Qt::AlignBottom);
    }

    this->setLayout(layout);

    usbThread = new UsbThread();
    connect(usbThread, &UsbThread::getUsbDevices, this, &USBDevices::handleUsbDevices);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);
    usbThread->start();
}

void USBDevices::handleUsbDevices(int hubcnt, int devcnt)
{
    tips->setText("检测到USB设备个数:" + QString::number(devcnt));
    hubCnt->setText("检测到HUB设备个数:" + QString::number(hubcnt));
}

void USBDevices::powerCtrl(){
    QString powerGPIOs = common.getConfig("USB","power_gpios");
    QStringList gpioList = powerGPIOs.split("_");
    if(powerBtn->text() == "电源:开"){
        powerBtn->setText("电源:关");
        foreach (QString str, gpioList) {
            qDebug()<<str;
            if(str == "gpio4c5"){ //otg is inverse
                this->setGPIOValue(str, "1");
            }
            this->setGPIOValue(str, "0");
        }
    }else{
        powerBtn->setText("电源:开");
        foreach (QString str, gpioList) {
            qDebug()<<str;
            if(str == "gpio4c5"){ //otg is inverse
                this->setGPIOValue(str, "0");
            }
            this->setGPIOValue(str, "1");
        }
    }
}

void USBDevices::setGPIOValue(QString gpio, QString value)
{
    QString cmd = QString("echo %1 > /proc/rp_gpio/%2").arg(value).arg(gpio);
    qDebug()<<cmd;
    ::system(cmd.toLocal8Bit());
}

void USBDevices::handleClick(bool result)
{
    usbThread->checkState = false;
    usbThread->quit();
    testResult = result;

    //update config file state
    common.setConfig("USB", "state", QString::number(result));

    emit testFinish();
    this->close();
}

#include "led.h"

Led::Led(QWidget *parent) : QWidget(parent)
{

}

void Led::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout();
    QLabel *irLed = common.getLabel("红外灯：");
    irLed->setAlignment(Qt::AlignRight);
    QLabel *blLed = common.getLabel("补光灯：");
    blLed->setAlignment(Qt::AlignRight);
    QPushButton *tips = common.getButton("自动拉高拉低补光灯和红外灯，请注意观察补光灯和红外灯是否有闪烁！");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QPushButton *pullUpIr = common.getButton("打开");
    QPushButton *pullDownIr = common.getButton("关闭");
    QPushButton *pullUpBl = common.getButton("打开");
    QPushButton *pullDownBl= common.getButton("关闭");
    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    bl_led_ctl = common.getConfig("led","bl_led_ctl");
    ir_led_ctl = common.getConfig("led","ir_led_ctl");

    connect(pullUpIr, &QPushButton::clicked, [=] { this->setValue(ir_led_ctl,1); });
    connect(pullDownIr, &QPushButton::clicked, [=] { this->setValue(ir_led_ctl,0); });
    connect(pullUpBl, &QPushButton::clicked, [=] { this->setValue(bl_led_ctl,1); });
    connect(pullDownBl, &QPushButton::clicked, [=] { this->setValue(bl_led_ctl,0); });

    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(faildBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    layout->addWidget(tips, 1, 1, 1, 4, Qt::AlignVCenter);
    layout->addWidget(irLed, 2, 1, 1, 1, Qt::AlignVCenter);
    layout->addWidget(pullUpIr, 2, 2, 1, 1, Qt::AlignVCenter);
    layout->addWidget(pullDownIr, 2, 3, 1, 1, Qt::AlignVCenter);
    layout->addWidget(blLed, 3, 1, 1, 1, Qt::AlignVCenter);
    layout->addWidget(pullUpBl, 3, 2, 1, 1, Qt::AlignVCenter);
    layout->addWidget(pullDownBl, 3, 3, 1, 1, Qt::AlignVCenter);

    layout->addWidget(passBtn, 4, 1, 1, 2, Qt::AlignBottom);
    layout->addWidget(faildBtn, 4, 3, 1, 2, Qt::AlignBottom);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]  {this->handleTimeout();});
    timer->start(1000);
}

void Led::setValue(QString ledname, int value)
{
    QString cmd = QString("echo %1 > %2").arg(value).arg(ledname);
    qDebug()<<cmd;
    ::system(cmd.toLocal8Bit());
}

void Led::handleTimeout()
{
    setValue(ir_led_ctl, ledValue);
    setValue(bl_led_ctl, ledValue);
    ledValue = !ledValue;
}

void Led::handleClick(bool result)
{
    if(timer){
        timer->stop();
    }
    testResult = result;

    //update config file state
    common.setConfig("led", "state", QString::number(result));

    emit testFinish();
    setValue(ir_led_ctl, 0);
    setValue(bl_led_ctl, 0);
    this->close();
}

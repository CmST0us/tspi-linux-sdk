#include "rtc.h"

RTC::RTC(QWidget *parent) : QWidget(parent)
{

}

void RTC::initWindow(int w, int h)
{
    tips = common.getLabel("正在自动测试RTC，请稍候。。。");
    tips->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPixelSize(16);
    tips->setFont(font);
    closeBtn = common.getButton("关闭窗口");
    setDateBtn = common.getButton("设置RTC时间");
    setDateBtn->setVisible(false);
    connect(setDateBtn, &QPushButton::clicked, [=]{this->setDate();});
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {closeWindow();});

    QGridLayout *layout = new QGridLayout();

    layout->addWidget(tips, 1, 1, 1, 7);
    layout->addWidget(setDateBtn,2, 4, 1, 1);
    layout->addWidget(closeBtn, 3, 4, 1, 1);
    this->setLayout(layout);

    rtcthread = new RTCThread();
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(rtcthread, &RTCThread::getResult, this, &RTC::handleResult);

    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=] {closeWindow();});
    rtcthread->start();
}

void RTC::handleResult(int result, QString str)
{
    if(result){
        tips->setText(QString("RTC检测到设备：").append(str));
        //0: not set
        //1: set
        QString setFlag = common.getConfig("rtc", "set_rtc_time");
        if (setFlag == "0"){
            this->setDate();
            QString str = QString("\n").append(defTime).append("\nRTC时间已设置,请断电2分钟重启设备再次点开RTC查看时间");
            tips->setText(tips->text().append(str));
            closeBtn->setVisible(false);
            common.setConfig("rtc", "set_rtc_time", "1");
        }else{
            this->getDate();
            setDateBtn->setVisible(true);
            closeBtn->setVisible(true);
        }
    }else{
        tips->setText(QString("RTC测试失败!"));
        testResult = result;
    }
}

void RTC::setDate()
{
    common.execLinuxCmd("date --set=\"2022-12-14 12:00:00\"");
    common.execLinuxCmd("hwclock -w");
}

void RTC::getDate(){
    QString hwclock = common.execLinuxCmd("hwclock");
    QString hwclockFormat = common.execLinuxCmd(QString("date -d \"%1\" +\"%Y-%m-%d %H:%M:%S\"").arg(hwclock));
    QString sysTime = common.execLinuxCmd("date +\"%Y-%m-%d %H:%M:%S\"");
    QString str1 = QString("\n").append("系统时间:").append(sysTime);
    QString str2 = QString("\n").append("RTC时间:").append(hwclockFormat);
    tips->setText(tips->text().append(defTime).append(str1).append(str2));

    if(sysTime.startsWith("2022-12-14")){

    }
    QDateTime dateTime;
    qint64 defaultTime = dateTime.fromString("2022-12-14 12:00:00", "yyyy-MM-dd hh:mm:ss").toTime_t();
    qint64 sysCurrentTime = dateTime.currentDateTime().toTime_t();

    qDebug()<<defaultTime;
    qDebug()<<sysCurrentTime;

    if(sysCurrentTime >= defaultTime){
        testResult = true;
    }else{
        testResult = false;
    }

    if(autoTest){
        timer->start(waitCloseSecond);
    }

}

void RTC::closeWindow()
{
    timer->stop();
    //update config file state
    common.setConfig("rtc", "state", QString::number(testResult));
    rtcthread->quit();
    emit testFinish();
    this->close();
}

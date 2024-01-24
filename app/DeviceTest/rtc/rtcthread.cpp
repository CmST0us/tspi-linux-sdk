#include "rtcthread.h"

#define RTC_NAME_PATH "/sys/class/rtc/rtc0/name"

RTCThread::RTCThread(QObject *parent): QThread(parent)
{

}

void RTCThread::run()
{
    int result;
    QString rtcName;
    QFile file(RTC_NAME_PATH);
    if(file.exists()){
        file.open(QIODevice::ReadOnly);
        rtcName = file.readAll();
        result = 1;
//        qDebug()<<rtcName;
        file.close();
    }else{
        result = 0;
    }
    emit getResult(result, rtcName);
}

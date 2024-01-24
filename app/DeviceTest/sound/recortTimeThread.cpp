#include "recortTimeThread.h"

RecordTimeThread::RecordTimeThread()
{

}

void RecordTimeThread::run()
{
    setRecordTime();
}

void RecordTimeThread::setRecordTime()
{
    int recordTime = 0;
    int minute = 0;
    QString timeStr;
    while (recordStart){

        if(recordTime < 10)
            timeStr = "0"+QString::number(minute)+":0" + QString::number(recordTime);
        else if(10 <= recordTime && recordTime <= 59)
            timeStr = "0"+QString::number(minute) +":"+ QString::number(recordTime);
        else{
            recordTime = 0;
            minute++;
            timeStr = "0"+QString::number(minute)+":0" + QString::number(recordTime);
        }

        recordTime++;
        emit updateRecordTime(timeStr);
        msleep(999);
    }
}

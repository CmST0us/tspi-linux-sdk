#include "playtimethread.h"

PlayTimeThread::PlayTimeThread()
{

}

void PlayTimeThread::run()
{
    setPlayTime();
}

void PlayTimeThread::setPlayTime()
{
    QString timeStr;
    int minute = 0;
    int second = 0;
    minute = playTime < 60 ? 0 : playTime / 60;
    second = playTime < 60 ? playTime : playTime % 60;

    while (playStart){
        if( 10 <= second && second <= 59 )
            timeStr = "0"+QString::number(minute) +":"+ QString::number(second);
        else if(0 <= second && second < 10)
            timeStr = "0"+QString::number(minute)+":0" + QString::number(second);
        else{
            if(minute <=0 )
                break;
            second = 59;
            minute --;
            timeStr = "0"+QString::number(minute)+":" + QString::number(second);
        }
        second --;
        emit updatePlayTime(timeStr);
        msleep(999);
    }
}

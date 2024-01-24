#include "pwmthread.h"

PwmThread::PwmThread()
{

}

void PwmThread::run()
{
    while (autoChangeValue == true) {
        setPwm();
        setBackLight();
        msleep(100);
    }
}

void PwmThread::setPwm(){
    QDir dir(PWM_DEV_PATH);
    for(QFileInfo info : dir.entryInfoList()){
        if(info.fileName() == "." || info.fileName() == "..")
            continue;
        if(sign == 0){
            pwmValue -= 10;
            if (pwmValue <=10)
                sign = 1;
        }else if(sign == 1){
            pwmValue += 10;
            if (pwmValue > period -10)
                sign = 0;
        }
        QFile file(QString(info.filePath() + "/pwm0/duty_cycle"));
        if(file.exists()){
            ::system(QString(" echo %1 > "+info.filePath() + "/pwm0/duty_cycle")
                 .arg(QString::number(pwmValue)).toLocal8Bit());
            emit setPwmValue(info.fileName(), pwmValue);
        }
    }
}

void PwmThread::setBackLight()
{
    if(backLightSign == 0){
        backLightLevel -= 10;
        if (backLightLevel <=10)
            backLightSign = 1;
    }else if(backLightSign == 1){
        backLightLevel += 10;
        if (backLightLevel > 255-10)
            backLightSign = 0;
    }
    ::system(QString(" echo %1 > /sys/class/backlight/backlight/brightness")
              .arg(QString::number(backLightLevel)).toLocal8Bit());
    emit setBackLightValue(backLightLevel);
}

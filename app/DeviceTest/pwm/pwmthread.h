#ifndef PWMTHREAD_H
#define PWMTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

#define PWM_DEV_PATH "/sys/class/pwm"

class PwmThread : public QThread
{
    Q_OBJECT
public:
    PwmThread();
    void run () override;

    void setPwm();
    void setBackLight();
    int sign = 0;
    int pwmValue = 700;
    int period = 1000;
    int duty_cycle = 700;
    int backLightLevel = 255;
    int backLightSign = 0;
    QString polarity = "normal";
    bool autoChangeValue = true;

signals:
    void setPwmValue(QString pwmChip, int value);
    void setBackLightValue(int value);
};

#endif // PWMTHREAD_H

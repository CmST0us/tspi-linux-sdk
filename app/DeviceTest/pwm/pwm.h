#ifndef PWM_H
#define PWM_H

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include "common/common.h"
#include "pwmthread.h"

class Pwm : public QWidget
{
    Q_OBJECT
public:
    explicit Pwm(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    int initPwm(QString pwmName);

    void setPolar(QString pwm, QString polar);

    Common common;
    PwmThread *pwmThread;
    bool testResult;

public slots:
    void setSliderValue(QString pwmChip, int value);
    void setBackSliderValue(int value);
    void manualSetBackLight(int value);
    void manualSetPwm(QString pwmChip, int value);
    void handleClick(bool result);

signals:
    void testFinish();

};

#endif // PWM_H

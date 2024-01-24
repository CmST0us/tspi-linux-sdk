#ifndef LED_H
#define LED_H

#include <QObject>
#include <QWidget>
#include "common/common.h"

class Led : public QWidget
{
    Q_OBJECT
public:
    explicit Led(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult=false;

protected:
    Common common;
    void setValue(QString ledName, int value);
    void getTestResult(bool testResult);
    void handleTimeout();
    void handleClick(bool result);
    QString bl_led_ctl;
    QString ir_led_ctl;
    int ledValue = 1;
    QTimer *timer;

signals:
    void testFinish();
};

#endif // LED_H

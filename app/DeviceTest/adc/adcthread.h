#ifndef ADCTHREAD_H
#define ADCTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "common/common.h"

#define ADC_PATH "/sys/bus/iio/devices/"
#define GPADC_PATH "/sys/class/gpadc/"  //for allwiner

class AdcThread : public QThread
{
    Q_OBJECT
public:
    AdcThread();

    void run() override;

    void readeAllAdcValue();

    bool readFlag = true;

signals:
    void getAdcValue(QString value, QString chnnel);
};

#endif // ADCTHREAD_H

#ifndef ADCKEY_H
#define ADCKEY_H

#include <QWidget>
#include <QFormLayout>
#include "common/common.h"
#include "adcthread.h"

class AdcKey : public QWidget
{
    Q_OBJECT
public:
    explicit AdcKey(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult;

protected:
    QLabel *keyValue;
    Common common;
    AdcThread *thread;

protected slots:
    void handleClick(bool restul);
    void setAdcValue(QString value, QString chnnel);

signals:
    void testFinish();

};

#endif // ADCKEY_H

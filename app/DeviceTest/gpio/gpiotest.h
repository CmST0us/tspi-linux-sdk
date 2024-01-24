#ifndef GPIOTEST_H
#define GPIOTEST_H

#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include "common/common.h"


class GPIOTest : public QWidget
{
    Q_OBJECT
public:
    explicit GPIOTest(QWidget *parent = nullptr);
    void initWindow(int w, int h);
    bool testResult = false;

protected:
    Common common;
    QGridLayout *layout;
    QLabel *gpioLabel;
    QPushButton *pullUp;
    QPushButton *pullDown;
    QPushButton *pullUpAll;
    QPushButton *pullDownAll;
    QPushButton *readBtn;
    QLabel *valueLabel;
    QStringList getAllGpios();

protected slots:
    void setValue(int value);

    void readValue();

    void setAllValue(int value);

    void handleClick(bool result);

signals:
    void testFinish();
};

#endif // GPIOTEST_H

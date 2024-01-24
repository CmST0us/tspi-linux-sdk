#ifndef RTC_H
#define RTC_H

#include <QObject>
#include <QMetaType>
#include <QTimer>
#include <QDate>
#include "common/common.h"
#include "rtcthread.h"

class RTC : public QWidget
{
    Q_OBJECT
public:
    explicit RTC(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult;

private:
    QTimer *timer;
    QLabel *tips;
    Common common;
    RTCThread *rtcthread;
    QPushButton *closeBtn;
    QPushButton *setDateBtn;
    QString defTime = "\n初始时间:2022-12-14 12:00:00\n";

private:
    void closeWindow();
    void setDate();
    void getDate();

protected slots:
    void handleResult(int resutl, QString str);

signals:
    void testFinish();

};

#endif // RTC_H

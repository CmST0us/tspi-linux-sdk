#ifndef RTCTHREAD_H
#define RTCTHREAD_H

#include <QThread>
#include <QDebug>
#include <QFile>

#include "common/common.h"

class RTCThread : public QThread
{
    Q_OBJECT
public:
    explicit RTCThread(QObject *parent = nullptr);

    void run();


signals:
    void getResult(int, QString);
};

#endif // RTCTHREAD_H

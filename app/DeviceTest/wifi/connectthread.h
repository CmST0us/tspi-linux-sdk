#ifndef CONNECTTHREAD_H
#define CONNECTTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QNetworkInterface>
#include "common/common.h"

class ConnectThread : public QThread
{
    Q_OBJECT
public:
    ConnectThread();

    void run() override;

    bool checkConnect = true;

    int checkTimes = 0;

protected:
    void connectState();
    Common common;

signals:
    void getWlanIp(QString ip);
};

#endif // CONNECTTHREAD_H

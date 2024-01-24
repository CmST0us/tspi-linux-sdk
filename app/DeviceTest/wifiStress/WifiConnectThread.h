#ifndef WIFICONNECTTHREAD_H
#define WIFICONNECTTHREAD_H

#include <QObject>
#include <QThread>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QFile>
#include <QApplication>
#include "common/common.h"

class WifiConnectThread : public QObject
{
    Q_OBJECT
public:
    explicit WifiConnectThread(QObject *parent = nullptr);
    ~WifiConnectThread();

    void setValidAplist(QList<QString> list);

private:
    void connectAp();
    bool checkConnectState();
    void configApInfo(QString apName);
    void startWpaServer();

private:
    Common common;
    bool checkFlag=true;
    bool threadExitFlag = false;
    int checkTimes=0;
    bool conState=false;
    QList<QString> validAplist;

signals:
    void connectState(QString apName, bool state);
    void allConnectFinished();
    void connectSuccess();

public slots:
    void startConnectAp();
    void connectContinue();
};

#endif // WIFICONNECTTHREAD_H

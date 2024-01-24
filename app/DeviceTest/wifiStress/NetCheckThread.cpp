#include "NetCheckThread.h"

#define SERVER_IP "192.168.12.1"

NetWorkCheckThread::NetWorkCheckThread(QObject *parent) : QObject(parent)
{
    checkNetWork = true;
}

void NetWorkCheckThread::startCheckNet()
{
    checkNetWork = true;
    qDebug()<<"start check network state";
    while (checkNetWork) {
        testNet();
        this->thread()->msleep(2000);
    }
}

void NetWorkCheckThread::stopCheckNet()
{
    qDebug()<<"stop check network!";
    this->checkNetWork = false;
}

void NetWorkCheckThread::testNet()
{
    QProcess *p = new QProcess(this);
    QString testCmd = QString("ping -I wlan0 %1 -c 1").arg(SERVER_IP);
    p->start("bash", QStringList() <<"-c" << testCmd);
    p->waitForFinished();
    QString result = p->readAll();
//    qDebug()<<result;
    qDebug()<<"check net state!";
    if(result.indexOf("ttl") < 0){
        times ++;
        if(times >= 2){
            emit netDisconneted();
        }
    }
}

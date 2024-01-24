#include "ScanThread.h".h"

ScanThread::ScanThread()
{
    scanThreadOut = false;
}

void ScanThread::run()
{
    qDebug()<<"run";
    while (!scanThreadOut) {
        if(scanFlag){
            scanAp();
        }
        sleep(5);
    }
}

void ScanThread::scanAp()
{
    QString apListstr = common.execLinuxCmd("iwlist wlan0 scan");
    QList<QString> resultList = apListstr.replace(" ","").split("Cell");
    QList<QString> apList;
    qDebug()<<"scanAp!!!!!!!!!!!!!!!";

    foreach (QString str, resultList) {
        //qDebug()<<str;
        QStringList wifiInfoList = str.split("\n");
        foreach (QString wifiInfo, wifiInfoList) {
            if(wifiInfo.startsWith("ESSID")){
                QString ssid = wifiInfo.replace("ESSID:", "").replace("\"","");
                if(!ssid.startsWith("rk3399-"))
                    continue;
                if(ssid == "")
                    break;
                apList.append(ssid);
            }
        }
    }
    emit scanApList(apList);
    apList.clear();
}

void ScanThread::startScan()
{
    qDebug()<<"start scan";
    this->scanFlag = true;
}

void ScanThread::stopScan()
{
    qDebug()<<"stop scan";
    this->scanFlag = false;
}

void ScanThread::exitThread()
{
    qDebug()<<"scan thread exit";
    this->scanThreadOut = true;
}

void ScanThread::startThread()
{
    qDebug()<<"scan thread start";
    this->scanThreadOut = false;
}



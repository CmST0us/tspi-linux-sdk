#include "wifithread.h"

WifiThread::WifiThread()
{

}

void WifiThread::run()
{
    qDebug()<<"run";
    openWifi();
    while(scanFlg){
        scanAp();
        msleep(5000);
    }
}

void WifiThread::openWifi()
{
    ::system("echo 1 > /sys/class/rfkill/rfkill1/state");
    ::system("ifconfig wlan0 up");
}

void WifiThread::scanAp()
{
    QMap<QString,QString> infoMap;
    QList<QMap<QString, QString>> infoList;
    QString apList = common.execLinuxCmd("iwlist wlan0 scan");
    QList<QString> resultList = apList.replace(" ","").split("Cell");

    infoList.clear();
    foreach (QString str, resultList) {
        //qDebug()<<str;
        infoMap.clear();
        QStringList wifiInfoList = str.split("\n");
        foreach (QString wifiInfo, wifiInfoList) {
            if(wifiInfo.startsWith("ESSID")){
                QString ssid = wifiInfo.replace("ESSID:", "").replace("\"","");
                if(ssid == "")
                    break;

                infoMap.insert("SSID",ssid);
                    //qDebug()<<ssid;
            }

            if(wifiInfo.startsWith("Frequency")){
                QString freq = wifiInfo.replace("Frequency:", "").replace("\"","");
                infoMap.insert("Frequency",freq);
            }
        }
        if(!infoMap.isEmpty())
            infoList.append(infoMap);
    }
    emit getApMap(infoList);
}



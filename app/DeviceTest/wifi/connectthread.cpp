#include "connectthread.h"

ConnectThread::ConnectThread()
{

}

void ConnectThread::run()
{
    ::system("killall wpa_supplicant");
    msleep(1000);
    ::system("wpa_supplicant -i wlan0 -Dnl80211 -c /tmp/wpa_supplicant.conf -B");
    qDebug()<<"run";
    while (checkConnect) {
        connectState();
        msleep(500);
    }
}

void ConnectThread::connectState()
{
    checkTimes ++ ;
    QString wlanIp = "";
    QString state = common.execLinuxCmd("wpa_cli -i wlan0 status");
    qDebug()<<state;
    if(state.indexOf("ssid=") >= 0){//connect success, and then get ipaddress
        common.execLinuxCmd("udhcpc -i wlan0");

        QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface interface, ifaceList) {

            QString interfaceName = interface.humanReadableName();
            if(!interfaceName.startsWith("wlan0"))
                continue;
            QList<QNetworkAddressEntry> addresses = interface.addressEntries();
            foreach (QNetworkAddressEntry address, addresses) {
                qDebug()<<address.ip();
                wlanIp = address.ip().toString();
                break;
            }
        }
    }

    if(wlanIp != "" || checkTimes >= 20){
        checkConnect = false;
        emit getWlanIp(wlanIp);
    }
}

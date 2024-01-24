#include "WifiConnectThread.h"

WifiConnectThread::WifiConnectThread(QObject *parent):QObject(parent)
{
}

WifiConnectThread::~WifiConnectThread()
{
}

void WifiConnectThread::startConnectAp()
{
    qDebug()<<"startConnectAp";
    connectAp();
}

void WifiConnectThread::connectAp()
{
    if(validAplist.count() == 0){
        emit allConnectFinished();
        return;
    }
    QString apName = validAplist.at(0);
    checkTimes = 0;
    checkFlag = true;
    this->configApInfo(apName);
    this->startWpaServer();
    while (checkFlag) {
        if(threadExitFlag)
            break;
        checkConnectState();
        this->thread()->msleep(500);
    }
    validAplist.removeAt(0);
    emit connectState(apName, conState);
    if(conState)
        emit connectSuccess();
    else
        connectAp();
}

bool WifiConnectThread::checkConnectState()
{
    checkTimes ++ ;
    QString wlanIp = "";
    QString state = common.execLinuxCmd("wpa_cli -i wlan0 status");
    qDebug()<<state;
    if(state.indexOf("ssid=") >= 0){//connect success, and then get ipaddress
        common.execLinuxCmd("killall udhcpc");
        common.execLinuxCmd("timeout 2 udhcpc -i wlan0");

        QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface interface, ifaceList) {

            QString interfaceName = interface.humanReadableName();
            if(!interfaceName.startsWith("wlan0"))
                continue;
            QList<QNetworkAddressEntry> addresses = interface.addressEntries();
            foreach (QNetworkAddressEntry address, addresses) {
                qDebug()<<address.ip();
                wlanIp = address.ip().toString();
                qDebug()<<"wlanIp"<<wlanIp;
                if(wlanIp != ""){
                    checkFlag = false;
                    conState = true;
                    return conState;
                }
            }
        }
    }else{
        if(checkTimes >= 20){
            checkFlag = false;
            conState = false;
            return conState;
        }
    }
}

void WifiConnectThread::configApInfo(QString apName)
{
    QFile file("/tmp/wpa_supplicant.conf");
    file.remove();
    if(file.open(QFile::ReadWrite)){
        QString str = "ctrl_interface=/var/run/wpa_supplicant\n"
                      "network={\n"
                            "ssid=\""+apName+"\"\n"
                            "psk=\"12345678\"\n"
                            //"key_mgmt=WPA2-PSK\n"
                      "}\n";
        file.write(str.toLocal8Bit());
    }
    file.close();
}

void WifiConnectThread::startWpaServer()
{
    ::system("killall wpa_supplicant");
    this->thread()->msleep(1000);
    ::system("wpa_supplicant -i wlan0 -Dnl80211 -c /tmp/wpa_supplicant.conf -B");
    this->thread()->msleep(1000);
}

void WifiConnectThread::setValidAplist(QList<QString> list)
{
    this->validAplist = list;
}

void WifiConnectThread::connectContinue()
{
    this->thread()->sleep(2);
    this->connectAp();
}

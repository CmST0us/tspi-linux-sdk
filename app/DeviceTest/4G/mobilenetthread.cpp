#include "mobilenetthread.h"

MobileNetThread::MobileNetThread()
{
}

void MobileNetThread::run()
{
    msleep(200);
    if(!checkMobileNet())
        pppDial();
}

bool MobileNetThread::checkMobileNet()
{
    QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, ifaceList) {

        QString interfaceName = interface.humanReadableName();
#ifndef USE_GOBINET
        if(!interfaceName.startsWith("ppp"))
#else
        if(!interfaceName.startsWith("usb"))
#endif
            continue;

        QList<QNetworkAddressEntry> addresses = interface.addressEntries();
        foreach (QNetworkAddressEntry address, addresses) {
            qDebug()<<address.ip();
            pppIp = address.ip().toString();
            qDebug()<<"usb0 IP:"<<pppIp;
            if(pppIp.startsWith("1")){
                // qDebug()<<pppIp;
                break;
            }else{
                pppIp = "";
            }
        }
    }

    emit getPppIP(pppIp);
    return pppIp != "";
}

void MobileNetThread::pppDial()
{
    if(pppIp != ""){
        qDebug()<<"dial already!";
//        this->dealReturn();
    }else{
#ifndef USE_GOBINET
        //killall process first
        ::system("echo "" > /tmp/4G.log");
        emit getReturnStr("");

        ::system("/etc/ppp/peers/quectel-ppp-kill");
        ::system("/etc/ppp/peers/quectel-pppd.sh > /tmp/4G.log");
#else
        emit getReturnStr("");
#endif
    }
}

void MobileNetThread::dealReturn(QString str)
{
    qDebug()<<"deal Return";
//    QFile file("/tmp/4G.log");
//    file.open(QIODevice::ReadOnly);
//    QString log = file.readAll();
      emit getReturnStr(str);
}

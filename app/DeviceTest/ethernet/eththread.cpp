#include "eththread.h"

EthThread::EthThread()
{

}

void EthThread::run()
{
    while(readFlag){
        //检测网线插拔
        this->getReticleStat();

        if(pingTestFlag){
            pingTest();
            pingTestFlag = false;
        }

        if(iperfTestFlag){
            iperfTestFlag = false;
        }

        msleep(200);
    }
}

void EthThread::pingTest()
{
    result = false;
    QString str;
    if(targetIp.startsWith("www")){
        str = eth +"测试开始\n开始测试域名解析，目的地址:" +targetIp;
    }else{
        str = eth +"测试开始\n开始测试IP通信，目的地址:" +targetIp;
    }

    emit pingTestFinish(str);

    QProcess *p = new QProcess;
    connect(p, &QProcess::readyRead, this, [=] {this->dealReturn(p->readAll());});

    //ping -I eth0 192.168.1.1 -c 4 -i 0.4
    QString testCmd = "ping -I " + eth + " " + targetIp + " -c 4" + " -i 0.4";
    p->start("bash", QStringList() <<"-c" << testCmd);
    p->waitForFinished();
}

void EthThread::dealReturn(QString returnTxt){
    emit pingTestFinish(returnTxt);

    //test finish
    if(targetIp.startsWith("www")){
        if(returnTxt.indexOf("transmitted") >= 0){
            emit setResult(result, eth, targetIp);
            return;
        }
    }

    //test finish
    if(returnTxt.indexOf("transmitted") >= 0){
        emit setResult(result, eth, targetIp);
        // targetIp = targetAddr;
        // pingTestFlag = true;
    }

    if(returnTxt.indexOf("ttl") >= 0) {
        //qDebug("test success");
        result = true;
    }
}

void EthThread::getReticleStat()
{
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();//获取所有网卡接口信息到list
    //遍历接口信息
    foreach(QNetworkInterface interface,list){
        QNetworkInterface::InterfaceFlags flags = interface.flags();//获取flag
        if(QNetworkInterface::Ethernet == interface.type()){
            if(flags.testFlag(QNetworkInterface::IsUp)){ //判断活动状态，可以此检测网线插拔
                // qDebug()<<interface.name()<<"is up";
                QList<QNetworkAddressEntry> iplist = interface.addressEntries();//获取当前ip
                ip = getIp(iplist);

                if(ip == ""){
                    QString cmd = QString("timeout 2 udhcpc -i %1").arg(interface.name());
                    system(cmd.toLocal8Bit());
                }
                ip = getIp(iplist);
                if(ip == "")
                    emit setIp(interface.name(), "");
                else
                    emit setIp(interface.name(), ip);

            }else{
                //qDebug()<<interface.name()<<"is down";
            }
        }
    }
}

QString EthThread::getIp(QList<QNetworkAddressEntry> ettry)
{
    QString ip;
    foreach (QNetworkAddressEntry address, ettry) {
        //qDebug()<<address.ip().toString();
        ip = address.ip().toString();
        break;
    }
    return ip;
}


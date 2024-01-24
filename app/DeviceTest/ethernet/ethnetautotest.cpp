#include "ethnetautotest.h"

EthnetAutoTest::EthnetAutoTest()
{

}

void EthnetAutoTest::run()
{
    msleep(500);
    getEth();
}

void EthnetAutoTest::getEth()
{
    QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
    int ethCount = 0;
    foreach (QNetworkInterface interface, ifaceList) {
        QString interfaceName = interface.humanReadableName();
        if(!interfaceName.startsWith("eth"))
            continue;
        ethCount ++;
        emit testEth(interfaceName);
        //wait for finish
        waitTestFinish();
        qDebug()<<"test nexit!";
    }
    msleep(waitCloseSecond);
    emit allEthTestFinish();
}

void EthnetAutoTest::waitTestFinish()
{
    while(!testFinish){
        // qDebug()<<"wait for test finish";
        msleep(1000);
    }
}

#include "ethernet.h"

EtherNet::EtherNet(QWidget *parent) : QWidget(parent)
{

}

void EtherNet::initWindow(int w, int h)
{
    ethCount = 0;
    successCount = 0;
    QPushButton *title = common.getButton("以太网连通性测试!");
    title->setStyleSheet(title->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QGridLayout *layout = new QGridLayout;
    thread = new EthThread;
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");

    layout->addWidget(title, 1, 1, 1, 9);
    QLabel *testLabel = common.getLabel("测试IP:");
    QLabel *testAddrLabel = common.getLabel("测试网址:");
    QLineEdit *testIP = common.getLineEdit();
    testIP->setText(this->getGateway());

    QLineEdit *testAddr = common.getLineEdit();
    testAddr->setText("www.baidu.com");
    layout->addWidget(testLabel,2 ,1 ,1 ,1);
    layout->addWidget(testIP, 2, 2, 1, 4);
    layout->addWidget(testAddrLabel,2 ,6 ,1,1);
    layout->addWidget(testAddr,2, 7, 1, 3);
    int row = 3;
    QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, ifaceList) {

        QString interfaceName = interface.humanReadableName();
        if(!interfaceName.startsWith("eth"))
            continue;
        ethCount ++;
        QString hardWareAddr = interface.hardwareAddress();

        QLineEdit *hardWareAddrEdit = common.getLineEdit();
        QLineEdit *ipaddr = common.getLineEdit();
        ipaddr->setObjectName(interfaceName);
        QLabel *ethLabel = common.getLabel(interfaceName);
        hardWareAddrEdit->setText(hardWareAddr);
        QList<QNetworkAddressEntry> addresses = interface.addressEntries();
        foreach (QNetworkAddressEntry address, addresses) {
            qDebug()<<address.ip();
            ipaddr->setText(address.ip().toString());
            break;
        }

        QPushButton *pingBtn = common.getButton("ping");
        pingBtn->setObjectName(interfaceName);
        connect(pingBtn, &QPushButton::clicked, this, [=] {
            this->handleTestBtnClick(true, testIP->text(), interfaceName, testAddr->text());
        });
        QPushButton *iperfBtn = common.getButton("iperf");

        layout->addWidget(ethLabel, row, 1, 1, 1);
        layout->addWidget(ipaddr, row, 2, 1, 3);
        layout->addWidget(hardWareAddrEdit, row, 5, 1, 3);
        layout->addWidget(pingBtn, row, 8, 1, 1);
        layout->addWidget(iperfBtn, row, 9, 1, 1);
    }

    QPushButton *failBtn = common.getButton("关闭窗口");
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(failBtn, &QPushButton::clicked, [=] {this->closeWindow();});

    detial = common.getTextEdit();
    layout->addWidget(detial, ++row, 1, 1, 9);
    layout->addWidget(failBtn, ++row, 5, 1, 3, Qt::AlignBottom);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(thread, &EthThread::pingTestFinish, this, &EtherNet::displayInfo);
    connect(thread, &EthThread::setResult, this, &EtherNet::getResult);
    connect(thread, &EthThread::setIp, this, &EtherNet::getIp);
    thread->start();

    //auto test
    autoTestThread = new EthnetAutoTest;
    connect(autoTestThread, &EthnetAutoTest::testEth, this, &EtherNet::autoPing);
    connect(autoTestThread, &EthnetAutoTest::allEthTestFinish, this, &EtherNet::handleAllTestFinish);
    autoTestThread->start();
}

QString EtherNet::getGateway(){
    QString routeRes = common.execLinuxCmd("route -n");
    qDebug()<<routeRes;
    foreach (QString str, routeRes.split("\n")) {
        str = str.replace(QRegExp("\\s{1,}"), " ");
        if(str.endsWith("eth0")){
            QString gateway = str.split(" ").at(1);
            if(gateway.length() > 7)// ! 0.0.0.0
                return gateway;
        }
    }
    return "0.0.0.0";
}

void EtherNet::displayInfo(QString txt)
{
    detial->append(txt);
}

void EtherNet::getResult(bool result, QString eth, QString ipaddr){
    QPushButton* btn = this->findChild<QPushButton *>(eth);
    if(ipaddr.startsWith("www")){
        if(result){
            detial->append(eth+" 域名解析测试成功!!\n");
        }else{
            detial->append(eth+" 域名解析测试失败!!\n");
        }
    }else{
        if(result){
            detial->append(eth+" IP地址通信测试成功!!\n");
            btn->setStyleSheet(btn->styleSheet()+"QPushButton{border-image:url(:/icon/pass.png)}");
            successCount++;
        }else{
            btn->setStyleSheet(btn->styleSheet()+"QPushButton{border-image:url(:/icon/faild.png)}");
            detial->append(eth+" IP地址通信测试失败!!\n");
        }
        autoTestThread->testFinish = true;
    }
}

void EtherNet::handleTestBtnClick(bool pingTestFlag, QString targetIp, QString eth, QString targetAddr)
{
    autoTestThread->testFinish = false;
    detial->clear();
    thread->pingTestFlag = pingTestFlag;
    thread->targetIp = targetIp;
    thread->eth = eth;
    thread->targetAddr = targetAddr;
}

void EtherNet::getIp(QString eth, QString ip)
{
    QLineEdit *lineEdit = this->findChild<QLineEdit *>(eth);
    lineEdit->setText(ip);
}

void EtherNet::autoPing(QString eth)
{
    QPushButton* btn = this->findChild<QPushButton *>(eth);
    emit btn->clicked();
}

void EtherNet::handleAllTestFinish()
{
    if(autoTest){
        closeWindow();
    }
}

void EtherNet::closeWindow()
{
    thread->readFlag = false;
    thread->quit();
    autoTestThread->quit();

    testResult = (ethCount == successCount);

    //update config file state
    common.setConfig("ethernet", "state", QString::number(testResult));

    emit testFinish();
    this->close();

}



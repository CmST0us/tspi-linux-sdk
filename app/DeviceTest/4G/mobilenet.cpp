#include "mobilenet.h"

MobileNet::MobileNet(QWidget *parent) : QWidget(parent)
{

}

void MobileNet::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout;
    title = common.getButton("4G PPP拨号测试");
    title->setStyleSheet(title->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QLabel *wlanLabel = common.getLabel("ppp0");
    pppIp = common.getLineEdit();
    QPushButton *pingBtn = common.getButton("ping");
    QPushButton *closeBtn = common.getButton("关闭窗口");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {this->closeWindow();});
    connect(pingBtn, &QPushButton::clicked, [=] {this->pingTest(pppIp->text());});

    detial = common.getTextEdit();

    layout->addWidget(title, 1, 1, 1, 5);
    layout->addWidget(wlanLabel, 2, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(pppIp, 2, 2, 1, 3);
    layout->addWidget(pingBtn,2, 5, 1, 1);
    layout->addWidget(detial, 3, 1, 1, 5);
    layout->addWidget(closeBtn, 4, 3, 1, 1, Qt::AlignBottom);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);

    mobleNetThread = new MobileNetThread();
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(mobleNetThread, &MobileNetThread::getPppIP, this, &MobileNet::setPppIp);
    connect(mobleNetThread, &MobileNetThread::getReturnStr, this, &MobileNet::dealReturnStr);
    mobleNetThread->start();

    closeTimer = new QTimer(this);
    connect(closeTimer, &QTimer::timeout, this, [=] {this->closeWindow();});
}

void MobileNet::setPppIp(QString ip)
{
    pppIp->setText(ip);

    //alredy dial
    if(ip != ""){
        detial->setText("4G已拨号!测试成功！");
        testResult = true;

        if(autoTest)
            closeTimer->start(waitCloseSecond);
    }
}

void MobileNet::dealReturnStr(QString str)
{
    //4g module not recognized
    QFile ttyFile("/dev/ttyUSB2");
    if(!ttyFile.exists()){
        detial->setText("/dev/ttyUSBx 节点不存在，4G模块识别失败!");
        testResult = false;
        if(autoTest)
            closeTimer->start(waitCloseSecond);
        return;
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=] {this->readLog();});
    timer->start(100);
#ifdef USE_GOBINET
    ::system("/usr/sbin/quectel-CM &");
#endif
}

void MobileNet::readLog()
{
    timeoutTimes ++;
#ifndef USE_GOBINET
    QFile file("/tmp/4G.log");
    if(file.exists()){
        file.open(QIODevice::ReadOnly);
        QString str = file.readAll();
        detial->setText(str);
        file.close();

        if(str.indexOf("local  IP address") >= 0){
            timer->stop();
            QList<QString> resultList = str.split("\n");
            for(QString strTxt : resultList){
                if(strTxt.startsWith("local  IP address")){
                    QString ipAddress = strTxt.replace("local  IP address", "").replace(" ","");
                    pppIp->setText(ipAddress);
                    detial->append("拨号成功!!!!");
                    break;
                }
            }
            //dial success
            testResult = true;
        }

        //maybe sim card not found!
        if(str.indexOf("Connect script failed") >= 0){
            timer->stop();
            detial->append("拨号失败!!!!");
            //dial fail
            testResult = false;
        }

        //time out
        if(timeoutTimes > 300){
            timer->stop();
            detial->append("拨号失败,连接超时!!!!");
            //dial fail
            testResult=false;
        }
        detial->moveCursor(QTextCursor::End);
    }
#else
    mobleNetThread->checkMobileNet();
    if(pppIp->text() != ""){
        detial->append("拨号成功!!!!");
        timer->stop();
    }
    if(timeoutTimes > 300){
        timer->stop();
        detial->append("拨号失败,连接超时!!!!");
        //dial fail
        testResult=false;
    }
#endif
    //for autotest
    if (autoTest){
        closeTimer->start(waitCloseSecond);
    }
}

void MobileNet::pingTest(QString ip)
{
    if(ip != ""){
        QProcess *p = new QProcess;
        connect(p, &QProcess::readyRead, this, [=] {detial->append(p->readAll());});
#ifndef USE_GOBINET
        QString testCmd = "ping -I ppp0 8.8.8.8 -i 1";
#else
        QString testCmd = "ping -I usb0 8.8.8.8 -i 1";
#endif
        p->start("bash", QStringList() <<"-c" << testCmd);
    }else{
        detial->append("4G未成功拨号！！！");
    }
}

void MobileNet::closeWindow()
{
    closeTimer->stop();
    mobleNetThread->quit();

    //update config file state
    common.setConfig("4G", "state", QString::number(testResult));
    emit testFinish();
    this->close();
}


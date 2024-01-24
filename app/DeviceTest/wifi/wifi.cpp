#include "wifi.h"

static QTableWidget *apList;
Wifi::Wifi(QWidget *parent) : QWidget(parent)
{

}

void Wifi::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout;
    QString wlanHardWareAddr = "No mac Address!!!";
    title = common.getButton("WIFI扫描测试");
    title->setStyleSheet(title->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QLabel *wlanLabel = common.getLabel("wlan0");
    wlanIp = common.getLineEdit();
    wlanMac = common.getLineEdit();

    QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, ifaceList) {
        QString interfaceName = interface.humanReadableName();
        if(interfaceName == "wlan0"){
            wlanHardWareAddr = interface.hardwareAddress();
            break;
        }
    }
    wlanMac->setText(wlanHardWareAddr);
    QPushButton *pingBtn = common.getButton("ping");
    QPushButton *closeBtn = common.getButton("关闭窗口");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {this->closeWindow();});
    connect(pingBtn, &QPushButton::clicked, [=] {this->pingTest(wlanIp->text());});

    apList = common.getTableWidget(2);
    QStringList strHeader;
    strHeader << "AP" << "频率";
    apList->setHorizontalHeaderLabels(strHeader);  //添加表头
    apList->horizontalHeader()->setHidden(true);
    apList->verticalHeader()->setDefaultSectionSize(50);
    apList->setSelectionBehavior(QTableWidget::SelectRows);
    connect(apList, &QTableWidget::cellClicked, this, [=] {this->showDialog(apList->currentRow());});

    thread = new WifiThread;

    layout->addWidget(title, 1, 1, 1, 6);
    layout->addWidget(wlanLabel, 2, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(wlanIp, 2, 2, 1, 2);
    layout->addWidget(wlanMac, 2, 4, 1, 2);
    layout->addWidget(pingBtn,2, 6, 1, 1);
    layout->addWidget(apList, 3, 1, 1, 6);
    layout->addWidget(closeBtn, 4, 3, 1, 1, Qt::AlignBottom | Qt::AlignCenter);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);

    qRegisterMetaType<QList<QMap<QString, QString>>>("QList");
    connect(thread, &WifiThread::getApMap, this, &Wifi::setApMap);
    thread->start();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=] {this->closeWindow();});
}

void Wifi::setApMap(QList<QMap<QString, QString>> infoList)
{
    apList->clear();
    apList->setRowCount(infoList.size());
    for (int i = 0; i < infoList.size(); i++) {

        QMapIterator<QString, QString> iterator(infoList.at(i));
        while (iterator.hasNext()) {
            iterator.next();
//            qDebug() << iterator.key() << ":" << iterator.value();
            QTableWidgetItem *tableItem = new QTableWidgetItem(iterator.value());
            if(iterator.key() == "SSID"){
                apList->setItem(i,0,tableItem);
            }else{
                tableItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                apList->setItem(i,1,tableItem);
            }
        }
    }

    if(autoTest){
        timer->start(waitCloseSecond);
    }
}

void Wifi::closeWindow()
{
    thread->scanFlg = false;
    thread->quit();

    if(apList->rowCount() < 1){
        testResult = false;
    }else{
        testResult = true;
    }

    timer->stop();

    //update config file state
    common.setConfig("WIFI", "state", QString::number(testResult));
    emit testFinish();
    this->close();
}

void Wifi::showDialog(int row)
{
    QString apName = apList->model()->index(row,0).data().toString();
    QDialog *dialog = new QDialog(this);

    dialog->setWindowModality(Qt::WindowModal);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    QLabel *ssidLabel = common.getLabel("SSID:");
    QLabel *passLabel = common.getLabel("PASSWORD:");
    QLineEdit *ssid = common.getLineEdit();
    ssid->setText(apName);
    QLineEdit *passwd = common.getLineEdit();
    if(apName == "rpdzkj")
        passwd->setText("aaaaaaaa");
    QPushButton *conBtn = common.getButton("连接");
    QPushButton *closeBtn = common.getButton("关闭");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(closeBtn, &QPushButton::clicked, this, [=] {dialog->close();});
    connect(conBtn, &QPushButton::clicked, this, [=] {
        connectWifi(ssid->text(), passwd->text());
        dialog->close();
    });

    QGridLayout *layout = new QGridLayout;

    layout->addWidget(ssidLabel, 1, 1, 1, 1);
    layout->addWidget(ssid, 1, 2, 1, 2);
    layout->addWidget(passLabel, 2, 1, 1, 1);
    layout->addWidget(passwd, 2, 2, 1, 2);
    layout->addWidget(conBtn, 3, 1, 1, 1);
    layout->addWidget(closeBtn, 3, 3, 1, 1);

    dialog->setLayout(layout);
    dialog->show();
    thread->scanFlg = false;//stop scan
}

void Wifi::connectWifi(QString apName, QString passwd)
{
    QFile file("/tmp/wpa_supplicant.conf");
    file.remove();
    if(file.open(QFile::ReadWrite)){
        QString str = "ctrl_interface=/var/run/wpa_supplicant\n"
                      "network={\n"
                            "ssid=\""+apName+"\"\n"
                            "psk=\""+passwd+"\"\n"
                            //"key_mgmt=WPA2-PSK\n"
                      "}\n";
        file.write(str.toLocal8Bit());
    }
    title->setText("正在连接WIFI，请稍候。。。");

    ConnectThread *conThread = new ConnectThread;
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(conThread, &ConnectThread::getWlanIp, this, &Wifi::getConnectState);
    conThread->start();
}

void Wifi::getConnectState(QString ip)
{
    if(ip == ""){
        title->setText("WIFI连接失败!");
    }else{
        wlanIp->setText(ip);
        title->setText("WIFI连接成功!");
    }
}

void Wifi::pingTest(QString ip)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    QGridLayout *layout = new QGridLayout;
    QPushButton *closeBtn = common.getButton("关闭");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(closeBtn, &QPushButton::clicked, this, [=] {dialog->close();});

    if (ip == ""){
        qDebug()<<"please connect AP first";
        QLabel *label = common.getLabel("请先连接WIFI!");
        layout->addWidget(label, 1, 1, 1, 3);
        layout->addWidget(closeBtn, 2, 2, 1, 1);
    }else{
        QTextEdit *pingState = common.getTextEdit();
        layout->addWidget(pingState, 1, 1, 3, 9);
        layout->addWidget(closeBtn, 4, 5, 1, 1);
        dialog->resize(650,400);

        QString testCmd = "ping -I wlan0 www.baidu.com -i 1";
        QProcess *p = new QProcess;
        pingState->append(testCmd);
        connect(p, &QProcess::readyRead, this, [=] {pingState->append(p->readAll());});
        p->start("bash", QStringList() <<"-c" << testCmd);
        // p->waitForFinished();
    }
    dialog->setLayout(layout);
    dialog->show();
}




#include "wifistress.h"

WifiStress::WifiStress(QWidget *parent) : QWidget(parent)
{
    initThreads();
}

void WifiStress::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout;
    QString wlanHardWareAddr = "No WIFI MAC!!";

    chnLabel = common.getLabel("信道");
    freqLabel = common.getLabel("频率");
    wlanMacLabel = common.getLabel("WIFI MAC(热点名称)");
    wlanMac = common.getLineEdit();
    chnCombo = common.getComboBox();
    freqCombo = common.getComboBox();
    startClient = common.getButton("连接热点");
    startServer = common.getButton("开启热点");
    closeWindow = common.getButton("关闭窗口");

    QList<QNetworkInterface> ifaceList=QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, ifaceList) {
        QString interfaceName = interface.humanReadableName();
        if(interfaceName == "wlan0"){
            wlanHardWareAddr = interface.hardwareAddress();
            break;
        }
    }
    wlanMac->setText(wlanHardWareAddr);

    connect(startServer, &QPushButton::clicked, [=] {this->openHotPoint();});
    connect(startClient, &QPushButton::clicked, [=] {this->showConnectWindow();});
    connect(closeWindow, &QPushButton::clicked, [=] {this->close();});
    connect(freqCombo, &QComboBox::currentTextChanged, [=] {this->changeFreq(freqCombo->currentText());});

    layout->addWidget(wlanMacLabel, 1, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(wlanMac, 1, 2, 1, 2);
    layout->addWidget(freqLabel, 2, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(freqCombo, 2, 2, 1, 2);
    layout->addWidget(chnLabel, 3, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(chnCombo, 3, 2, 1, 2);
    layout->addWidget(startServer, 4, 1, 1, 1, Qt::AlignHCenter);
    layout->addWidget(startClient, 4, 2, 1, 1, Qt::AlignHCenter);
    layout->addWidget(closeWindow, 4, 3, 1, 1, Qt::AlignHCenter);

    this->setLayout(layout);
    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->initData();
}

void WifiStress::startThreads()
{
    scanThread->startScan();
    scanThread->startThread();
    scanThread->start();

    m_scanThread->start();
    m_fileThread->start();
    m_netCheckThread->start();
}

void WifiStress::exitThreads()
{
    scanThread->exitThread();
    scanThread->quit();
    scanThread->wait();

    m_scanThread->quit();
    m_scanThread->wait();

    netCheckThread->stopCheckNet();
    m_netCheckThread->quit();
    m_netCheckThread->wait();

    m_fileThread->quit();
    m_fileThread->wait();
}

void WifiStress::initThreads()
{
    //scan thread
    scanThread = new ScanThread;
    qRegisterMetaType<QList<QString>>("QList");
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");

    //connect thread
    m_scanThread = new QThread();
    connectThread = new WifiConnectThread;
    connectThread->moveToThread(m_scanThread);

    //file transfer thread
    fileTransferThread = new FileTransferThread();
    m_fileThread = new QThread;
    fileTransferThread->moveToThread(m_fileThread);

    //check thread
    netCheckThread = new NetWorkCheckThread();
    m_netCheckThread = new QThread;
    netCheckThread->moveToThread(m_netCheckThread);

    //check net thread
    connect(netCheckThread, &NetWorkCheckThread::netDisconneted, fileTransferThread, &FileTransferThread::handleNetDisconnected);
    connect(netCheckThread, &NetWorkCheckThread::netDisconneted, this, &WifiStress::socketBreak);

    //scan thread
    connect(scanThread, &ScanThread::scanApList, this, &WifiStress::handleApList);

    //connect thread
    connect(connectThread, &WifiConnectThread::connectState, this, &WifiStress::getConnectState);
    connect(connectThread, &WifiConnectThread::connectSuccess, fileTransferThread, &FileTransferThread::startDownLoad);
    connect(connectThread, &WifiConnectThread::allConnectFinished, [=] {scanThread->startScan();});
    connect(this, &WifiStress::connectWifi, connectThread, &WifiConnectThread::startConnectAp);

    //file transfer thread
    connect(fileTransferThread, &FileTransferThread::socketConnectSuccess, netCheckThread, &NetWorkCheckThread::startCheckNet);
    connect(fileTransferThread, &FileTransferThread::socketConnectTimeout, this, &WifiStress::socketTimeout);
    connect(fileTransferThread, &FileTransferThread::socketDisconnect, this, &WifiStress::socketDisconnect);
    connect(fileTransferThread, &FileTransferThread::downLoadFinished, this, &WifiStress::fileDownSuccess);
    connect(fileTransferThread, &FileTransferThread::fileSize, this, &WifiStress::initProgressBar);
    connect(fileTransferThread, &FileTransferThread::reciveSize, this, &WifiStress::updateProgressBar);
}

WifiStress::~WifiStress()
{
    qDebug()<<"~WifiStress!";
    fileTransferThread->deleteLater();
    netCheckThread->deleteLater();
    connectThread->deleteLater();
}

void WifiStress::initData()
{
    QStringList chnList;
    for(int i=1; i<=11; i++){
        chnList.append(QString::number(i));
    }
    chnCombo->addItems(chnList);
    freqCombo->addItem("2.4G");
    freqCombo->addItem("5G");
    //5G chnnel 149、153、157、161、165
}

void WifiStress::openHotPoint()
{
    chn = chnCombo->currentText();
    freq = freqCombo->currentText();
    mac = "rk3399-"+wlanMac->text();

    if(freq == "2.4G"){
        freq = "2.4";
    }else{
        freq = "5";
    }

    ::system("killall create_ap");
    ::system("killall socket_server");
    QString apCmd = QString("create_ap -n wlan0 %1 12345678 -c %2 --freq-band %3 --no-virt --daemon").arg(mac, chn, freq);
    ::system(apCmd.toLocal8Bit());

    ::system("rm /etc/init.d/S99_wlan0_ap.sh -rf");
    ::system(QString("echo %1 >> /etc/init.d/S99_wlan0_ap.sh").arg(apCmd).toLocal8Bit());
    ::system("echo socket_server & >> /etc/init.d/S99_wlan0_ap.sh");
    ::system("chmod a+x /etc/init.d/S99_wlan0_ap.sh");
    ::system("sync");

    startServer->setText("热点已开启");
    startServer->update();
}

void WifiStress::getConnectState(QString apName, bool state)
{
    qDebug()<<apName<<":"<<state;
    if(state)
        stateTxt->setText(QString(apName + " " + "连接成功!!!"));
    else
        stateTxt->setText(QString(apName + " " + "连接失败!!!"));
}


void WifiStress::handleApList(QList<QString> apList)
{
    qDebug()<<"handleApList";
//    wifiStressThread->setScanFlag(false);
    connectThread->setValidAplist(apList);
    scanThread->stopScan();

    int count = apList.count();
    QString his = QString("第%1次扫描个数：%2").arg(QString::number(scanTimes), QString::number(count));
    scanTimes ++;
    history->append(his);
    history->append(apList.join(", "));
    history->append("\n");

    count = count/3+count%3;
    int row = 0;
    int col = 0;
    apTable->setRowCount(count);
    for (int i = 0; i < apList.count(); ++i) {
        qDebug()<<apList.at(i);
        QTableWidgetItem *tableItem = new QTableWidgetItem(apList.at(i));
        if( i >= count && i % count == 0 ){
            row = 0;
            col ++;
        }
        apTable->setItem(row, col, tableItem);
        row ++;
    }
    apTable->update();
    dialog->update();
    emit connectWifi();
}

void WifiStress::showConnectWindow()
{
    this->startThreads();

    dialog = new QDialog(this);

    dialog->setWindowModality(Qt::WindowModal);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    dialog->setGeometry(0,0,1024,600);
    QGridLayout *layout = new QGridLayout();
    apTable = common.getTableWidget(3);
    apTable->horizontalHeader()->setHidden(true);
    apTable->verticalHeader()->setDefaultSectionSize(50);
    apTable->setDisabled(true);
    QFont font;
    font.setPixelSize(20);
    apTable->setFont(font);
    processBar = new QProgressBar();
    processBar->setStyleSheet("QProgressBar {height:45;border-radius:2;text-align:center}");
    stateLabel = common.getLabel("连接状态:");
    stateTxt = common.getLabel("");
    historyBtn = common.getButton("历史记录");
    closeDialogBtn = common.getButton("关闭");
    connect(historyBtn, &QPushButton::clicked, [=]{this->showHistory();});
    connect(closeDialogBtn, &QPushButton::clicked, [=]{
        this->exitThreads();
        this->dialog->close();
    });

    stateTxt->setFont(font);
    layout->addWidget(apTable, 1, 1, 1, 4);
    layout->addWidget(stateTxt, 2, 1, 1, 2);
    layout->addWidget(processBar, 3, 1, 1, 2);
    layout->addWidget(historyBtn, 3, 3, 1, 1);
    layout->addWidget(closeDialogBtn, 3, 4, 1, 1);
    dialog->setLayout(layout);
    dialog->show();
}

void WifiStress::showHistory()
{
    QDialog *historyDialog = new QDialog(this);
    historyDialog->setWindowModality(Qt::WindowModal);
    historyDialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    historyDialog->setGeometry(0,0,1024,600);

    QGridLayout *layout = new QGridLayout();
    QPushButton *closeBtn = common.getButton("关闭");
    connect(closeBtn, &QPushButton::clicked, [=]{historyDialog->close();});

    QFont font;
    font.setPixelSize(15);
    history->setFont(font);

    layout->addWidget(history);
    layout->addWidget(closeBtn);
    historyDialog->setLayout(layout);
    historyDialog->show();
}

void WifiStress::initProgressBar(qint64 fileSize)
{
//    qDebug()<<"init progress!!";
    processBar->setMinimum(0);
    processBar->setMaximum(fileSize/1024);
    processBar->setValue(0);
}

void WifiStress::updateProgressBar(qint64 reciveSize)
{
//    qDebug()<<"update progress!!";
    processBar->setValue(reciveSize/1024);
}

void WifiStress::fileDownSuccess()
{
    stateTxt->setText(stateTxt->text().append("文件传输成功!"));
    netCheckThread->stopCheckNet();
}

void WifiStress::socketTimeout()
{
    stateTxt->setText(stateTxt->text().append("文件传输连接超时!"));
    connectThread->connectContinue();
}

void WifiStress::socketDisconnect()
{
    connectThread->connectContinue();
    netCheckThread->stopCheckNet();
}

void WifiStress::socketBreak()
{
    stateTxt->setText(stateTxt->text().append("文件传输中断!"));
}

void WifiStress::changeFreq(QString freq)
{
    QStringList chnList;
    qDebug()<<"change freq to " << freq;
    if(freq == "5G"){
        chnList<<"149"<<"143"<<"157"<<"161"<<"165";
    }else{
        for(int i=1; i<=11; i++){
            chnList.append(QString::number(i));
        }
    }
    chnCombo->clear();
    chnCombo->addItems(chnList);
}

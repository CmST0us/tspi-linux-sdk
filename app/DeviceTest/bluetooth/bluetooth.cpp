#include "bluetooth.h"

Bluetooth::Bluetooth(QWidget *parent) : QWidget(parent)
{

}

void Bluetooth::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout();
    QFont font;
    font.setPixelSize(14);
    listWidget = new QListWidget();
    listWidget->setFont(font);
    QPushButton *tips = common.getButton("蓝牙扫描测试!");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");
    QPushButton *closeBtn = common.getButton("关闭窗口");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {this->closeWindow();});

    layout->addWidget(tips, 1, 1, 1, 5);
    layout->addWidget(listWidget, 2, 1, 1, 5);
    layout->addWidget(closeBtn,3, 3, 1, 1, Qt::AlignBottom);
    this->setLayout(layout);
    this->resize(w, h);
    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

    closeTimer = new QTimer(this);
    connect(closeTimer, &QTimer::timeout, this, [=] {this->closeWindow();});

    thread = new BluetoothThread;
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(thread, &BluetoothThread::getLeScanResult, this, &Bluetooth::setLeScanList);
    thread->start();
}

void Bluetooth::setLeScanList(QString str)
{
    listWidget->clear();
    if(str.startsWith("Scanning ...")){
        QStringList leList = str.split("\n\t");
        foreach (QString le, leList) {
            if(le.startsWith("Scanning"))
                continue;
            listWidget->addItem(le);
        }
    }

    testResult = listWidget->count() > 0;

    if(autoTest){
        thread->leScan=false;
        closeTimer->start(waitCloseSecond);
    }
}

void Bluetooth::closeWindow()
{
    closeTimer->stop();
    thread->leScan = false;
    thread->quit();

    //update config file state
    common.setConfig("BT", "state", QString::number(testResult));

    emit testFinish();
    this->close();
}

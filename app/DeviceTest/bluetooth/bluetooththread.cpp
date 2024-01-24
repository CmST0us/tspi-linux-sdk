#include "bluetooththread.h"

BluetoothThread::BluetoothThread()
{

}

void BluetoothThread::run()
{
//    qDebug()<<"run";
    openBluetooth();
    while (leScan) {
        scanBluetooth();
        msleep(5000);
    }

}

void BluetoothThread::openBluetooth()
{
    QFile hci("/sys/class/bluetooth/hci0");
    if(hci.exists()){
        ::system("hciconfig hci0 up ");
        return;
    }else{
        ::system("bt_init.sh");
        msleep(2000);
        ::system("hciconfig hci0 up ");
    }
}

void BluetoothThread::scanBluetooth()
{
    QFile fiel("/sys/class/bluetooth/hci0");
    if(!fiel.exists()){
        emit getLeScanResult("");
        return;
    }
    QProcess *p = new QProcess;
    connect(p, &QProcess::readyRead, this, [=] {this->dealReturn(p->readAll());});

    //ping -I eth0 192.168.1.1 -c 4
    QString testCmd = "hcitool scan";
    p->start("bash", QStringList() <<"-c" << testCmd);
    p->waitForFinished();
}

void BluetoothThread::dealReturn(QString returnTxt)
{
    qDebug() << returnTxt;
    emit getLeScanResult(returnTxt);
}

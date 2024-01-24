#ifndef BLUETOOTHTHREAD_H
#define BLUETOOTHTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "common/common.h"

class BluetoothThread : public QThread
{
    Q_OBJECT
public:
    BluetoothThread();

    void run() override;

    Common common;

    bool leScan = true;

protected:
    void openBluetooth();
    void scanBluetooth();
    void dealReturn(QString returnTxt);

signals:
    void getLeScanResult(QString str);
};

#endif // BLUETOOTHTHREAD_H

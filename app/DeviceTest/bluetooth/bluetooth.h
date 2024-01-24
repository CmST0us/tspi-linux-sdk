#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include "common/common.h"
#include "bluetooththread.h"

class Bluetooth : public QWidget
{
    Q_OBJECT
public:
    explicit Bluetooth(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    bool testResult = false;

protected:
    void closeWindow();
    Common common;
    QListWidget *listWidget;
    BluetoothThread *thread;
    QTimer *closeTimer;

signals:
    void testFinish();


protected slots:
    void setLeScanList(QString str);
};

#endif // BLUETOOTH_H

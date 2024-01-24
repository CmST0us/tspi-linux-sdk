#ifndef USBTHREAD_H
#define USBTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QDir>

class UsbThread : public QThread
{
    Q_OBJECT
public:
    UsbThread();

    void run() override;

    bool checkState = true;

protected:
    void checkUsbDevices();
    int usbDevCnt = 0;
    int usbHubCnt = 0;

signals:
    void getUsbDevices(int hubcnt, int devcnt);
};

#endif // USBTHREAD_H

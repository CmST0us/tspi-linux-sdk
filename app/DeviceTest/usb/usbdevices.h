#ifndef USBDEVICES_H
#define USBDEVICES_H

#include <QObject>
#include <QWidget>
#include "common/common.h"
#include "usbthread.h"

class USBDevices : public QWidget
{
    Q_OBJECT
public:
    explicit USBDevices(QWidget *parent = nullptr);
    void initWindow(int w, int h);
    bool testResult = false;

protected:
    void handleClick(bool result);

    Common common;
    QPushButton *tips;
    QPushButton *hubCnt;
    UsbThread *usbThread;
    QPushButton *powerBtn;

protected slots:
    void handleUsbDevices(int hubcnt, int devcnt);
    void powerCtrl();
    void setGPIOValue(QString gpio, QString value);
signals:
    void testFinish();
};

#endif // USBDEVICES_H

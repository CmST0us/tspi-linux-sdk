#include "usbthread.h"

#define USB_DEVICE_PATH "/sys/bus/usb/devices/"
UsbThread::UsbThread()
{

}

void UsbThread::run()
{
    qDebug()<<"run";
    while(checkState){
        checkUsbDevices();
        msleep(500);
    }
}

void UsbThread::checkUsbDevices()
{
    usbHubCnt = 0;
    usbDevCnt = 0;
    QDir dir(USB_DEVICE_PATH);
    for(QFileInfo info : dir.entryInfoList()){
        if(info.fileName() == "." || info.fileName() == "..")
            continue;
        QFile file(QString(info.filePath() + "/product"));
        if(file.exists()){
            file.open(QIODevice::ReadOnly);
            QString product = file.readAll();

            if(product.indexOf("Controller") >= 0 || product.indexOf("controller") >= 0)
                continue;

            if(product.indexOf("Hub") >= 0)
                usbHubCnt ++;
            else
                usbDevCnt ++;
        }
    }
    emit getUsbDevices(usbHubCnt, usbDevCnt);
}

#include "devicethread.h"

DeviceThread::DeviceThread()
{

}

void DeviceThread::run()
{
    getCpuModle();
    getCoreNum();
    getDDRSize();
    getEMMCSIze();
}

void DeviceThread::getCpuModle()
{
    QString detial = common.execLinuxCmd("cat /proc/device-tree/compatible");
    emit endGetCpuModle(detial, detial);
}

void DeviceThread::getCoreNum()
{
    QString detial = common.execLinuxCmd("cat /proc/cpuinfo");
    QStringList strlist = detial.split("\n");
    int coreNum = 0;
    foreach (QString str, strlist) {
        if (str.startsWith("processor"))
            coreNum ++;
    }
    emit endGeCoreNum(QString::number(coreNum), detial);
}

void DeviceThread::getDDRSize()
{
    QString detial = common.execLinuxCmd("cat /proc/meminfo");
    QStringList strlist = detial.split("\n");
    QString DDRSize;
    foreach (QString str, strlist){
        if (str.startsWith("MemTotal")){
            DDRSize = str.replace(QRegExp("\\s{1,}"), " ").split(" ").at(1);
            break;
        }
    }

    long size = DDRSize.toLong();
    if(size <= 512*1024){
        size = size / 1024;
        emit endGetDDRSize(QString::number(size) + "M", detial);
    }else{
        size = size / 1048576 + 1; //1024*1024
        emit endGetDDRSize(QString::number(size) + "G", detial);
    }
}

void DeviceThread::getEMMCSIze()
{
    QString sizeStr;
    long totalSize = 0;
    QString detial = common.execLinuxCmd("fdisk -l");

    // cat /sys/block/mmcblk0/device/type get MMC type
    // cat /sys/block/mmcblk0/size  get MMC Size

    for(int i=0; i <= 5; i++){
        QString mmcTypePath = QString("/sys/block/mmcblk%1/device/type").arg(QString::number(i));
        QString mmcSizePath = QString("/sys/block/mmcblk%1/size").arg(QString::number(i));
        QFile mmcTypeFile(mmcTypePath);
        if(mmcTypeFile.exists()){
            mmcTypeFile.open(QIODevice::ReadOnly);
            QString type = mmcTypeFile.readAll();
            qDebug()<<type;
            mmcTypeFile.close();

            if(type == "MMC\n"){
                QFile mmcSizeFile(mmcSizePath);
                mmcSizeFile.open(QIODevice::ReadOnly);
                sizeStr = mmcSizeFile.readAll();
                qDebug()<<sizeStr;
                mmcSizeFile.close();
                break;
            }
        }
    }

    totalSize = sizeStr.toLong()*512/1024/1024/1024 + 1;

    if (totalSize <= 8 && totalSize >=7)
        totalSize = 8;

    if (totalSize <= 16 && totalSize >=15)
        totalSize = 16;

    if (totalSize <= 32 && totalSize >=28)
        totalSize = 32;

    if (totalSize <= 64 && totalSize >=56)
        totalSize = 64;

    if (totalSize <= 128 && totalSize >=110)
        totalSize = 128;

    emit endGetEMMCSize(QString::number(totalSize)+"G", detial);
}

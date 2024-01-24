#ifndef DEVICETHREAD_H
#define DEVICETHREAD_H

#include <QThread>
#include <QDebug>
#include <QObject>
#include <QStorageInfo>
#include <QDir>

#include "common/common.h"

class DeviceThread : public QThread
{
    Q_OBJECT
public:
    DeviceThread();

    void getCpuModle();

    void getCoreNum();

    void getDDRSize();

    void getEMMCSIze();

    QString traverseDir(QDir dir);

    void run();

    Common common;

signals:
    void endGetCpuModle(QString, QString);
    void endGeCoreNum(QString, QString);
    void endGetDDRSize(QString, QString);
    void endGetEMMCSize(QString, QString);

};

#endif // DEVICETHREAD_H

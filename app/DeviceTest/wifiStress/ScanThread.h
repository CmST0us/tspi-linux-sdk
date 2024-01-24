#ifndef SCANTHREAD_H
#define SCANTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "common/common.h"

class ScanThread : public QThread
{
    Q_OBJECT
public:
    ScanThread();

    void run() override;
    void startScan();
    void stopScan();
    void exitThread();
    void startThread();

private:
    void scanAp();

private:
    Common common;
    bool scanFlag = false;
    bool scanThreadOut = false;

signals:
    void scanApList(QList<QString> apList);
};

#endif // SCANTHREAD_H

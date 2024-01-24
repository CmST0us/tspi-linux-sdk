#ifndef NPUTHREAD_H
#define NPUTHREAD_H

#include <QObject>
#include <QThread>
#include "common/common.h"

class NPUThread : public QThread
{
    Q_OBJECT
public:
    NPUThread();

    void run() override;

protected:
    void checkPCIdev();
    void runNPUDemo();

    Common common;
    bool connectState = false;

signals:
    void checkConnect(bool state);
    void runDemoState(bool state, QString detail);
};

#endif // NPUTHREAD_H

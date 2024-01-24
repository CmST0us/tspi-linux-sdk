#ifndef STRESSTHREAD_H
#define STRESSTHREAD_H

#include <QObject>
#include <QThread>
#include <QFile>
#include "common/common.h"

class StressThread : public QThread
{
    Q_OBJECT
public:
    StressThread();

    void run() override;

    bool testStart = true;

protected:
    void getCpu();
    QString getCpuLoad();
    QString getCpuFreq();
    QString getCpuTemp();

    void getDDR();
    QString getDDRFreq();
    QString getDDRLoad();

    Common common;

signals:
    void getCpuInfo(QString cpuload, QString cpufreq, QString cputemp);
    void getDdrInfo(QString ddrload, QString ddrfreq);
};

#endif // STRESSTHREAD_H

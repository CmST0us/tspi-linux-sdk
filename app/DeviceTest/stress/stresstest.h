#ifndef STRESSTEST_H
#define STRESSTEST_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include "common/common.h"
#include "stressthread.h"

class StressTest : public QWidget
{
    Q_OBJECT
public:
    explicit StressTest(QWidget *parent = nullptr);

    void initWindow(int w, int h);

protected:
    Common common;
    QLabel *cpuStress;
    QLabel *cpuLoadLable;
    QLabel *cpuLoad ;
    QLabel *cpuTempLabel;
    QLabel *cpuTemp;
    QLabel *cpuFreqLabel;
    QLabel *cpuFreq;

    QLabel *ddrStress;
    QLabel *ddrLoadLable;
    QLabel *ddrLoad;
    QLabel *ddrFreqLabel;
    QLabel *ddrFreq;
    StressThread *thread;
    QTimer *timer;
    QLabel *cutDownTime;
    bool showDilogFlag = false;
    void closeWindow();
    void updateTime();

protected slots:
    void handleCpuInfo(QString cpuload, QString cpufreq, QString cputemp);
    void handleDdrInfo(QString ddrload, QString ddrfreq);
    void getDDRTestInfo();
    void showDialog(QString ddrTestTxt);

signals:

};

#endif // STRESSTEST_H

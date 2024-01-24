#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QMetaType>
#include "common/common.h"
#include "devicethread.h"

class DeviceInfo : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceInfo(QWidget *parent = nullptr);
    void initWindow(int w, int h);
    void closeWindow(bool result);
    bool testResult;

    Common common;
    QLineEdit *cpuModel;
    QLineEdit *cpuNum;
    QLineEdit *ddrSize;
    QLineEdit *emmcSize;
    QTextEdit *detial;
    DeviceThread *devthread;

public slots:
    void setCpuModel(QString modle, QString detial);
    void setCpuNum(QString num, QString detial);
    void setDDRSize(QString size, QString detail);
    void setEMMCSize(QString size, QString detail);

signals:
    void testFinish();

};

#endif // DEVICEINFO_H

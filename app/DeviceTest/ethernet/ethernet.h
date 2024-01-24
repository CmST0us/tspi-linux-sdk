#ifndef ETHERNET_H
#define ETHERNET_H

#include <QObject>
#include <QWidget>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QList>
#include "common/common.h"
#include "eththread.h"
#include "ethnetautotest.h"

class EtherNet : public QWidget
{
    Q_OBJECT
public:
    explicit EtherNet(QWidget *parent = nullptr);
    void initWindow(int w, int h);
    bool testResult;

protected:
    Common common;
    QTextEdit *detial;
    EthThread *thread;
    EthnetAutoTest *autoTestThread;
    int successCount = 0;
    int ethCount = 0;

protected slots:
    void displayInfo(QString txt);
    void handleTestBtnClick(bool pingTestFlag, QString targetIp , QString eth , QString targetAddr);
    void getResult(bool result, QString eth, QString ipaddr);
    void getIp(QString eth, QString ip);
    void closeWindow();
    void autoPing(QString eth);
    void handleAllTestFinish();
    QString getGateway();

signals:
    void testFinish();
};

#endif // ETHERNET_H

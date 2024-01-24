#ifndef ETHNETAUTOTEST_H
#define ETHNETAUTOTEST_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QNetworkInterface>
#include "common/common.h"

class EthnetAutoTest : public QThread
{
    Q_OBJECT
public:
    EthnetAutoTest();

    void run() override;

    bool testFinish = false;

    int successCount = 0;

protected:
    void getEth();
    void waitTestFinish();

    Common common;

signals:
    void testEth(QString eth);
    void allEthTestFinish();
};

#endif // ETHNETAUTOTEST_H

#ifndef WIFITHREAD_H
#define WIFITHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMap>
#include <QTextCodec>
#include "common/common.h"

class WifiThread : public QThread
{
    Q_OBJECT
public:
    WifiThread();

    void run() override;
    void openWifi();
    void scanAp();
    bool scanFlg=true;
    Common common;

signals:
    void getApMap(QList<QMap<QString, QString>> infoList);
};

#endif // WIFITHREAD_H

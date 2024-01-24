#ifndef ETHTHREAD_H
#define ETHTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QHostAddress>
#include <QNetworkInterface>

class EthThread : public QThread
{
    Q_OBJECT
public:
    EthThread();

    void run() override;

    void pingTest();
    void getReticleStat();
    QString getIp(QList<QNetworkAddressEntry> ettry);

    bool pingTestFlag = false;
    bool iperfTestFlag = false;
    bool result=false;
    bool readFlag=true;
    QString targetIp;
    QString targetAddr;
    QString eth;
    QString ip;

public slots:
    void dealReturn(QString returnTxt);

signals:
    void pingTestFinish(QString text);
    void setResult(bool result, QString eth, QString ipaddr);
    void setIp(QString eth, QString ip);
};

#endif // ETHTHREAD_H

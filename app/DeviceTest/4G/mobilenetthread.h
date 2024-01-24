#ifndef MOBILENETTHREAD_H
#define MOBILENETTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QNetworkInterface>
#include <QProcess>
#include <QFile>
#include <QTimer>

//#define USE_GOBINET //current only for gaussian

class MobileNetThread : public QThread
{
    Q_OBJECT
public:
    MobileNetThread();

    void run() override;

    bool checkMobileNet();

protected:

    void pppDial();

//    void dealReturn(QString str);

    QString pppIp = "";

signals:
    void getPppIP(QString ip);
    void getReturnStr(QString str);

protected slots:
    void dealReturn(QString str);
};

#endif // MOBILENETTHREAD_H

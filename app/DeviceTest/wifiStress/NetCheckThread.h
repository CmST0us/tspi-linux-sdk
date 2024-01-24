#ifndef NETCHECKTHREAD_H
#define NETCHECKTHREAD_H

#include <QObject>
#include <QThread>
#include <QProcess>
#include <QDebug>

class NetWorkCheckThread : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkCheckThread(QObject *parent = nullptr);

public slots:
    void startCheckNet();
    void stopCheckNet();

private:
    void testNet();

private:
    bool checkNetWork = false;
    int times = 0;
    QProcess *p;

signals:
    void netDisconneted();
};

#endif // NETCHECKTHREAD_H

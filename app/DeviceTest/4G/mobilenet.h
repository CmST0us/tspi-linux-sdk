#ifndef MOBILENET_H
#define MOBILENET_H

#include <QObject>
#include <QWidget>
#include "common/common.h"
#include "mobilenetthread.h"

class MobileNet : public QWidget
{
    Q_OBJECT
public:
    explicit MobileNet(QWidget *parent = nullptr);

    void initWindow(int w, int h);
    bool testResult=false;

protected:
    Common common;
    QLineEdit *pppIp;
    void showDialog(int row);
    void pingTest(QString ip);
    void closeWindow();

    MobileNetThread *mobleNetThread;
    QPushButton *title;
    QTextEdit *detial;
    QTimer *timer;
    int timeoutTimes = 0;
    QTimer *closeTimer;

public slots:
    void setPppIp(QString ip);
    void dealReturnStr(QString txt);
    void readLog();

signals:
    void testFinish();
};

#endif // MOBILENET_H

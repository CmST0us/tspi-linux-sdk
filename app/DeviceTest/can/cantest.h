#ifndef CANTEST_H
#define CANTEST_H

#include <QObject>
#include <QWidget>
#include <QLayout>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QCanBus>
#include <QCanBusDevice>
#include <QFile>
#include <QElapsedTimer>

#include "common/common.h"

class CANTest : public QWidget
{
    Q_OBJECT
public:
    explicit CANTest(QWidget *parent = nullptr);

    void processErrors(QCanBusDevice::CanBusError error) const;
    void slotTime(QString name);
    void processReceivedFrames(QString name);
    void initWindow(int w, int h);
    void handleClick(bool result);
    void sendFrame(const QCanBusFrame &writeframe) const;
    void serialsend(QString plugin,QString name,const int bitrate);

    Common common;
    QTextEdit *reciveMessage;
    QTextEdit *payloadEdit;
    QTextEdit *frameldEdit;
    QTimer *timer;
    QList<QCanBusDeviceInfo> m_interfaces;
    QCanBusDevice *m_canDevice;
    QCanBusFrame writeframe;
    QPushButton *send;


    int canNum;
    bool testResult;

signals:
    void testFinish();

};

#endif // CANTEST_H

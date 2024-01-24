#ifndef UART_H
#define UART_H

#include <QObject>
#include <QLayout>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

#include "common/common.h"

class Uart : public QWidget
{
    Q_OBJECT
public:
    explicit Uart(QWidget *parent = nullptr);

    void initWindow(int w, int h);

    void reciveData(QSerialPort *testSerial);
    void sendSerialData(QString message, QSerialPort *testSerial);
    void getTestResult();

    Common common;
    QTextEdit *reciveMessage;
    QTextEdit *sendMessage;
    QTimer *timer;
    int uartNum;
    bool testResult;

signals:
    void testFinish();
};

#endif // UART_H

#include "uart.h"

static QString testText = "www.rpdzkj.com";
Uart::Uart(QWidget *parent) : QWidget(parent)
{

}

void Uart::initWindow(int w, int h)
{
    uartNum = 0;
    timer = new QTimer(this);
    QString ignoreTTY = common.getConfig("serial", "ignore_tty");

    QHBoxLayout *boxLayout_h = new QHBoxLayout();
    QVBoxLayout *boxLayout_v = new QVBoxLayout(); //垂直
    QHBoxLayout *btnLayout_h = new QHBoxLayout();
    QGridLayout *uartLayout = new QGridLayout();

    QLabel *reciveLabel = common.getLabel("接收端：");
    QLabel *sendLabel = common.getLabel("发送端：");
    reciveMessage = common.getTextEdit();
//    reciveMessage->setDisabled(true);
    reciveMessage->setText("短接tx和rx,自动收发!");

    sendMessage = common.getTextEdit();
    sendMessage->setText(testText);

    QLabel *serial = common.getLabel("串口:");
    QLabel *baudSel = common.getLabel("波特率:");
    QLabel *dataBitSel = common.getLabel("数据位:");
    QLabel *stopBitSel = common.getLabel("停止位:");
    QLabel *flowContrlSel = common.getLabel("流控:");

    QGridLayout *serialLayout = new QGridLayout();


    QLineEdit *baudRate = common.getLineEdit();
    baudRate->setText("115200");
    baudRate->setDisabled(true);

    QLineEdit *dataBit = common.getLineEdit();
    dataBit->setText("8");
    dataBit->setDisabled(true);

    QLineEdit *stopBit = common.getLineEdit();
    stopBit->setText("1");
    stopBit->setDisabled(true);

    QLineEdit *flowContrl = common.getLineEdit();
    flowContrl->setText("0");
    flowContrl->setDisabled(true);

    QPushButton *closeWindow = common.getButton("关闭窗口");
    closeWindow->setStyleSheet(closeWindow->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeWindow, &QPushButton::clicked, [=](){this->getTestResult();});

    QPushButton *cleanBtn = common.getButton("清空数据");
    connect(cleanBtn, &QPushButton::clicked, reciveMessage, &QTextEdit::clear);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            if(serial.portName().indexOf("ttyUSB") < 0 && ignoreTTY.indexOf(serial.portName()) < 0)
            {
                uartNum++;
                QPushButton *serialBtn = common.getButton(serial.portName());
                QSerialPort *testSerial = new QSerialPort(this);

                testSerial->setPortName(serial.portName());
                testSerial->open(QIODevice::ReadWrite);
                testSerial->setBaudRate(QSerialPort::Baud115200);
                testSerial->setDataBits(QSerialPort::Data8);
                testSerial->setParity(QSerialPort::NoParity);
                testSerial->setStopBits(QSerialPort::OneStop);
                testSerial->setFlowControl(QSerialPort::NoFlowControl);

                serialLayout->addWidget(serialBtn, uartNum, 1);

                connect(testSerial, &QSerialPort::readyRead, [=] { reciveData(testSerial); });
                connect(timer, &QTimer::timeout, [=] {
                    this->sendSerialData(sendMessage->toPlainText().toLatin1(),testSerial);
                });
            }
        }
    }

    //for can not auto detect uart
    if( uartNum == 0 ){
        QString allTtyTxt = common.execLinuxCmd("ls /dev/tty*");

        QStringList ttyList = allTtyTxt.split("\n");
        for(QString tty : ttyList){
            QString needTty = tty.replace("/dev/", "");
            if(needTty == "")
                continue;

            QString tmp = needTty;
            if(tmp.replace("tty","").contains(QRegExp("^\\d+$")))
                continue;

            QSerialPort serial;
            serial.setPortName(needTty);
            qDebug()<<serial.portName();
            if(serial.portName().indexOf("ttyUSB") < 0 && ignoreTTY.indexOf(serial.portName()) < 0){
                uartNum++;
                QPushButton *serialBtn = common.getButton(serial.portName());
                QSerialPort *testSerial = new QSerialPort(this);

                testSerial->setPortName(serial.portName());
                testSerial->open(QIODevice::ReadWrite);
                testSerial->setBaudRate(QSerialPort::Baud115200);
                testSerial->setDataBits(QSerialPort::Data8);
                testSerial->setParity(QSerialPort::NoParity);
                testSerial->setStopBits(QSerialPort::OneStop);
                testSerial->setFlowControl(QSerialPort::NoFlowControl);

                serialLayout->addWidget(serialBtn, uartNum, 1);

                connect(testSerial, &QSerialPort::readyRead, [=] { reciveData(testSerial); });
                connect(timer, &QTimer::timeout, [=] {
                    this->sendSerialData(sendMessage->toPlainText().toLatin1(),testSerial);
                });
            }
        }
    }

    boxLayout_v->addWidget(reciveLabel);
    boxLayout_v->addWidget(reciveMessage);
    boxLayout_v->addWidget(sendLabel);
    boxLayout_v->addWidget(sendMessage);
    btnLayout_h->addWidget(cleanBtn);
    boxLayout_v->addLayout(btnLayout_h);
    uartLayout->addWidget(serial, 1, 1, Qt::AlignRight);
    uartLayout->addLayout(serialLayout, 1, 2);
    uartLayout->addWidget(baudSel, 2, 1, Qt::AlignRight);
    uartLayout->addWidget(baudRate, 2, 2);
    uartLayout->addWidget(dataBitSel, 3, 1, Qt::AlignRight);
    uartLayout->addWidget(dataBit, 3, 2);
    uartLayout->addWidget(stopBitSel, 4, 1, Qt::AlignRight);
    uartLayout->addWidget(stopBit, 4, 2);
    uartLayout->addWidget(flowContrlSel, 5, 1, Qt::AlignRight);
    uartLayout->addWidget(flowContrl, 5, 2);
    uartLayout->addWidget(closeWindow, 6, 2);
    boxLayout_h->addLayout(boxLayout_v);
    boxLayout_h->addLayout(uartLayout);
    this->setLayout(boxLayout_h);
    this->resize(w,h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->showFullScreen();
    timer->start(100);
}

void Uart::reciveData(QSerialPort *testSerial)
{
    QByteArray buf;
    buf = testSerial->readAll();
    if(!buf.isEmpty()){
        QString dis = buf;
        if(dis == testText){
            reciveMessage->append(testSerial->portName() + ":" + dis);
            reciveMessage->moveCursor(QTextCursor::End);
        }else
            return;

        QList<QPushButton *> buttons = this->findChildren<QPushButton *>();
        for (QPushButton *btn : buttons) {
            if(btn->text() == testSerial->portName()){
                btn->setStyleSheet(btn->styleSheet()+"QPushButton{border-image:url(:/icon/pass.png)}");
            }
             btn->update();
        }
    }

    buf.clear();
}

void Uart::sendSerialData(QString message, QSerialPort *testSerial)
{
    testSerial->open(QIODevice::ReadWrite);

    if( testSerial != nullptr && testSerial->isOpen())

        testSerial->write(message.toLocal8Bit());

    else
        reciveMessage->append("请先选择串口！");
}

void Uart::getTestResult()
{
    QList<QPushButton *> buttons = this->findChildren<QPushButton *>();
    int successCount = 0;
    for (QPushButton *btn : buttons) {
        if(btn->text().startsWith("tty")){
            if(btn->styleSheet().indexOf("pass") >= 0)
                successCount ++;
        }
    }

    if(successCount == uartNum)
        testResult = true;
    else
        testResult = false;

    common.setConfig("serial", "state", QString::number(testResult));
    emit testFinish();
    this->close();
    this->deleteLater();
}



#include "cantest.h"

static QString testText = "112233445566";
CANTest::CANTest(QWidget *parent) : QWidget(parent)
{
    ::system("ifconfig can0 down");
    ::system("ip link set can0 type can bitrate 100000");
    ::system("ifconfig can0 up");
    ::system("ifconfig can1 down");
    ::system("ip link set can1 type can bitrate 100000");
    ::system("ifconfig can1 up");
    ::system("ifconfig can2 down");
    ::system("ip link set can2 type can bitrate 100000");
    ::system("ifconfig can2 up");
    ::system("ifconfig can3 down");
    ::system("ip link set can3 type can bitrate 100000");
    ::system("ifconfig can3 up");
}

void CANTest::initWindow(int w, int h)
{
    timer = new QTimer(this);

    QHBoxLayout *boxLayout_h = new QHBoxLayout();
    QVBoxLayout *boxLayout_v = new QVBoxLayout(); //垂直
    QHBoxLayout *btnLayout_h = new QHBoxLayout();
    //QHBoxLayout *sendLayout_h = new QHBoxLayout();
    QGridLayout *canLayout = new QGridLayout();

    QLabel *reciveLabel = common.getLabel("接收端：");
    QLabel *sendLabel = common.getLabel("发送端：");
    reciveMessage = common.getTextEdit();
    reciveMessage->setText(" 等待接收信息!");

    frameldEdit = common.getTextEdit();
    frameldEdit->setText("123");
    payloadEdit = common.getTextEdit();
    payloadEdit->setText(testText);

    QLabel *can = common.getLabel("can:");
    QLabel *baudSel = common.getLabel("波特率:");

    QGridLayout *canport = new QGridLayout();

    QPushButton *passBtn = common.getButton("测试成功");
    QPushButton *faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(faildBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    QPushButton *cleanBtn = common.getButton("清空数据");
    connect(cleanBtn, &QPushButton::clicked, reciveMessage, &QTextEdit::clear);

    send = common.getButton("发送");
    connect(send, &QPushButton::clicked, [=] {
        const uint frameId = frameldEdit->toPlainText().toUInt(nullptr, 16);;
        QString data = payloadEdit->toPlainText();
        const QByteArray payload = QByteArray::fromHex(data.remove(QLatin1Char(' ')).toLatin1());

       writeframe = QCanBusFrame(frameId, payload);
        this->sendFrame(writeframe);
    });

    const int bitrate=100000;
    //bool connectflag=false;
    QLineEdit *baudRate = common.getLineEdit();
    baudRate->setText(QString::number(bitrate));
    baudRate->setDisabled(true);

    m_canDevice=nullptr;
    const QString plugin="socketcan";
    //QString errorString;
    m_interfaces = QCanBus::instance()->availableDevices(plugin);
    //foreach (const QCanBusDeviceInfo &info,m_interfaces)
    for(canNum=0; canNum < m_interfaces.size();)
    {
     if(!m_interfaces.empty())
        {
            qDebug()<<m_interfaces.at(canNum).name();
            QString name = m_interfaces.at(canNum).name();
            /*m_canDevice=QCanBus::instance()->createDevice(plugin, m_interfaces.at(canNum).name(), &errorString);
            m_canDevice->setConfigurationParameter(QCanBusDevice::BitRateKey,QVariant(bitrate));
            name = m_interfaces.at(canNum).name();
            connectflag= m_canDevice->connectDevice();
            if(connectflag)
               {
                   connect(m_canDevice, &QCanBusDevice::errorOccurred,
                            this,&CANTest::processErrors);
                   connect(m_canDevice, &QCanBusDevice::framesReceived,
                           [=] {processReceivedFrames(name);});
                   connect(timer, &QTimer::timeout, [=] {
                                this->slotTime(name);});
                   timer->start(10000);
                   qDebug()<<"name:"<<name;
                   //ui->canBT->setEnabled(false);
              }
            qDebug()<<"connecflag"<<connectflag;*/
            QPushButton *serialBtn = common.getButton(m_interfaces.at(canNum).name());
            connect(serialBtn,&QPushButton::clicked, [=]{
                this->serialsend(plugin,name,bitrate);});
            canport->addWidget(serialBtn,canNum,1);
            canNum++;

        }
        else
        {
            qDebug()<<"do not find available interface for pcan";
        }
    }

     boxLayout_v->addWidget(reciveLabel);
     boxLayout_v->addWidget(reciveMessage);
     boxLayout_v->addWidget(sendLabel);
     boxLayout_v->addWidget(frameldEdit);
     boxLayout_v->addWidget(payloadEdit);
     boxLayout_v->addWidget(send);
     btnLayout_h->addWidget(cleanBtn);
     boxLayout_v->addLayout(btnLayout_h);

     canLayout->addWidget(can, 1, 1, Qt::AlignRight);
     canLayout->addLayout(canport, 1, 2);
     canLayout->addWidget(baudSel, 2, 1, Qt::AlignRight);
     canLayout->addWidget(baudRate, 2, 2);
     canLayout->addWidget(passBtn, 3, 2,Qt::AlignBottom);
     canLayout->addWidget(faildBtn, 4, 2,Qt::AlignBottom);

     boxLayout_h->addLayout(boxLayout_v);
     boxLayout_h->addLayout(canLayout);
     this->setLayout(boxLayout_h);
     this->resize(w,h);
     this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
     this->showFullScreen();

}
void CANTest::serialsend(QString plugin, QString name,const int bitrate)
{
    QString equipment = QString("设备%1已连接!").arg(name);
    reciveMessage->append(equipment);
    bool connectflag=false;
    QString errorString;
    m_canDevice=QCanBus::instance()->createDevice(plugin, name, &errorString);
    m_canDevice->setConfigurationParameter(QCanBusDevice::BitRateKey,QVariant(bitrate));

    connectflag= m_canDevice->connectDevice();
    if(connectflag)
       {
           connect(m_canDevice, &QCanBusDevice::errorOccurred,
                    this,&CANTest::processErrors);
           connect(m_canDevice, &QCanBusDevice::framesReceived,
                   [=] {processReceivedFrames(name);});

      }


}
/*void CANTest::slotTime(QString name)
{
    static int i=0;
    for(; i < 10;i++)
    {
        qDebug()<<"time:"<<i;
    }

    connect(m_canDevice, &QCanBusDevice::errorOccurred,
             this,&CANTest::processErrors);
    connect(m_canDevice, &QCanBusDevice::framesReceived,
            [=] {processReceivedFrames(name);});
    emit send->click();
}
*/
void CANTest::processErrors(QCanBusDevice::CanBusError error) const
{
    switch (error) {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        qDebug()<<m_canDevice->errorString();
        m_canDevice->disconnectDevice();
        qDebug()<<"设备已断开!!";
        break;
    default:
        break;
    }
}

void CANTest::processReceivedFrames(QString name)
{
    if (!m_canDevice)
        return;
    while (m_canDevice->framesAvailable())
    {
        const QCanBusFrame frame = m_canDevice->readFrame();
        //qDebug()<<frame.error();
        // qDebug()<<frame.frameId();//frame id,it's decimal
        //qDebug()<<frame.frameType();//1 means dataframe
        //qDebug()<<m_canDevice->state();

        QString view;

        if (frame.frameType() == QCanBusFrame::ErrorFrame)
        {
            view = m_canDevice->interpretErrorFrame(frame);
            qDebug()<<"This is Framer!";
            m_canDevice->disconnectDevice();
            qDebug()<<"设备已断开!!!";
        }
        else if(frame.frameType()==QCanBusFrame::DataFrame)
        {
            view=frame.toString();

        }
        if(view != "")
        {
            QString Frame = QString("%1:%2").arg(name).arg(view);
            reciveMessage->append(Frame);
            view.clear();

        }

    }
}

void CANTest::sendFrame(const QCanBusFrame &writeframe)const
{
    if (!m_canDevice)
        return;
    m_canDevice->writeFrame(writeframe);
}


void CANTest::handleClick(bool result)
{
    if(timer){
        timer->stop();
    }
    testResult = result;

    //update config file state
    common.setConfig("CAN", "state", QString::number(result));

    emit testFinish();
    this->close();
}

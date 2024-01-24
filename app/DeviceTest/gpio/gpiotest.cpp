#include "gpiotest.h"

static QComboBox *gpios;
static QLineEdit *gpioValue;

GPIOTest::GPIOTest(QWidget *parent) : QWidget(parent)
{

}

void GPIOTest::initWindow(int w, int h)
{
    layout = new QGridLayout();

    gpios = common.getComboBox();
    gpioLabel = common.getLabel("GPIO:");
    pullUp = common.getButton("拉高");
    pullDown = common.getButton("拉低");
    readBtn = common.getButton("读取");

    pullDownAll = common.getButton("全部拉低");
    pullUpAll = common.getButton("全部拉高");

    valueLabel = common.getLabel("Value:");
    gpioValue = common.getLineEdit();

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(faildBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    gpioValue->setDisabled(true);

    //get all gpios
    gpios->addItems(getAllGpios());
    connect(pullUp, &QPushButton::clicked, [=] { this->setValue(1); });
    connect(pullDown, &QPushButton::clicked, [=] { this->setValue(0); });

    connect(pullUpAll, &QPushButton::clicked, [=] { this->setAllValue(1); });
    connect(pullDownAll, &QPushButton::clicked, [=] { this->setAllValue(0); });

    connect(readBtn, &QPushButton::clicked, [=] { this->readValue(); });

    layout->addWidget(gpioLabel, 1, 1, 1, 2, Qt::AlignBottom);
    layout->addWidget(gpios, 1, 3, 1, 4, Qt::AlignBottom);
    layout->addWidget(pullUp, 2, 1, 1, 2, Qt::AlignBottom);
    layout->addWidget(pullDown, 2, 3, 1, 2, Qt::AlignBottom);
    layout->addWidget(readBtn, 2, 5, 1, 2, Qt::AlignBottom);

    layout->addWidget(pullUpAll, 3, 1, 1, 3, Qt::AlignBottom);
    layout->addWidget(pullDownAll, 3, 4, 1, 3, Qt::AlignBottom);

    layout->addWidget(valueLabel,4 ,1 ,1 ,2, Qt::AlignBottom);
    layout->addWidget(gpioValue, 4, 3, 1, 4, Qt::AlignBottom);
    layout->addWidget(passBtn, 5, 1, 1, 3, Qt::AlignBottom);
    layout->addWidget(faildBtn, 5, 4, 1, 3, Qt::AlignBottom);

    this->setLayout(layout);
    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
}

QStringList GPIOTest::getAllGpios()
{
    QStringList gpios;
    QDir dir("/proc/rp_gpio/");
    dir.setFilter(QDir::Files);
    if(!dir.exists()){
        //not found any exported gpio!!
         return gpios;
    }
    QFileInfoList list = dir.entryInfoList();
    int file_count = list.count();
    if(file_count <= 0){
        //not found any exported gpio!!
        return gpios;
    }

    for(int i=0; i<file_count; i++){
        QFileInfo file_info = list.at(i);
        qDebug()<<"found gpio:"<<file_info.fileName();
        gpios.append(file_info.fileName());
    }
    return gpios;
}

void GPIOTest::setValue(int value)
{
    if(QString(gpios->currentIndex()).isEmpty())
        return;

    qDebug()<<"current gpio:"<< gpios->currentText();
    QString cmd = QString("echo %1 > /proc/rp_gpio/%2").arg(value).arg(gpios->currentText());
    qDebug()<<cmd;
    ::system(cmd.toLocal8Bit());
    this->readValue();
}

void GPIOTest::setAllValue(int value)
{
//    QString ignoreGPIOs="gpio2b4_gpio1c4_gpio1b2_gpio1a3_gpio0b4";
    QString ignoreGPIOs = common.getConfig("GPIO", "ignore_gpios");
    QDir gpioDir("/proc/rp_gpio");
    for(QFileInfo info : gpioDir.entryInfoList()){
        if(info.fileName() == "." || info.fileName() == "..")
            continue;
        if(ignoreGPIOs.indexOf(info.fileName()) >= 0 ){
            qDebug()<<"ignore:"<<info.fileName();
            continue;
        }

        QString cmd = QString("echo %1 > /proc/rp_gpio/%2").arg(value).arg(info.fileName());
        qDebug()<<cmd;
        ::system(cmd.toLocal8Bit());
    }
}


void GPIOTest::readValue()
{
    if(QString(gpios->currentIndex()).isEmpty())
        return;

    QString path = "/proc/rp_gpio/" + gpios->currentText();
    qDebug()<<path;
    QFile gpioValueFile(path);
    if(!gpioValueFile.open(QIODevice::ReadOnly)){
        gpioValue->setText("gpio read value failed!");
    }else{
        QString value = gpioValueFile.readAll();
        if(value == "1\n"){
            gpioValue->setText("1");
        }else{
            gpioValue->setText("0");
        }
    }
    gpioValueFile.close();
}

void GPIOTest::handleClick(bool result)
{
    //setAllValue(1);
    //update config file state
    common.setConfig("GPIO", "state", QString::number(result));

    testResult = result;
    emit testFinish();
    this->close();
}

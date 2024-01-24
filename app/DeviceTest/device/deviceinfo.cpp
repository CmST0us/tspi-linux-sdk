#include "deviceinfo.h"

DeviceInfo::DeviceInfo(QWidget *parent) : QWidget(parent)
{

}

void DeviceInfo::initWindow(int w, int h)
{
    devthread = new DeviceThread;

    QGridLayout *layout = new QGridLayout();

    QPushButton *title = common.getButton("确认设备信息");
    title->setStyleSheet(title->styleSheet()+
                         "QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QLabel *cpuModelLabel = common.getLabel("CPU型号:");
    cpuModel = common.getLineEdit();
    cpuModel->setDisabled(true);

    QLabel *coreNumLabel = common.getLabel("CPU核心:");
    cpuNum = common.getLineEdit();
    cpuNum->setDisabled(true);

    QLabel *ddrSizeLabel = common.getLabel("DDR容量:");
    ddrSize = common.getLineEdit();
    ddrSize->setDisabled(true);

    QLabel *emmcSizeLabel = common.getLabel("EMMC容量:");
    emmcSize = common.getLineEdit();
    emmcSize->setDisabled(true);

    QPushButton *detialBtn = common.getButton("详细信息");
    connect(detialBtn, &QPushButton::clicked, [=] {detial->setVisible(!detial->isVisible());});
    detial = common.getTextEdit();
//    detial->setDisabled(true);
    detial->setVisible(false);

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *failBtn = common.getButton("测试失败");
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(passBtn, &QPushButton::clicked, [=] {closeWindow(true);});
    connect(failBtn, &QPushButton::clicked, [=] {closeWindow(false);});

    layout->addWidget(title, 1, 1, 1, 4);
    layout->addWidget(cpuModelLabel, 2, 1, 1, 1);
    layout->addWidget(cpuModel, 2, 2, 1, 1);
    layout->addWidget(coreNumLabel, 2, 3, 1, 1);
    layout->addWidget(cpuNum, 2, 4, 1, 1);
    layout->addWidget(ddrSizeLabel, 3, 1, 1, 1);
    layout->addWidget(ddrSize, 3, 2, 1, 1);
    layout->addWidget(emmcSizeLabel, 3, 3, 1, 1);
    layout->addWidget(emmcSize, 3, 4, 1, 1);
    layout->addWidget(detialBtn, 4, 1, 1, 4, Qt::AlignHCenter);
    layout->addWidget(detial, 5, 1, 1, 4);
    layout->addWidget(passBtn, 6, 1, 1, 2, Qt::AlignBottom);
    layout->addWidget(failBtn, 6, 3, 1, 2, Qt::AlignBottom);

    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(devthread, &DeviceThread::endGetCpuModle, this, &DeviceInfo::setCpuModel);
    connect(devthread, &DeviceThread::endGeCoreNum, this, &DeviceInfo::setCpuNum);
    connect(devthread, &DeviceThread::endGetDDRSize, this, &DeviceInfo::setDDRSize);
    connect(devthread, &DeviceThread::endGetEMMCSize, this, &DeviceInfo::setEMMCSize);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);
    this->show();
    devthread->start();
}

void DeviceInfo::setCpuModel(QString modle, QString detialTxt)
{
    cpuModel->setText(modle);
    detial->append("--------------------CPU型号----------------- ");
    detial->append(detialTxt);
    detial->update();
}

void DeviceInfo::setCpuNum(QString num, QString detialTxt)
{
    cpuNum->setText(num);
    detial->append("--------------------CPU信息----------------- ");
    detial->append(detialTxt);
    detial->update();
}

void DeviceInfo::setDDRSize(QString size, QString detialTxt)
{
    ddrSize->setText(size);
    detial->append("--------------------DDR信息----------------- ");
    detial->append(detialTxt);
    detial->update();
}

void DeviceInfo::setEMMCSize(QString size, QString detialTxt)
{
    emmcSize->setText(size);
    detial->append("--------------------EMMC分区信息----------------- ");
    detial->append(detialTxt);
    detial->update();
}

void DeviceInfo::closeWindow(bool result)
{
    testResult = result;
    common.setConfig("deviceInfo","state",QString::number(result));
    emit testFinish();
    devthread->quit();
    this->close();
}



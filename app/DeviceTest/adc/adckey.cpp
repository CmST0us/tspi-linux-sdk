#include "adckey.h"

AdcKey::AdcKey(QWidget *parent) : QWidget(parent)
{

}

void AdcKey::initWindow(int w, int h)
{
    QFormLayout *pLayout = new QFormLayout;
    QGridLayout *layout = new QGridLayout;
    QDir dir(ADC_PATH);

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *failBtn = common.getButton("测试失败");

    connect(passBtn, &QPushButton::clicked, [=] {this->handleClick(true);});
    connect(failBtn, &QPushButton::clicked, [=] {this->handleClick(false);});
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    int row = 1;
    QPushButton *title = common.getButton("ADC通道读取测试");
    title->setStyleSheet(title->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");
    layout->addWidget(title, row, 1, 1, 4);

    if(!dir.exists()){
        QDir allWinerDir(GPADC_PATH);
        if (allWinerDir.exists()){
            QLabel *chnnelLabel = common.getLabel(QString("GPADC:"));
            QLineEdit *chnnelEdit = common.getLineEdit();
            chnnelEdit->setObjectName("GPADC");
            chnnelEdit->setDisabled(true);
            pLayout->addRow(chnnelLabel, chnnelEdit);
            layout->addLayout(pLayout, ++row, 1, 1, 4);

            thread = new AdcThread;
            qRegisterMetaType<QVector<QString>>("QVector<QString>&");
            connect(thread, &AdcThread::getAdcValue, this, &AdcKey::setAdcValue);
            thread->start();
        }else{
            qDebug()<<"not found any adc group!";
            QLabel *noChnnel = common.getLabel("没有找到任何ADC通道！");
            layout->addWidget(noChnnel, ++row, 1, 1, 4);
        }
    }else{
        QFileInfoList list = dir.entryInfoList();
        foreach (QFileInfo groups, list) {
            int j = 0;
            if(groups.fileName().startsWith("."))
                continue;

            QPushButton *groupLabel = common.getButton("第"+QString::number(row++) + "组:");
            groupLabel->setStyleSheet(groupLabel->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");
            layout->addWidget(groupLabel, ++row, 1, 1, 4, Qt::AlignHCenter);

            QDir chnnels(QString(ADC_PATH + groups.fileName()));
            QFileInfoList chnnelList = chnnels.entryInfoList();
            foreach (QFileInfo chnnel, chnnelList) {
                if(chnnel.fileName().endsWith("raw")){
                    QLabel *chnnelLabel = common.getLabel(QString("通道"+QString::number(j++)+":"));
                    QLineEdit *chnnelEdit = common.getLineEdit();
                    chnnelEdit->setObjectName(groups.fileName()+chnnel.fileName());
                    chnnelEdit->setDisabled(true);
                    pLayout->addRow(chnnelLabel, chnnelEdit);
                }
            }
        }

        layout->addLayout(pLayout, ++row, 1, 1, 4);
        thread = new AdcThread;
        qRegisterMetaType<QVector<QString>>("QVector<QString>&");
        connect(thread, &AdcThread::getAdcValue, this, &AdcKey::setAdcValue);
        thread->start();
    }

    layout->addWidget(passBtn, ++row, 1, 1, 2, Qt::AlignBottom);
    layout->addWidget(failBtn, row, 3, 1, 2, Qt::AlignBottom);
    this->setLayout(layout);
    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
}

void AdcKey::setAdcValue(QString value, QString chnnel)
{
    QLineEdit *lineEdit = this->findChild<QLineEdit *>(chnnel);
    if(lineEdit){
        lineEdit->setText(value);
    }
}

void AdcKey::handleClick(bool result)
{
    if(thread){
        thread->readFlag = false;
        thread->quit();
    }
    testResult = result;

    //update config file state
    common.setConfig("ADC", "state", QString::number(result));

    emit testFinish();
    this->close();
}

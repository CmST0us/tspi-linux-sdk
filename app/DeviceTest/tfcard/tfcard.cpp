#include "tfcard.h"

TFCard::TFCard(QWidget *parent) : QWidget(parent)
{

}

void TFCard::initWindow(int w, int h)
{
    tips = common.getButton("正在自动测试TFCard，请稍候。。。");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QPushButton *closeBtn = common.getButton("关闭窗口");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {this->closeWindow();});

    QGridLayout *layout = new QGridLayout();

    layout->addWidget(tips, 1, 1, 1, 6, Qt::AlignBottom);
    layout->addWidget(closeBtn, 2, 3, 1, 2, Qt::AlignBottom);
    this->setLayout(layout);

    tfThread = new TFCardThread();
    connect(tfThread, &TFCardThread::getTFCardState, this, &TFCard::setTFCardState);

    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);

    closeTimer = new QTimer(this);
    connect(closeTimer, &QTimer::timeout, this, [=] {this->closeWindow();});

    tfThread->start();

}

void TFCard::setTFCardState(bool state)
{
    testResult = state;
    if(state)
        tips->setText("TF卡测试成功!!!");
    else
        tips->setText("TF卡测试失败!!!");

    if(autoTest){
        tfThread->checkFlag = false;
        tfThread->quit();
        closeTimer->start(waitCloseSecond);
    }
}

void TFCard::closeWindow()
{
    qDebug()<<"close window!";
    closeTimer->stop();
    tfThread->checkFlag = false;
    tfThread->quit();

    //update config file state
    common.setConfig("TFcard", "state", QString::number(testResult));

    this->close();
    emit testFinish();
}


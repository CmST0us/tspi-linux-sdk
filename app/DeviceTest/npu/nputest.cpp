#include "nputest.h".h"

NPUTest::NPUTest(QWidget *parent) : QWidget(parent)
{

}

void NPUTest::initWindow(int w, int h)
{
    tips = common.getButton("正在自动测试RK1808M0计算卡，请稍候。。。");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    detial = common.getTextEdit();

    QPushButton *closeBtn = common.getButton("关闭窗口");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {closeWindow();});

    QGridLayout *layout = new QGridLayout();

    layout->addWidget(tips, 1, 1, 1, 6);
     layout->addWidget(detial, 2, 1, 1, 6);
    layout->addWidget(closeBtn, 3, 3, 1, 2, Qt::AlignBottom);
    this->setLayout(layout);

    npuThread = new NPUThread();
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(npuThread, &NPUThread::checkConnect, this, &NPUTest::handleCheckState);
    connect(npuThread, &NPUThread::runDemoState, this, &NPUTest::handleRunDemoState);

    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=] {closeWindow();});
    npuThread->start();
}

void NPUTest::handleCheckState(bool result)
{
    if(result){
        tips->setText(QString("RK1808M0通信成功!"));
    }else{
        tips->setText(QString("RK1808M0通信失败!"));
    }
}

void NPUTest::handleRunDemoState(bool result, QString detialTxt)
{
    testResult = result;

    if(testResult)
        detial->append("NPU Demo 运行成功!\n");
    else
        detial->append("NPU Demo 运行失败!\n");

    detial->append(detialTxt);

    if(autoTest){
        timer->start(waitCloseSecond);
    }
}

void NPUTest::closeWindow()
{
    timer->stop();
    //update config file state
    common.setConfig("NPU", "state", QString::number(testResult));
    npuThread->quit();
    emit testFinish();
    this->close();
}

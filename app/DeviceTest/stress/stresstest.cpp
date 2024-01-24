#include "stresstest.h"

#define DDR_TEST_CMD "memtester 3G > /tmp/memtester.log 2>&1 &"
#define CPU_TEST_CMD "stress -c 6 &"


static int recordTime = 0;
static int minute = 0;
static QString timeStr;

StressTest::StressTest(QWidget *parent) : QWidget(parent)
{
     recordTime = 0;
     minute = 0;
}

void StressTest::initWindow(int w, int h)
{
    QFont font;
    font.setPointSize(30);
    cutDownTime = common.getLabel("00:00");
    cutDownTime->setFont(font);
    cpuStress = common.getLabel("CPU压力测试");
    cpuLoadLable = common.getLabel("CPU负载:");
    cpuLoad = common.getLabel("");
    cpuTempLabel = common.getLabel("CPU温度:");
    cpuTemp = common.getLabel("");
    cpuFreqLabel = common.getLabel("CPU频率:");
    cpuFreq = common.getLabel("");

    ddrStress = common.getLabel("DDR压力测试");
    ddrLoadLable = common.getLabel("DDR负载:");
    ddrLoad = common.getLabel("");
    ddrFreqLabel = common.getLabel("DDR频率:");
    ddrFreq = common.getLabel("");
    QPushButton *closeBtn = common.getButton("关闭窗口");

    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");
    connect(closeBtn, &QPushButton::clicked, [=] {this->closeWindow();});

    QGridLayout *layout = new QGridLayout();

    layout->addWidget(cutDownTime, 1, 1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(cpuStress, 2, 1, 1, 2);
    layout->addWidget(cpuLoadLable, 3, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(cpuLoad, 3, 2, 1, 1, Qt::AlignLeft);
    layout->addWidget(cpuTempLabel, 4, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(cpuTemp, 4, 2, 1, 1, Qt::AlignLeft);
    layout->addWidget(cpuFreqLabel, 5, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(cpuFreq, 5, 2, 1, 1, Qt::AlignLeft);

    layout->addWidget(ddrStress, 6, 1, 1, 2);
    layout->addWidget(ddrLoadLable, 7, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(ddrLoad, 7, 2, 1, 1, Qt::AlignLeft);
    layout->addWidget(ddrFreqLabel, 8, 1, 1, 1, Qt::AlignRight);
    layout->addWidget(ddrFreq, 8, 2, 1, 1, Qt::AlignLeft);
    layout->addWidget(closeBtn, 9, 1, 1, 2, Qt::AlignHCenter);

    this->setLayout(layout);
    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

    thread = new StressThread;
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(thread, &StressThread::getCpuInfo, this, &StressTest::handleCpuInfo);
    connect(thread, &StressThread::getDdrInfo, this, &StressTest::handleDdrInfo);
    thread->start();

    ::system(QString(CPU_TEST_CMD).toLocal8Bit());
    ::system(QString(DDR_TEST_CMD).toLocal8Bit());

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=] {this->getDDRTestInfo();});
    timer->start(1000);
}

void StressTest::handleCpuInfo(QString cpuload, QString cpufreq, QString cputemp)
{
    this->cpuLoad->setText(cpuload);
    this->cpuFreq->setText(cpufreq);
    this->cpuTemp->setText(cputemp);
}

void StressTest::handleDdrInfo(QString ddrload, QString ddrfreq)
{
    this->ddrFreq->setText(ddrfreq);
    this->ddrLoad->setText(ddrload);
}

void StressTest::updateTime()
{
    if(recordTime < 10)
        timeStr = "0"+QString::number(minute)+":0" + QString::number(recordTime);
    else if(10 <= recordTime && recordTime <= 59)
        timeStr = "0"+QString::number(minute) +":"+ QString::number(recordTime);
    else{
        recordTime = 0;
        minute++;
        timeStr = "0"+QString::number(minute)+":0" + QString::number(recordTime);
    }

    cutDownTime->setText(timeStr);
    recordTime++;
}

void StressTest::getDDRTestInfo()
{
    QFile logFile ("/tmp/memtester.log");
    if(logFile.exists()){
        logFile.open(QIODevice::ReadOnly);
        QString ddrTestTxt = logFile.readAll();
        if ( ddrTestTxt.indexOf("FAILURE") >= 0 ){
            qDebug() <<ddrTestTxt;
            this->showDialog(ddrTestTxt);
        }
        logFile.close();
    }

    this->updateTime();
}

void StressTest::showDialog(QString ddrTestTxt)
{
    if (!showDilogFlag){
        timer->stop();
        QTextEdit *txtEdit;
        QDialog *dialog = new QDialog(this);
        dialog->setStyleSheet("QDialog{border:1px solid black}");
        dialog->setWindowModality(Qt::WindowModal);
        dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

        QVBoxLayout *layout = new QVBoxLayout;
        QLabel *label = common.getLabel("DDR压力测试异常!!!");
        txtEdit = common.getTextEdit();
        txtEdit->setText(ddrTestTxt);
        QPushButton *closeBtn = common.getButton("关闭");
        connect(closeBtn, &QPushButton::clicked, [=] {
            dialog->close();
            showDilogFlag=false;
            timer->start(1000);
        });
        layout->addWidget(label);
        layout->addWidget(txtEdit);
        layout->addWidget(closeBtn);
        dialog->setLayout(layout);
        showDilogFlag = true;
        dialog->show();
        qDebug()<<"showed";
    }else{
//        txtEdit->setText(ddrTestTxt);
    }
}

void StressTest::closeWindow()
{
    ::system("killall stress");
    ::system("killall memtester");
    timer->stop();
    thread->testStart = false;
    thread->quit();
    this->close();
}


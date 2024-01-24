#include "pwm.h"

Pwm::Pwm(QWidget *parent) : QWidget(parent)
{
}

void Pwm::initWindow(int w, int h)
{
    QGridLayout *mainLayout = new QGridLayout();
    pwmThread = new PwmThread;
    QDir dir(PWM_DEV_PATH);
    dir.setFilter(QDir::Dirs);

    QPushButton *tips = common.getButton("自动改变背光PWM的占空比，请查看屏幕背光是否有亮暗变化！");
    tips->setStyleSheet(tips->styleSheet()+"QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *faildBtn = common.getButton("测试失败");
    faildBtn->setStyleSheet(faildBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(faildBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    int i = 1;
    for(QFileInfo info : dir.entryInfoList()){
        if(info.fileName() == "." || info.fileName() == "..")
            continue;

        if(initPwm(info.filePath())){
            i++;
            QLabel *pwmLabel = common.getLabel(info.fileName());
            QSlider *pwmSlider = new QSlider();
            pwmSlider->setOrientation(Qt::Horizontal);
            pwmSlider->setMaximum(pwmThread->period);
            pwmSlider->setMinimum(0);
            pwmSlider->setSingleStep(10);
            pwmSlider->setValue(pwmThread->duty_cycle);
            pwmSlider->setObjectName(info.fileName());

            QLabel *polarLabel = common.getLabel("极性:");
            QButtonGroup *polarGroup = new QButtonGroup(this);
            QRadioButton *normal = common.getRadioButton("normal");
            normal->setChecked(true);
            QRadioButton *inversed = common.getRadioButton("inversed");
            polarGroup->addButton(normal);
            polarGroup->addButton(inversed);

            connect(pwmSlider, &QSlider::sliderMoved, [=] { this->manualSetPwm(info.filePath(), pwmSlider->value()); });
            connect(normal, &QRadioButton::clicked, [=] {this->setPolar(info.filePath(), "normal");});
            connect(inversed, &QRadioButton::clicked, [=] {this->setPolar(info.filePath(), "inversed");});

            mainLayout->addWidget(pwmLabel,i, 1, 1, 1, Qt::AlignLeft);
            mainLayout->addWidget(pwmSlider,i, 2, 1, 6,Qt::AlignVCenter);
            mainLayout->addWidget(polarLabel,i, 8, 1, 1,Qt::AlignVCenter);
            mainLayout->addWidget(normal,i, 9, 1, 1,Qt::AlignVCenter);
            mainLayout->addWidget(inversed,i, 10, 1, 1,Qt::AlignVCenter);
        }
    }

    //backlight
    QSlider *backLight = new QSlider();
    backLight->setOrientation(Qt::Horizontal);
    backLight->setMaximum(255);
    backLight->setMinimum(1);
    backLight->setValue(200);
    backLight->setObjectName("backLight");

    connect(backLight, &QSlider::sliderMoved, [=] { this->manualSetBackLight(backLight->value()); });
    QLabel *backLightLabel = common.getLabel("背光:");

    mainLayout->addWidget(tips, 1, 1, 1, 10,Qt::AlignVCenter);
    mainLayout->addWidget(backLightLabel, i+1, 1,Qt::AlignVCenter);
    mainLayout->addWidget(backLight, i+1, 2, 1, 9,Qt::AlignVCenter);
    mainLayout->addWidget(passBtn, i+2, 1, 1, 5, Qt::AlignBottom);
    mainLayout->addWidget(faildBtn, i+2, 7, 1, 4, Qt::AlignBottom);

    this->setLayout(mainLayout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);

    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(pwmThread, &PwmThread::setPwmValue, this, &Pwm::setSliderValue);
    connect(pwmThread, &PwmThread::setBackLightValue, this, &Pwm::setBackSliderValue);
    pwmThread->start();
}

int Pwm::initPwm(QString pwmPath)
{
    QFile file(pwmPath + "/pwm0");
    if(!file.exists()){
        /*
        cd /sys/class/pwm/pwmchip8/
        echo 0 > export
        cd pwm0
        echo 10000 > period
        echo 5000 > duty_cycle
        echo normal > polarity
        echo 1 > enable

        enable：写入 1 使能 pwm，写入 0 关闭 pwm；
        polarity：有 normal 或 inversed 两个参数选择，表示输出引脚电平翻转；
        duty_cycle：在 normal 模式下，表示一个周期内高电平持续的时间（单位：纳秒），在 reversed 模
        式下，表示一个周期中低电平持续的时间（单位：纳秒)；
        period：表示 pwm 波的周期(单位：纳秒)；*/
#if 0
        //1.echo 0 > export
        QFile exportFile(QString(pwmPath +"/export"));
        exportFile.open(QIODevice::ReadWrite);
        exportFile.write("0");
        exportFile.close();

        //2.echo 10000 > pwm0/period
        QFile periodtFile(QString(pwmPath +"/pwm0/period"));
        periodtFile.open(QIODevice::ReadWrite);
        periodtFile.write(QString::number(period).toLatin1());
        periodtFile.close();

        //3.echo 5000 > pwm0/duty_cycle
        QFile dutyFile(QString(pwmPath +"/pwm0/duty_cycle"));
        dutyFile.open(QIODevice::ReadWrite);
        dutyFile.write(QString::number(duty_cycle).toLatin1());
        dutyFile.close();

        //4.echo normal > pwm0/polarity
        QFile polarityFile(QString(pwmPath +"/pwm0/polarity"));
        polarityFile.open(QIODevice::ReadWrite);
        polarityFile.write(polarity.toLatin1());
        polarityFile.close();

        //5.echo 1 > pwm0/enable
        QFile stateFile(QString(pwmPath +"/pwm0/enable"));
        stateFile.open(QIODevice::ReadWrite);
        stateFile.write("1");
        stateFile.close();
#else
        ::system(QString("echo 0 > %1/export").
                 arg(pwmPath).toLocal8Bit());

        ::system(QString("echo %1 > %2/pwm0/period").
                 arg(QString::number(pwmThread->period)).
                 arg(pwmPath).toLocal8Bit());

        ::system(QString("echo %1 > %2/pwm0/duty_cycle").
                 arg(QString::number(pwmThread->duty_cycle)).
                 arg(pwmPath).toLocal8Bit());

        ::system(QString("echo %1 > %2/pwm0/polarity").
                 arg(pwmThread->polarity).
                 arg(pwmPath).toLocal8Bit());

        ::system(QString("echo %1 > %2/pwm0/enable").
                 arg(QString::number(1)).
                 arg(pwmPath).toLocal8Bit());
#endif
    }

    //pwm0 exits mean already export
    if(file.exists()){
        return 1;
    }
    return 0;
}

void Pwm::setPolar(QString pwmPath, QString polar)
{
    QFile polarityFile(QString(pwmPath +"/pwm0/polarity"));
    polarityFile.open(QIODevice::ReadWrite);
    polarityFile.write(polar.toLatin1());
    polarityFile.close();
}

void Pwm::setSliderValue(QString pwmChip, int value)
{
    QSlider *pwmSlider = this->findChild<QSlider *>(pwmChip);
    pwmSlider->setValue(value);
}

void Pwm::setBackSliderValue(int value)
{
    QSlider *backLightSlider = this->findChild<QSlider *>("backLight");
    backLightSlider->setValue(value);
}

void Pwm::manualSetBackLight(int value)
{
    //manual set value
    pwmThread->autoChangeValue = false;

    ::system(QString(" echo %1 > /sys/class/backlight/backlight/brightness")
              .arg(QString::number(value)).toLocal8Bit());
}

void Pwm::manualSetPwm(QString pwmChip, int value)
{
    //manual set value
    pwmThread->autoChangeValue = false;
    ::system(QString(" echo %1 > "+pwmChip + "/pwm0/duty_cycle")
         .arg(QString::number(value)).toLocal8Bit());
}

void Pwm::handleClick(bool result)
{
    pwmThread->autoChangeValue = false;
    pwmThread->quit();
    testResult = result;

    //update config file state
    common.setConfig("pwm", "state", QString::number(result));

    emit testFinish();
    this->close();
}


#include "sound.h"
#include "QAudioFormat"
#include "QDebug"
#include "QFile"
#include "QAudioDeviceInfo"
#include "QAudioInput"
#include "QAudioOutput"
#include "QMediaPlayer"
#include <QTimer>

#define SAMPLE_RATE 44100
#define SINGLE_CHANNEL 1
#define DUAL_CHANNEL 2
#define SAMPLE_SIZE 16
#define AUDIO_CODEC "audio/pcm"

#define AUDIO_PATH "/rp_test/123.wav"
#define RECORD_PATH "/tmp/record.wav"

static QString filterName;

static int dateRate = (SAMPLE_RATE * DUAL_CHANNEL * SAMPLE_SIZE / 8);

static QFile outputFile;
static QFile inputFile;

static bool reFormatInputDev;
static bool reFormatOutputDev;

Sound::Sound(QWidget *parent) : QWidget(parent)
{
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(DUAL_CHANNEL);
    format.setSampleSize(SAMPLE_SIZE);
    format.setCodec(AUDIO_CODEC);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    reFormatInputDev = false;
    reFormatOutputDev = false;
    filterName = "default_speexrate_oss_pulse_upmix_vdownmix"; //maybe  alsa-plugin, not need
}

void Sound::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout;

    QList<QAudioDeviceInfo> availableOutputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QList<QAudioDeviceInfo> availableInputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    int row = 0;
    QLabel *inputLabel = common.getLabel("支持录音的声卡:");
    layout->addWidget(inputLabel, ++row, 1, 1, 4, Qt::AlignBottom);

    for (QAudioDeviceInfo device:availableInputDevices){
        QString devName = device.deviceName();

        if(devName.startsWith("default")
                || devName.indexOf("Loopback") >=0
                || filterName.indexOf(devName) >= 0 //filter alsa plugin
                || devName.indexOf("alsa_") >= 0)   //filter alsa plugin
            continue;

        QLabel *cardLabel = common.getLabel("声卡:");
        QLineEdit *card = common.getLineEdit();
        card->setText(device.deviceName());
        QPushButton *recordBtn = common.getButton("录音");
        recordBtn->setObjectName(device.deviceName()+"_record");
        connect(recordBtn, &QPushButton::clicked, this, [=] {this->recordAudio(device, recordBtn->text());});
        row ++;
        layout->addWidget(cardLabel, row, 1, 1, 1, Qt::AlignVCenter);
        layout->addWidget(card, row, 2, 1, 2, Qt::AlignVCenter);
        layout->addWidget(recordBtn, row, 4, 1, 1, Qt::AlignVCenter);
    }
    recordTime = common.getLabel("00:00");
    recordTime->setAlignment(Qt::AlignCenter);
    QFont font;
    font.setPointSize(30);
    recordTime->setFont(font);

    layout->addWidget(recordTime, ++row, 1, 1, 4, Qt::AlignTop);

    QLabel *outputLabel = common.getLabel("支持播放的声卡:");
    layout->addWidget(outputLabel, ++row, 1, 1, 4, Qt::AlignBottom);
    for (QAudioDeviceInfo device:availableOutputDevices){
        QString devName = device.deviceName();

        if(devName.startsWith("default")
                || devName.indexOf("Loopback") >=0
                || filterName.indexOf(devName) >= 0 //filter alsa plugin
                || devName.indexOf("alsa_") >= 0)   //filter alsa plugin
            continue;

        QLabel *cardLabel = common.getLabel("声卡:");
        QLineEdit *card = common.getLineEdit();
        card->setText(device.deviceName());
        QPushButton *playBtn = common.getButton("播放");
        playBtn->setObjectName(device.deviceName());
        connect(playBtn, &QPushButton::clicked, this, [=] {this->playAudio(device, playBtn->text());});

        row ++;
        layout->addWidget(cardLabel, row, 1, 1, 1, Qt::AlignVCenter);
        layout->addWidget(card, row, 2, 1, 2, Qt::AlignVCenter);
        layout->addWidget(playBtn, row, 4, 1, 1, Qt::AlignVCenter);
    }

    playTime = common.getLabel("00:00");
    playTime->setAlignment(Qt::AlignCenter);
    playTime->setFont(font);
    layout->addWidget(playTime, ++row, 1, 1, 4, Qt::AlignTop);

    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *failBtn = common.getButton("测试失败");
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(passBtn, &QPushButton::clicked, [=] { this->handleClick(true); });
    connect(failBtn, &QPushButton::clicked, [=] { this->handleClick(false); });

    layout->addWidget(passBtn, ++row, 1, 1, 2);
    layout->addWidget(failBtn, row, 3, 1, 2);


    recordTimeThread = new RecordTimeThread;
    playTimeThread = new PlayTimeThread;
    qRegisterMetaType<QVector<QString>>("QVector<QString>&");
    connect(recordTimeThread, &RecordTimeThread::updateRecordTime, this, &Sound::setRecordTime);
    connect(playTimeThread, &PlayTimeThread::updatePlayTime, this, &Sound::setPlayTime);

    this->setLayout(layout);
    this->resize(w, h);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
}

void Sound::playAudio(QAudioDeviceInfo device, QString btnTxt){
    QPushButton *btn = this->findChild<QPushButton *>(device.deviceName());
    if(btnTxt == "播放"){
        btn->setText("停止");
        if (device.isFormatSupported(format)){
            outputFile.setFileName(RECORD_PATH);
            if(!outputFile.exists() || outputFile.size() == 0)
                outputFile.setFileName(AUDIO_PATH);

            if(outputFile.open( QIODevice::ReadOnly)){
                outputDev = new QAudioOutput(device, format, this);
//                outputDev->setVolume(0.1f);
                int duration = (outputFile.size()-44) / dateRate;//get wav file time
                if(outputDev){
                    playTimeThread->playTime = duration;
                    playTimeThread->playStart = true;
                    outputDev->start(&outputFile);
                    playTimeThread->start();
                    connect(outputDev, &QAudioOutput::stateChanged, [=] {
                        this->handleOutDevState(outputDev->state(), device);
                    });
                }
            }else{
                qDebug()<<"File not exits!";
                btn->setText("播放");
            }
        }else{
            btn->setText("播放");
            if(!reFormatOutputDev){
                format.setChannelCount(SINGLE_CHANNEL);
                dateRate = (SAMPLE_RATE * SINGLE_CHANNEL * SAMPLE_SIZE / 8);
                reFormatOutputDev = true;
                emit btn->clicked();  //call playAudio();
            }else{
                showDialog();
            }
        }
    }else{
        btn->setText("播放");
        closeOutDev();
    }
}

void Sound::recordAudio(QAudioDeviceInfo device, QString btnTxt)
{
    QPushButton *btn = this->findChild<QPushButton *>(device.deviceName()+"_record");
    if(btnTxt == "录音"){
        btn->setText("停止");
        if (device.isFormatSupported(format)){
            inputFile.setFileName(RECORD_PATH);
            if(inputFile.open( QIODevice::WriteOnly | QIODevice::Truncate )){
                inputDev = new QAudioInput(device, format, this);
                if(inputDev){
                    connect(inputDev, &QAudioInput::stateChanged, [=] {
                        this->handleInDevState(inputDev->state());
                    });
                    inputDev->reset();
                    inputDev->start(&inputFile);
                    recordTimeThread->recordStart = true;
                    recordTimeThread->start();
                }
            }
        }else{
            btn->setText("录音");
            if(!reFormatInputDev){
                format.setChannelCount(SINGLE_CHANNEL);
                dateRate = (SAMPLE_RATE * SINGLE_CHANNEL * SAMPLE_SIZE / 8);
                reFormatInputDev = true;
                emit btn->clicked(); //call recordAudio();
            }else{
                showDialog();
            }
        }
    }else{
        btn->setText("录音");
        closeInDev();
    }
}

void Sound::handleOutDevState(QAudio::State state, QAudioDeviceInfo device)
{
    if(state == QAudio::StoppedState || state == QAudio::IdleState){
        playTimeThread->playStart = false;
        playTimeThread->playTime = 0;
        playTimeThread->quit();
        QPushButton *btn = this->findChild<QPushButton *>(device.deviceName());
        btn->setText("播放");
        playTime->setText("00:00");
        closeOutDev();
    }
}


void Sound::handleInDevState(QAudio::State state)
{
    if(state == QAudio::StoppedState){
        recordTimeThread->recordStart = false;
        recordTimeThread->quit();
    }
}

void Sound::setRecordTime(QString timeStr)
{
    recordTime->setText(timeStr);
}

void Sound::setPlayTime(QString timeStr)
{
    playTime->setText(timeStr);
}

//format is not support
void Sound::showDialog()
{
    QDialog *dialog = new QDialog(this);
    dialog->setStyleSheet("QDialog{border:1px solid black}");
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    QGridLayout *layout = new QGridLayout;
    QPushButton *closeBtn = common.getButton("关闭");
    closeBtn->setStyleSheet(closeBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    connect(closeBtn, &QPushButton::clicked, this, [=] {dialog->close();});

    QLabel *label = common.getLabel("声卡 format 设置失败!!");

    QLabel *sampleLabe = common.getLabel("采样率:");
    QLineEdit *sample = common.getLineEdit();
    sample->setText(QString::number(format.sampleRate()));

    QLabel *chnnleLabe = common.getLabel("通道:");
    QLineEdit *chnnle = common.getLineEdit();
    chnnle->setText(QString::number(format.channelCount()));

    QLabel *sampleSizeLabe = common.getLabel("采样大小:");
    QLineEdit *sampleSize = common.getLineEdit();
    sampleSize->setText(QString::number(format.sampleSize()));

    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, 1, 1, 1, 3);
    layout->addWidget(sampleLabe, 2, 1, 1, 1);
    layout->addWidget(sample, 2, 2, 1, 2);
    layout->addWidget(chnnleLabe, 3, 1, 1, 1);
    layout->addWidget(chnnle, 3, 2, 1, 2);
    layout->addWidget(sampleSizeLabe, 4, 1, 1, 1);
    layout->addWidget(sampleSize, 4, 2, 1, 2);
    layout->addWidget(closeBtn, 5, 2, 1, 1);
    dialog->setLayout(layout);
    dialog->show();
}

void Sound::closeOutDev()
{
    if(outputDev != nullptr){
        // qDebug()<<"outputDev != nullptr";
        disconnect(outputDev);
        outputDev->stop();
        outputDev->deleteLater();
        outputDev=nullptr;
        outputFile.close();
    }
}

void Sound::closeInDev()
{
    if(inputDev != nullptr){
        // qDebug()<<"inputDev != nullptr";
        disconnect(inputDev);
        inputDev->stop();
        inputDev->deleteLater();
        inputDev=nullptr;
        inputFile.close();
    }
}

void Sound::handleClick(bool result)
{
    recordTimeThread->recordStart = false;
    playTimeThread->playStart = false;
    recordTimeThread->quit();
    playTimeThread->quit();

    closeInDev();
    closeOutDev();

    //update config file state
    common.setConfig("soundCard", "state", QString::number(result));

    testResult = result;
    emit testFinish();
    this->close();
}


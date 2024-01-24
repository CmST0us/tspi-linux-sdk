#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QWidget>
#include <QAudioDeviceInfo>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudio>
#include <QSoundEffect>
#include <QDialog>
#include "common/common.h"
#include "recortTimeThread.h"
#include "playtimethread.h"

class Sound : public QWidget
{
    Q_OBJECT
public:
    explicit Sound(QWidget *parent = nullptr);

    void initWindow(int w, int h);
    bool testResult = false;

protected:
    Common common;
    QAudioFormat format;
    QAudioOutput *outputDev=nullptr;
    QAudioInput *inputDev=nullptr;
    QLabel *recordTime;
    QLabel *playTime;
    RecordTimeThread *recordTimeThread;
    PlayTimeThread *playTimeThread;
    int second = 0;

    void playAudio(QAudioDeviceInfo device, QString btnTxt);
    void recordAudio(QAudioDeviceInfo device, QString btnTxt);
    void handleClick(bool result);
    void closeOutDev();
    void closeInDev();
    void showDialog();

protected slots:
    void handleOutDevState(QAudio::State state, QAudioDeviceInfo device);
    void handleInDevState(QAudio::State state);
    void setRecordTime(QString timeStr);
    void setPlayTime(QString timeStr);

signals:
    void testFinish();
};

#endif // SOUND_H

#ifndef PLAYTIMETHREAD_H
#define PLAYTIMETHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>

class PlayTimeThread : public QThread
{
    Q_OBJECT
public:
    PlayTimeThread();

    void run() override;

    int playTime = 0;

    bool playStart = false;

protected:
    void setPlayTime();

signals:
    void updatePlayTime(QString timeStr);
};

#endif // PLAYTIMETHREAD_H

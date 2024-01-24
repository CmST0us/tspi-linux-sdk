#ifndef TFCARDTHREAD_H
#define TFCARDTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QDir>

class TFCardThread : public QThread
{
    Q_OBJECT
public:
    TFCardThread();

    void run() override;

    bool result;

    bool checkFlag=true;

protected:
    void checkTFCard();

signals:
    void getTFCardState(bool state);
};

#endif // TFCARDTHREAD_H

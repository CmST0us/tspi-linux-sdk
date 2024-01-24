#ifndef TIMETHREAD_H
#define TIMETHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>

class RecordTimeThread : public QThread
{
    Q_OBJECT
public:
    RecordTimeThread();

    void run() override;

    bool recordStart = false;

protected:
    void setRecordTime();


signals:
    void updateRecordTime(QString timeStr);
};

#endif // TIMETHREAD_H

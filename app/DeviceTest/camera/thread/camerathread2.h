#ifndef CAMERTHREAD2_H
#define CAMERTHREAD2_H

#include <QObject>
#include <QThread>
#include <QDebug>

class CameraThread2 : public QThread
{
    Q_OBJECT
public:
    CameraThread2();
    void run() override;

    void getCam2Buf();

    QString camera2Dev="";

    bool previewCam2 = true;

signals:
    void showCamera2(unsigned char *buffer);
};

#endif // CAMERTHREAD2_H

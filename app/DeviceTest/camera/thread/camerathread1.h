#ifndef CAMERATHREAD1_H
#define CAMERATHREAD1_H

#include <QObject>
#include <QThread>
#include <QDebug>

class CameraThread1 : public QThread
{
    Q_OBJECT
public:
    CameraThread1();

    void run() override;

    void getCam1Buf();

    bool previewCam1 = true;

    QString camera1Dev="";

signals:
    void showCamera1(unsigned char *buffer);
};

#endif // CAMERATHREAD1_H

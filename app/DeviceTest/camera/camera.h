#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QWidget>
#include "common/common.h"
#include "thread/camerathread1.h"
#include "thread/camerathread2.h"

class Camera : public QWidget
{
    Q_OBJECT
public:
    explicit Camera(QWidget *parent = nullptr);
    void initWindow(int w, int h);

    void display();

    void checkIspServer();

    CameraThread1 *cameraThread1;
    CameraThread2 *cameraThread2;
    QLabel *camera1;
    QLabel *camera2;
    QSize picSize;
    Common common;
    bool testResult;

public slots:
    void displayCam1Buf(unsigned char *buffer);
    void displayCam2Buf(unsigned char *buffer);
    void handleClick(bool result);

signals:
    void testFinish();

};

#endif // CAMERA_H

#include "camera.h"
#include <QLayout>

Camera::Camera(QWidget *parent) : QWidget(parent)
{

}

void Camera::initWindow(int w, int h)
{
    QGridLayout *layout = new QGridLayout();
    camera1 = new QLabel();
    camera2 = new QLabel();
    QLabel *camer1Label = common.getLabel("Camera1");
    QLabel *camer2Label = common.getLabel("Camera2");
    QPushButton *title = common.getButton("摄像头预览测试");
    title->setStyleSheet(title->styleSheet()+
                         "QPushButton{background:none;color:black}QPushButton:pressed{background:none;}");
    QPushButton *passBtn = common.getButton("测试通过");
    QPushButton *failBtn = common.getButton("测试失败");
    connect(passBtn, &QPushButton::clicked, [=] {this->handleClick(true);});
    connect(failBtn, &QPushButton::clicked, [=] {this->handleClick(false);});
    failBtn->setStyleSheet(failBtn->styleSheet()+"QPushButton{background-color:#f56c6c}");

    if(h > w){
        layout->addWidget(title, 1, 1, 1, 2);
        layout->addWidget(camer1Label, 2, 1, 1, 2);
        layout->addWidget(camera1, 3, 1, 1, 2);
        layout->addWidget(camer2Label, 4, 1, 1, 2);
        layout->addWidget(camera2, 5, 1, 1, 2);
        layout->addWidget(passBtn, 6, 1, 1, 1);
        layout->addWidget(failBtn, 6, 2, 1, 1);

    }else{
        layout->addWidget(title, 1, 1, 1, 2);
        layout->addWidget(camer1Label, 2, 1, 1, 1);
        layout->addWidget(camer2Label, 2, 2 , 1, 1);
        layout->addWidget(camera1, 3, 1, 1, 1);
        layout->addWidget(passBtn, 4, 1, 1, 1);
        layout->addWidget(failBtn, 4, 2, 1, 1);
    }
    this->setLayout(layout);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->resize(w, h);
    picSize.setHeight(w);
    picSize.setHeight(h);

    cameraThread1 = new CameraThread1;
    cameraThread2 = new CameraThread2;
    connect(cameraThread1, &CameraThread1::showCamera1, this, &Camera::displayCam1Buf);
    connect(cameraThread2, &CameraThread2::showCamera2, this, &Camera::displayCam2Buf);

    checkIspServer();

    cameraThread1->start();
    cameraThread2->start();
}

void Camera::displayCam1Buf(unsigned char *buffer)
{
    QImage img;
    QPixmap pixmap;
    img = QImage(buffer, 640, 480, QImage::Format_RGB888);
    QPixmap scaledPixmap = pixmap.fromImage(img);

    camera1->setPixmap(scaledPixmap);
}

void Camera::displayCam2Buf(unsigned char *buffer)
{
    QImage img;
    QPixmap pixmap;
    img = QImage(buffer, 640, 480, QImage::Format_RGB888);
    QPixmap scaledPixmap = pixmap.fromImage(img);

    camera2->setPixmap(scaledPixmap);
}

void Camera::checkIspServer()
{
    QString compatible = common.execLinuxCmd("cat /proc/device-tree/compatible");

    //for rv1126/rv1109, camera test should run ispserver first
    if(compatible.indexOf("rv1109") >= 0 || compatible.indexOf("rv1126") >= 0){

        QString str = common.execLinuxCmd("ps -ef");
        if(str.indexOf("ispserver") >= 0 ){
            qDebug()<<"fund ispserver process!";
        }else{
            qDebug()<<"ispserver not fund!.exec it";
            ::system("ispserver &");
        }

        QFile mediaFile("/dev/media3");
        //double gc2093
        if(mediaFile.exists()){
            cameraThread1->camera1Dev = "/dev/video32";
            cameraThread2->camera2Dev = "/dev/video40";

        //imx307x1
        }else{
            cameraThread1->camera1Dev = "/dev/video15";
            cameraThread2->camera2Dev = "";
        }

    //other platform
    }else{
        cameraThread1->camera1Dev = common.getConfig("camera","camera1");
        cameraThread2->camera2Dev = common.getConfig("camera","camera2");
    }
}

void Camera::handleClick(bool result)
{
    testResult = result;
    cameraThread1->previewCam1=false;
    cameraThread2->previewCam2=false;
    cameraThread1->quit();
    cameraThread2->quit();
    //update config file state
    common.setConfig("camera", "state", QString::number(result));

    emit testFinish();
    this->close();
}


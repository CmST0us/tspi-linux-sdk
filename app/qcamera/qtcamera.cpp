/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtcamera.h"
#include <QApplication>
#include <QMediaService>
#include <QMediaRecorder>
#include <QCameraViewfinder>
#include <QCameraInfo>
#include <QMediaMetaData>

#include <QMessageBox>
#include <QPalette>
#include <QTabWidget>
#include <QtWidgets>
#include <QHBoxLayout>
#include <QVBoxLayout>

#define FONT_SIZE 12
#define QCAMERA_CAPTURE_MODE "Image Mode"
#define QCAMERA_VIDEO_MODE "Video Mode"
#define DIR_USERDATA "/userdata"
#define DIR_HOME QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
Q_DECLARE_METATYPE(QCameraInfo)

qtCamera::qtCamera()
{
    initlayout();
    QFileInfo fi(DIR_USERDATA);
    if(fi.isDir()){
        locationDir = DIR_USERDATA;
    }else {
        QFileInfo fi(DIR_HOME);
        if(fi.isDir()){
            locationDir = DIR_HOME;
        }
    }
    imageCnt = videoCnt = 0;
    setCamera(QCameraInfo::defaultCamera());
}

void qtCamera::initlayout()
{
    QBoxLayout *vLayout = new QVBoxLayout();
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    QFont font;
    font.setPixelSize(FONT_SIZE);
    resize(availableGeometry.width(), availableGeometry.height());

    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : availableCameras) {
        qDebug() << cameraInfo.description();
        QPushButton *camera = getButton();
        camera->setText(cameraInfo.description());
        camera->setFont(font);
        camera->setCheckable(true);
        if (cameraInfo == QCameraInfo::defaultCamera()){
            camera->setDefault(true);
        }else {
            camera->setDefault(false);
        }
        connect(camera, SIGNAL(clicked(bool)), this, SLOT(on_cameraSwitch()));
        vLayout->addWidget(camera);
    }

    modeButton = getButton();
    modeButton->setText(cameraMode);
    modeButton->setFont(font);
    connect(modeButton, SIGNAL(clicked(bool)), this, SLOT(updateCaptureMode()));

    captureButton = getButton();
    captureButton->setText(tr("Capture"));
    captureButton->setFont(font);
    connect(captureButton, SIGNAL(clicked(bool)), this, SLOT(on_captureClicked()));

    exitButton = getButton();
    exitButton->setText(tr("Exit"));
    exitButton->setFont(font);
    connect(exitButton, SIGNAL(clicked(bool)), this, SLOT(on_exitClicked()));

    vLayout->addWidget(modeButton);
    vLayout->addWidget(captureButton);
    vLayout->addWidget(exitButton);
    vLayout->setAlignment(Qt::AlignTop);

    viewfinder.setWindowFlag(Qt::FramelessWindowHint);
    //viewfinder.setFixedSize(availableGeometry.width() - 150, availableGeometry.height());

    QBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->addWidget(&viewfinder);
    hlayout->addLayout(vLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(hlayout);
    setCentralWidget(widget);
    setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::FramelessWindowHint);
}

void qtCamera::setCamera(const QCameraInfo &cameraInfo)
{
    m_camera.reset(new QCamera(cameraInfo));

    connect(m_camera.data(), &QCamera::stateChanged, this, &qtCamera::updateCameraState);
    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &qtCamera::displayCameraError);

    m_mediaRecorder.reset(new QMediaRecorder(m_camera.data()));
    connect(m_mediaRecorder.data(), &QMediaRecorder::stateChanged, this, &qtCamera::updateRecorderState);

    m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));

    connect(m_mediaRecorder.data(), &QMediaRecorder::durationChanged, this, &qtCamera::updateRecordTime);
    connect(m_mediaRecorder.data(), QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &qtCamera::displayRecorderError);

    m_mediaRecorder->setMetaData(QMediaMetaData::Title, QVariant(QLatin1String("Test Title")));

    configureCaptureSettings();

    m_camera->setViewfinder(&viewfinder);

    updateCameraState(m_camera->state());
    updateRecorderState(m_mediaRecorder->state());

    connect(m_imageCapture.data(), &QCameraImageCapture::imageSaved, this, &qtCamera::imageSaved);
    connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
            this, &qtCamera::displayCaptureError);

    updateCaptureMode();
}

void qtCamera::configureCaptureSettings()
{
    QSize size(640, 480);

    m_imageSettings.setCodec("jpeg");
    m_imageSettings.setQuality(QMultimedia::VeryHighQuality);
    m_imageSettings.setResolution(size);
    m_imageCapture->setEncodingSettings(m_imageSettings);

    m_audioSettings.setCodec("audio/x-adpcm");
    m_audioSettings.setChannelCount(2);
    m_audioSettings.setQuality(QMultimedia::NormalQuality);
    m_mediaRecorder->setAudioSettings(m_audioSettings);

    m_videoSettings.setCodec("video/x-h264");
    m_videoSettings.setResolution(size);
    m_videoSettings.setQuality(QMultimedia::NormalQuality);
    m_mediaRecorder->setVideoSettings(m_videoSettings);

    m_mediaRecorder->setContainerFormat("video/quicktime");

    if (0) {
        QList<QSize> supportedResolutions;
        supportedResolutions = m_imageCapture->supportedResolutions();
        for (const QSize &resolution : supportedResolutions) {
             qDebug() << "image resolution: " << resolution.width() << "x" << resolution.height();
        }

        supportedResolutions = m_mediaRecorder->supportedResolutions();
        for (const QSize &resolution : supportedResolutions) {
            qDebug() << "video resolution: " << resolution.width() << "x" << resolution.height();
        }

        const QStringList supportedAudioCodecs = m_mediaRecorder->supportedAudioCodecs();
        for (const QString &codecName : supportedAudioCodecs) {
            QString description = m_mediaRecorder->audioCodecDescription(codecName);
            qDebug() << "audio codec:" << codecName + ": " + description;
        }

        const QStringList supportedVideoCodecs = m_mediaRecorder->supportedVideoCodecs();
        for (const QString &codecName : supportedVideoCodecs) {
            QString description = m_mediaRecorder->videoCodecDescription(codecName);
            qDebug() << "video codec:" << codecName + ": " + description;
        }

        const QStringList formats = m_mediaRecorder->supportedContainers();
        for (const QString &format : formats) {
            QString description = m_mediaRecorder->containerDescription(format);
            qDebug() << "container: " << format << ": " << description;
        }
    }
}

QPushButton* qtCamera::getButton()
{
    QPushButton *button = new QPushButton;
    button->setFixedSize(144, 70);
    return button;
}

void qtCamera::updateRecordTime()
{
    quint32 dura_smal;
    quint64 duration;

    dura_smal = m_mediaRecorder->duration() % 1000;
    duration = m_mediaRecorder->duration() / 1000;
    if (dura_smal >= 500)
        duration += 1;

    QString str = QString("Recorded %1 sec").arg(duration);
    statusBar()->showMessage(str);
}

void qtCamera::record()
{
    QFileInfo fi;
    QString lo;

    lo = locationDir + "/" + "VIDEO" + QString::number(videoCnt) + ".mov";
    fi = QFileInfo(lo);

    while(fi.isFile()){
        videoCnt++;
        lo = locationDir + "/" + "VIDEO" + QString::number(videoCnt) + ".mov";
        fi = QFileInfo(lo);
    }

    m_mediaRecorder->setOutputLocation(QUrl::fromLocalFile(lo));
    m_mediaRecorder->record();
    updateRecordTime();
}

void qtCamera::stop()
{
    m_mediaRecorder->stop();
}

void qtCamera::takeImage()
{
    m_isCapturingImage = true;
    QFileInfo fi;
    QString lo;

    lo = locationDir + "/" + "PIC" + QString::number(imageCnt) + ".jpg";
    fi = QFileInfo(lo);

    while(fi.isFile()){
        imageCnt++;
        lo = locationDir + "/" + "PIC" + QString::number(imageCnt) + ".jpg";
        fi = QFileInfo(lo);
    }

    m_imageCapture->capture(lo);
}

void qtCamera::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    m_isCapturingImage = false;
}

void qtCamera::updateCaptureMode()
{
    QCamera::CaptureModes captureMode;
    QString capture;
    if (cameraMode.compare(QCAMERA_CAPTURE_MODE)){
        captureMode = QCamera::CaptureStillImage ;
    }else {
        captureMode = QCamera::CaptureVideo;
    }

    if (m_camera->isCaptureModeSupported(captureMode)){
        m_camera->unload();
        m_camera->setCaptureMode(captureMode);
        m_camera->start();
        if(captureMode == QCamera::CaptureStillImage){
            cameraMode = QString(QCAMERA_CAPTURE_MODE);
            capture = "Capture";
        }else {
            cameraMode = QString(QCAMERA_VIDEO_MODE);
            capture = "Record";
        }
        modeButton->setText(cameraMode);
        captureButton->setText(capture);
    }
}

void qtCamera::updateCameraState(QCamera::State state)
{
    switch (state) {
    case QCamera::ActiveState:
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        break;
    }
}

void qtCamera::updateRecorderState(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::StoppedState:
        captureButton->setText(tr("Record"));
        break;
    case QMediaRecorder::PausedState:
        break;
    case QMediaRecorder::RecordingState:
        captureButton->setText(tr("Recording"));
        break;
    }
}

void qtCamera::displayRecorderError()
{
    QMessageBox::warning(this, tr("Capture Error"), m_mediaRecorder->errorString());
}

void qtCamera::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera Error"), m_camera->errorString());
}

void qtCamera::imageSaved(int id, const QString &fileName)
{
    Q_UNUSED(id);
    statusBar()->showMessage(tr("Captured \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    statusBar()->show();
    m_isCapturingImage = false;
    if (m_applicationExiting)
        close();
}

void qtCamera::closeEvent(QCloseEvent *event)
{
    if (m_isCapturingImage) {
        setEnabled(false);
        m_applicationExiting = true;
        event->ignore();
    } else {
        event->accept();
    }
}

void qtCamera::on_cameraSwitch()
{
    QList<QPushButton *> buttons = centralWidget()->findChildren<QPushButton *>();
    for(auto *bt: buttons){
        if(bt->isChecked()){
            for(auto *button: buttons){
                if(button->isDefault())
                    button->setDefault(false);
            }
            bt->setDefault(true);
            bt->setChecked(false);
            qDebug() << "switch to " + bt->text();
            const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
            for (const QCameraInfo &cameraInfo : availableCameras) {
                if(! bt->text().compare(cameraInfo.description())){
                    qDebug() << cameraInfo.description();
                    setCamera(cameraInfo);
                }
            }
            break;
        }
    }
}

void qtCamera::on_captureClicked()
{
    if (m_camera->captureMode() == QCamera::CaptureStillImage) {
        if(m_imageCapture->isReadyForCapture())
            takeImage();
    } else if(m_camera->captureMode() == QCamera::CaptureVideo){
        if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
            stop();
        else
            record();
    }
}

void qtCamera::on_exitClicked()
{
        qApp->exit(0);
}

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

#include "qplayer.h"
#include <QtWidgets>
#include <QVideoSurfaceFormat>

#define MIME_IS_GIF(s)   !(s).compare("image/gif")
#define MIME_IS_IMAGE(s) ((s).startsWith("image/") && !MIME_IS_GIF(s))
#define MIME_IS_VIDEO(s) ((s).startsWith("video/") || MIME_IS_GIF(s))
#define MIME_IS_AUDIO(s) (s).startsWith("audio/")

QPlayer::QPlayer()
    : player(nullptr, QMediaPlayer::VideoSurface)
    , list(nullptr)
    , playButton(nullptr)
    , positionSlider(nullptr)
{
    exitButton = new QPushButton(tr("Exit"));
    connect(exitButton, &QAbstractButton::clicked, this, &QPlayer::exit);

    playButton = new QPushButton;
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(playButton, &QAbstractButton::clicked, this, &QPlayer::play);

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 0);
    connect(positionSlider, &QAbstractSlider::sliderMoved, this, &QPlayer::setPosition);
    connect(positionSlider, &QAbstractSlider::sliderReleased, this, &QPlayer::unMute);

    controlLayout = new QHBoxLayout;
    controlLayout->addWidget(exitButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);
    controlLayout->setMargin(0);
    control = new QWidget();
    control->setLayout(controlLayout);
    control->setWindowFlag(Qt::FramelessWindowHint);

    videoViewer = new QVideoWidget;
    imageViewer = new QLabel();
    imageViewer->setAlignment(Qt::AlignCenter);
    QBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(videoViewer);
    mainLayout->addWidget(imageViewer);
    mainLayout->addWidget(control);

    QWidget *widget = new QWidget();
    widget->setLayout(mainLayout);

    list = new QMediaPlaylist;
    player.setVideoOutput(videoViewer);

    setCentralWidget(widget);
    setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::FramelessWindowHint);

    QPalette palette = QWidget::palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);

    connect(&player, &QMediaPlayer::stateChanged, this, &QPlayer::mediaStateChanged);
    connect(&player, &QMediaPlayer::positionChanged, this, &QPlayer::positionChanged);
    connect(&player, &QMediaPlayer::durationChanged, this, &QPlayer::durationChanged);
    connect(&player, &QMediaPlayer::currentMediaChanged, this, &QPlayer::currentMediaChanged);
    connect(&player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &QPlayer::handleError);
    connect(&timer1, &QTimer::timeout, this, &QPlayer::displayImage);
    connect(&timer2, &QTimer::timeout, this, &QPlayer::next);
}

QPlayer::~QPlayer()
{
}

bool QPlayer::isPlayerAvailable() const
{
    return player.isAvailable();
}

void QPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        if(isFullScreen()){
            showMaximized();
            control->show();
        }else {
            showFullScreen();
            control->hide();
        }
    }
}

void QPlayer::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){

    }
}

void QPlayer::keyReleaseEvent(QKeyEvent *e)
{
    int v;

    switch(e->key())
    {
        case Qt::Key_VolumeUp:
        case Qt::Key_Up:
            v = player.volume();
            if(v < 100){
                v = v + 10;
                v = v<100?v:100;
                player.setVolume(v);
                qDebug() << "Key_VolumeUp: " << v;
            }
            break;
        case Qt::Key_VolumeDown:
        case Qt::Key_Down:
            v = player.volume();
            if(v > 0){
                v = v - 10;
                v = v>0?v:0;
                player.setVolume(v);
                qDebug() << "Key_VolumeDown: " << v;
            }
            break;
    }
}

void QPlayer::exit()
{
    qApp->exit(0);
}

void QPlayer::displayImage()
{
    QMimeDatabase db;
    QUrl url = list->currentMedia().canonicalUrl();
    QString s = db.mimeTypeForUrl(url).name();

    player.pause();
    timer1.stop();
    if(MIME_IS_IMAGE(s)){
        QImage img;
        img.load(url.toLocalFile());
        int w = geometry().width();
        int h = geometry().height() * 4 /5;
        imageViewer->setPixmap(QPixmap::fromImage(img.scaled(w, h, Qt::KeepAspectRatio)));
        imageViewer->show();
        if(list->mediaCount() > 1){
            timer2.start(5000);
        }
    }
}

void QPlayer::next()
{
    timer2.stop();
    list->setCurrentIndex(list->nextIndex());
    play();
}

void QPlayer::setPlaylist(QStringList l)
{
    if(! l.empty()){
        for(int i = 0; i < l.size(); i++){
            QString str = l.at(i);
            QFile f(l.at(i));
            QUrl u(str);

            if(f.exists()){
                list->addMedia(QUrl::fromLocalFile(str));
            }else if(u.isValid()){
                list->addMedia(u);
            }

        }
        list->setCurrentIndex(1);
        player.setPlaylist(list);
    }

    if(list->mediaCount() > 1){
        list->setPlaybackMode(QMediaPlaylist::Loop);
    } else {
        list->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
    }
}

void QPlayer::load(const QUrl &url)
{
    player.setMedia(url);
    playButton->setEnabled(true);
}

void QPlayer::play()
{
    switch(player.state()) {
    case QMediaPlayer::PlayingState:
        player.pause();
        break;
    default:
        player.play();
        break;
    }
}

void QPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    case QMediaPlayer::StoppedState:
        qApp->exit(0);
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void QPlayer::positionChanged(qint64 position)
{
    positionSlider->setValue(position);
}

void QPlayer::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
}

void QPlayer::setPosition(int position)
{
    if(! player.isSeekable())
        return;
    if(! player.isMuted())
        player.setMuted(true);
    player.setPosition(position);
}

void QPlayer::unMute()
{
    player.setMuted(false);
}

void QPlayer::currentMediaChanged(const QMediaContent &media)
{
        QUrl url = media.canonicalUrl();
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForUrl(url);
        QString s = mt.name();
        if(MIME_IS_IMAGE(s)){
            imageViewer->show();
            player.setVideoOutput(new QVideoWidget);
            timer1.start(1);
        }else if (MIME_IS_AUDIO(s)){
            int w = geometry().width();
            int h = geometry().height() - controlLayout->sizeHint().height();
            imageViewer->setPixmap(QPixmap(":/album.jpeg").scaled(w*2/3, h*2/3, Qt::KeepAspectRatio));
            imageViewer->show();
            player.setVideoOutput(new QVideoWidget);
        }else if(MIME_IS_VIDEO(s)){
            imageViewer->hide();
            player.setVideoOutput(videoViewer);
            videoViewer->show();
        }
}

void QPlayer::handleError()
{
    const QString errorString = player.errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(player.error()));
    else
        message += errorString;
    qDebug() << message;

    if(QMessageBox::critical(this, "Qplayer Error", message)){
        exit();
    }
}

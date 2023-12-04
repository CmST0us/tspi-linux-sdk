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

#include "multivideoplayer.h"
#include <QtWidgets>
#include <QVideoSurfaceFormat>

MultiVideoPlayer::MultiVideoPlayer(QStringList list)
{
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    int i, num = 0;
    resize(screenGeometry.width(), screenGeometry.height());

    playerList = new QList<QMediaPlayer*>();
    videoList = new QList<QVideoWidget*>();
    hLayoutList = new QList<QHBoxLayout*>();
    multiPlayList = new QList<QMediaPlaylist*>();
    urlList = list;

    exitButton = new QPushButton(tr("Exit"));
    connect(exitButton, &QAbstractButton::clicked, this, &MultiVideoPlayer::exit);

    if(urlList.count() > 0){
        num = qSqrt(urlList.count());
        if(qreal(num) < qSqrt(list.count())){
            num++;
        }
    }

    for(i=0; i<num; i++){
        QHBoxLayout *layout = new QHBoxLayout;
        hLayoutList->append(layout);
    }

    for(i=0; i<urlList.count(); i++){
        QMediaPlayer *player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
        playerList->append(player);
        QVideoWidget *video = new QVideoWidget();
        videoList->append(video);
        QMediaPlaylist *playlist = new QMediaPlaylist(this);
        multiPlayList->append(playlist);
        QString str = list.at(i);
        QFile f(urlList.at(i));
        QUrl u(str);

        if(f.exists()){
            playlist->addMedia(QUrl::fromLocalFile(str));
        }else if(u.isValid()){
            playlist->addMedia(u);
        }
        playlist->setCurrentIndex(1);
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
        player->setPlaylist(playlist);
        QHBoxLayout *layout = hLayoutList->value(i/num);
        if(layout){
            layout->addWidget(video);
            layout->setMargin(0);
            layout->setSpacing(0);
        }
    }

    QBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    for(i=0; i<hLayoutList->count(); i++){
        QWidget *widget = new QWidget();
        widget->setLayout(hLayoutList->value(i));
        mainLayout->addWidget(widget);
    }
    mainLayout->addWidget(exitButton);

    QWidget *widget = new QWidget();
    widget->setLayout(mainLayout);

    setCentralWidget(widget);
    setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::FramelessWindowHint);

    QPalette palette = QWidget::palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
}

MultiVideoPlayer::~MultiVideoPlayer()
{
}

void MultiVideoPlayer::play()
{
    for(int i=0; i<urlList.count(); i++){
        QVideoWidget *video = videoList->value(i);
        QMediaPlayer *player = playerList->value(i);
        if(video && player){
            player->setVideoOutput(video);
            video->show();
            player->play();
        }
    }
}

void MultiVideoPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
        if(isFullScreen()){
            showNormal();
        }else {
            showFullScreen();
        }
    }
}

void MultiVideoPlayer::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton){
    }
}

void MultiVideoPlayer::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_VolumeUp:
        case Qt::Key_Up:
            break;
        case Qt::Key_VolumeDown:
        case Qt::Key_Down:
            break;
    }
}

void MultiVideoPlayer::exit()
{
    qApp->exit(0);
}


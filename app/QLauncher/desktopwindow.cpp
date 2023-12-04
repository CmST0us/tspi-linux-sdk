/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include "desktopwindow.h"
#include "xdgdesktopfile.h"
#include <QAbstractScrollArea>

#define DESKTOP_DIR "/usr/share/applications"

#define ICON_SIZE 128
#define ITEM_SPACE ICON_SIZE / 2
#define FONT_SIZE ICON_SIZE / 6
DesktopWindow::DesktopWindow()
{
    QDesktopWidget *desktopwidget = QApplication::desktop();
    QRect desktoprect = desktopwidget->availableGeometry();
    qDebug() << "QLauncher available size :" << desktoprect.width() << "x" << desktoprect.height();

    QDir dir(DESKTOP_DIR);
    QStringList filters;
    filters << "*.desktop";
    dir.setNameFilters(filters);
    list = dir.entryInfoList();

    if (list.length()!=0) {
        QListWidget *desktopList = new QListWidget();
        for (int i = 0; i < list.size(); ++i) {
            XdgDesktopFile df;
            df.load(list.at(i).fileName());
            QListWidgetItem *item = new QListWidgetItem(df.icon(ICON_SIZE), df.name());
//            qDebug() << "QLauncher add application:" << i << df.name();
            QFont font;
            font.setPixelSize(FONT_SIZE);
            item->setFont(font);
            item->setSizeHint(QSize(ICON_SIZE + ITEM_SPACE, ICON_SIZE + FONT_SIZE + ITEM_SPACE));
            desktopList->addItem(item);
        }
        desktopList->setSpacing(ITEM_SPACE);
        desktopList->setViewMode(QListView::IconMode);
        desktopList->setFlow(QListView::LeftToRight);
        desktopList->setDragEnabled(false);
        desktopList->setWordWrap(true);
        desktopList->setWrapping(true);
        desktopList->setFrameShape(QListWidget::NoFrame);
        desktopList->setGridSize(QSize(ICON_SIZE + ITEM_SPACE, ICON_SIZE + ITEM_SPACE));
        desktopList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        desktopList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        desktopList->setStyleSheet("background-color:transparent");
        desktopList->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        desktopList->setResizeMode(QListWidget::Adjust);
        setCentralWidget(desktopList);
        setWindowState(Qt::WindowMaximized);
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
        setStyleSheet("QListWidget{background-color:transparent}");
        QProcess p;
        p.start("cat /etc/os-release");
        p.waitForStarted();
        p.waitForFinished();
        QTextStream tt(p.readAllStandardOutput());
        QString ll;
        bool found = 0;
        do{
            ll = tt.readLine();
//            qDebug() << ll;
            if(!ll.compare("NAME=\"Debian GNU/Linux\"")){
                found = true;
            }
        }while (! ll.isNull());
        if(found)
            setStyleSheet("QMainWindow{border-image: url(:/resources/background_debian.jpg);}");
        else
            setStyleSheet("QMainWindow{border-image: url(:/resources/background_linux.jpg);}");
        connect(desktopList, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
    } else
        qDebug()<<"QLauncher no found .desktop file in"<<DESKTOP_DIR;
        resize(QGuiApplication::primaryScreen()->availableSize());
}

void DesktopWindow::on_itemClicked(QListWidgetItem * item)
{
    for (int i = 0; i < list.size(); ++i) {
        XdgDesktopFile df;
        df.load(list.at(i).fileName());
        if (df.name() == item->text()) {
            qDebug()<<"QLauncher start"<<df.name();
            df.startDetached();
        }
    }
}



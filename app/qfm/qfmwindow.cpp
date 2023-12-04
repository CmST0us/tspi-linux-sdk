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

#include "qfmwindow.h"
#include <QApplication>
#include <QColor>
#include <QCheckBox>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QDirIterator>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QToolBar>


#define FILEMANAGER "File Manager"
QfmWindow::QfmWindow(QWidget *parent) : QMainWindow(parent)
{
    m_curDir= "top";
    m_multichecking = false;
    m_topDirList<<"Root"<<"Home"<<"Oem"<<"User Data"<<"SD Card"<<"USB Disk";
    m_topPathList<<"/"<<QStandardPaths::standardLocations(QStandardPaths::HomeLocation)<<"/oem"<<"/userdata"<<"/sdcard"<<"/udisk";
    m_videoSuffixList<<"*.mp4"<<"*.m4v"<<"*.avi"<<"*.wmv"<<"*.mkv"<<"*.asf"<<"*.mov"<<"*.ts"<<"*.mpg"<<"*.mpeg"<<"*.vob"<<"*.m2ts"<<"*.webm";
    m_musicSuffixList<<"*.mp1"<<"*.mp2"<<"*.mp3"<<"*.wav"<<"*.wave"<<"*.wma"<<"*.ogg"<<"*.m4a"<<"*.flac";
    m_picSuffixList<<"*.jpg"<<"*.png"<<"*.bmp"<<"*.jpeg";
    m_Filter = FileAll;
    initLayout();
    connect(m_btnopen, SIGNAL(clicked(bool)), this, SLOT(on_openClicked()));
    connect(m_btnreturn, SIGNAL(clicked(bool)), this, SLOT(on_returnClicked()));
    connect(m_listWid, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
}

void QfmWindow::initLayout()
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    resize(availableGeometry.width(), availableGeometry.height());

    m_btnreturn = new QPushButton(this);
    m_btnreturn->setStyleSheet(tr("border-image: url(:/image/return.png);"));
    QPixmap pixmap(":/image/return.png");
    m_btnreturn->setFixedSize(pixmap.width(), pixmap.height());

    m_titleLabel = new QLabel(tr(FILEMANAGER), this);
    QFont font = m_titleLabel->font();
    font.setBold(true);
    m_titleLabel->setFont(font);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("background-color:rgba(255, 255, 255, 0)");

    m_btnopen = new QPushButton(this);
    m_btnopen->setText(tr("open"));
    m_btnopen->setVisible(true);

    m_listWid = new QListWidget(this);
    m_listWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listWid->setStyleSheet("background-color:rgb(204,228,247)");
    getlist(m_listWid, nullptr);
    m_listWid->setObjectName(tr("filelist"));

    m_toolbar = new QToolBar(this);
    m_toolbar->addWidget(m_btnreturn);
    m_toolbar->addWidget(m_btnopen);
    m_toolbar->addWidget(m_titleLabel);


    setCentralWidget(m_listWid);
    addToolBar(m_toolbar);
    setStyleSheet("background-color:rgb(204,228,247)");
    setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::FramelessWindowHint);
}

QFileInfoList QfmWindow::getFiles(const QString &path)
{
    QDir *dir = new QDir(path);
    QFileInfoList files;

    switch (m_Filter) {
    case FileAll:
        files = dir->entryInfoList(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
        break;
    case FileVideo:
        files = dir->entryInfoList(m_videoSuffixList, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
        break;
    case FileMusic:
        files = dir->entryInfoList(m_musicSuffixList, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
        break;
    case FilePic:
        files = dir->entryInfoList(m_picSuffixList, QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
        break;
    }

    for(int i=0;i<files.count();i++){
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForFile(files[i].fileName());
        qDebug() << files[i].fileName() << "mime type: " << type.name() << "dir: " << files[i].isDir();
    }

    return files;
}

void QfmWindow::updatelabel(void)
{
    if(m_curDir.compare("top")){
        m_titleLabel->setText(m_curDir);
     }else {
        m_titleLabel->setText(tr(FILEMANAGER));
    }
}

void QfmWindow::updatecurdir(QString path, bool back)
{
    if(istop(m_curDir)){
        for(int i = 0; i < m_topDirList.count(); i++){
            if(!m_topDirList.at(i).compare(path)){
                m_curDir = m_topPathList.at(i);
            }
        }
    }else if(inTopList(m_curDir)){
        if(back){
             m_curDir = "top";
             return;
        }
        if(m_curDir.compare("/")){
            m_curDir = m_curDir + '/' + path;
        }else {
            m_curDir = '/' + path;
        }
    }else {
        if(back){
            int index = m_curDir.lastIndexOf('/');

            if(index == 0){
                m_curDir = "/";
            }else {
                m_curDir = m_curDir.mid(0, index);
            }
        }else {
            if(m_curDir.compare("/")){
                m_curDir = m_curDir + '/' + path;
            }else {
                m_curDir = '/' + path;
            }
        }
    }
}

QListWidgetItem* QfmWindow::getitem(QString name)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(name);
    item->setTextAlignment(Qt::AlignJustify);
    item->setTextColor(QColor(Qt::black));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    return item;
}

void QfmWindow::getlist(QListWidget *listWid, QListWidgetItem *item)
{
    QFileInfoList files;
    bool intop = istop(m_curDir);
    int row = listWid->row(item);
    QString path;// = item->text();

    if(item){
        path = item->text();
    }

    listWid->clear();
    if(intop){
        if(item){
            path = m_topPathList.at(row);
            files = getFiles(path);
            if(files.empty()){
                return;
            }

            for(int i = 0; i < files.count(); ++i){
                listWid->addItem(getitem(files[i].fileName()));
            }
        }else{
            for(int i = 0; i < m_topDirList.size(); ++i){
                listWid->addItem(getitem(m_topDirList.at(i)));
            }
        }

    }else {
        if(inTopList(m_curDir)){
            if(m_curDir.compare("/")){
                files = getFiles(m_curDir + '/' + path);
            }else{
                files = getFiles('/' + path);
            }

            if(files.empty()){
                return;
            }

            for(int i = 0; i < files.count(); ++i){
                listWid->addItem(getitem(files[i].fileName()));
            }
        }else {
            if(m_curDir.compare("/")){
                files = getFiles(m_curDir + '/' + path);
            }else{
                files = getFiles('/' + path);
            }

            if(files.empty()){
                return;
            }

            for(int i = 0; i < files.count(); ++i){
                listWid->addItem(getitem(files[i].fileName()));
            }
        }

    }

}

bool QfmWindow::istop(QString path)
{
    if(! path.compare("top")){
        return true;
    }

    return false;
}

bool QfmWindow::inTopList(QString path)
{
    for(int i = 0; i < m_topPathList.count(); i++){
        if(!m_topPathList.at(i).compare(path)){
            return true;
        }
    }
    return false;
}

int QfmWindow::getCheckedItemCnt(void)
{
    int cnt = 0;
    for(int i=0; i < m_listWid->count(); i++){
        QListWidgetItem * ii = m_listWid->item(i);
        if(ii->checkState() == Qt::Checked){
            cnt++;
        }
    }
    return cnt;
}

void QfmWindow::on_openClicked()
{
    QStringList files;
    bool gogogo = false;

    if(! istop(m_curDir)){
        for(int i=0; i < m_listWid->count(); i++){
            QListWidgetItem * item = m_listWid->item(i);
            if(item->checkState() == Qt::Checked){
                QString path = m_curDir + "/" + item->text();
                QFileInfo file(path);
                if(file.exists()){
                    gogogo = true;
                    files.append(file.absoluteFilePath());
                }
            }
        }
    }
    if(gogogo){
        m_mimeUtils.openFiles(files);
    }
}

void QfmWindow::on_returnClicked()
{
    if(istop(m_curDir)){
        qApp->exit(0);
    }else{
        updatecurdir(nullptr, true);
        getlist(m_listWid, nullptr);
        updatelabel();
    }
}

void QfmWindow::on_itemClicked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked){
        m_btnopen->show();
    }
    int checkedcnt = getCheckedItemCnt();
    if(checkedcnt >= 1){
        return;
    }
    if(! istop(m_curDir)){
        QString path = m_curDir + "/" + item->text();
        QDir dir(path);
        if(! dir.exists()){
            m_mimeUtils.openInApp(path, "");
            return;
        }
    }
    QString path = item->text();
    getlist(m_listWid, item);
    updatecurdir(path, false);
    updatelabel();
}


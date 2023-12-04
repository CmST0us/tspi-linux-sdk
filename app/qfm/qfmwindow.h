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
#ifndef QFMWINDOW_H
#define QFMWINDOW_H

#include <QFileInfoList>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QThread>
#include "mimeutils.h"

enum FileFilter
{
    FileAll,
    FileVideo,
    FileMusic,
    FilePic
};

class QfmWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit QfmWindow(QWidget *parent = 0);
    ~QfmWindow(){}

private:
    enum FileFilter m_Filter;
    QStringList m_videoSuffixList;
    QStringList m_musicSuffixList;
    QStringList m_picSuffixList;
    bool mediaHasUpdate;
    QPushButton *m_btnreturn;
    QLabel *m_titleLabel;
    QPushButton *m_btnopen;
    QListWidget *m_listWid;
    QStringList m_topDirList;
    QStringList m_topPathList;
    QString m_curDir;
    MimeUtils m_mimeUtils;
    QToolBar *m_toolbar;
    bool m_multichecking;
    void initLayout();
    QFileInfoList getFiles(const QString &path);
    void updatelabel();
    void updatecurdir(QString path, bool back);
    QListWidgetItem* getitem(QString name);
    void getlist(QListWidget *listWid, QListWidgetItem *curitem);
    bool istop(QString path);
    bool inTopList(QString path);
    int getCheckedItemCnt(void);
private slots:
    void on_openClicked();
    void on_returnClicked();
    void on_itemClicked(QListWidgetItem *item);
};


#endif // QFMWINDOW_H


#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include "qtupdate.h"

#define SD_UPDATE_FILE "/sdcard/update.img"
#define DATA_UPDATE_FILE "/userdata/update.img"
#define UPDATE_EXE "/usr/bin/update"

qtUpdate::qtUpdate(QWidget *parent)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(parent);

    QString s = "Please put update.img in \n";
    s.append(SD_UPDATE_FILE);
    s.append("\n or \n");
    s.append(DATA_UPDATE_FILE);
    s.append("\n then click OK button.");
    QFont font;
    font.setBold(true);
    font.setPixelSize(availableGeometry.height()/40);
    label.setFont(font);
    label.setText(s);
    label.setAlignment(Qt::AlignCenter);
    btn.setText("O K");
    connect(&btn, SIGNAL(clicked(bool)), this, SLOT(on_btnClicked()));
    vLayout.addWidget(&label);
    vLayout.addWidget(&btn);
    setLayout(&vLayout);
    setStyleSheet("background-color:rgb(204,228,247)");
    setObjectName("Update");
}

qtUpdate::~qtUpdate()
{

}

void qtUpdate::on_btnClicked()
{
    QFileInfo userdata = QFileInfo(DATA_UPDATE_FILE);
    QFileInfo sd = QFileInfo(SD_UPDATE_FILE);
    QFileInfo update = QFileInfo(UPDATE_EXE);
    QString path;

    if(userdata.exists()){
        path = DATA_UPDATE_FILE;
    }else if(sd.exists()){
        path = SD_UPDATE_FILE;
    }else {
        QMessageBox::warning(this, "Error", "Don't find update.img in " DATA_UPDATE_FILE " and " SD_UPDATE_FILE "!");
        return;
    }

    QMessageBox::StandardButton rb = QMessageBox::question(
                this, "Update",
                "Found update.img in " + path + ", Do you want to reboot and update it?",
                QMessageBox::Yes | QMessageBox::No);
    if(rb == QMessageBox::Yes){
        if(update.exists()){
            QStringList slist;
            QProcess p;
            slist << "ota" << path;
            p.start(UPDATE_EXE, slist);
            p.waitForStarted();
            p.waitForFinished();
            QString err = QString::fromLocal8Bit(p.readAllStandardOutput());
            QMessageBox::critical(this, "Error", err);
        }else {
            QMessageBox::warning(this, "Error", "Don't find " UPDATE_EXE "!");
        }
    }
}

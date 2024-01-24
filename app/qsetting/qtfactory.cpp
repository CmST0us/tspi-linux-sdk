
#include <QApplication>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include "qtfactory.h"

#define UPDATE_EXE "/usr/bin/update"

qtFactoryReset::qtFactoryReset(QWidget *parent)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(parent);
    QFont font;
    font.setBold(true);
    font.setPixelSize(availableGeometry.height()/40);
    label.setFont(font);
    label.setText("Factory Reset will wipe all the user data.\n Make sure your device are ready.\n then click OK button.");
    label.setAlignment(Qt::AlignCenter);
    btn.setText("O K");
    connect(&btn, SIGNAL(clicked(bool)), this, SLOT(on_btnClicked()));
    vLayout.addWidget(&label);
    vLayout.addWidget(&btn);
    setLayout(&vLayout);
    setStyleSheet("background-color:rgb(204,228,247)");
    setObjectName("Factory Reset");
}

qtFactoryReset::~qtFactoryReset()
{

}

void qtFactoryReset::on_btnClicked()
{
    QFileInfo update = QFileInfo(UPDATE_EXE);
    QString path;

    QMessageBox::StandardButton rb = QMessageBox::question(
                this, "Factory Reset",
                "Do you want to reboot and do factory reset? It will wipe all your user data",
                QMessageBox::Yes | QMessageBox::No);

    if(rb == QMessageBox::Yes){
        if(update.exists()){
            QProcess p;

            p.start(UPDATE_EXE);
            p.waitForStarted();
            p.waitForFinished();
            QString err = QString::fromLocal8Bit(p.readAllStandardOutput());
            QMessageBox::critical(this, "Error", err);
        }else {
            QMessageBox::warning(this, "Error", "Don't find " UPDATE_EXE "!");
        }
    }
}

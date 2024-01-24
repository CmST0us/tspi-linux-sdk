#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QToolBar>
#include <QVBoxLayout>

#define USERDATA_SETTING_CFG "/userdata/setting.cfg"
#define HOME_SETTING_CFG QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/setting.cfg"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)  
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    resize(availableGeometry.width(), availableGeometry.height());
    setMinimumWidth(availableGeometry.width());

    audio = nullptr;
    wifi = nullptr;
    bt = nullptr;
    update = nullptr;
    factoryReset = nullptr;

    getConfig();

    returnBtn.setStyleSheet(tr("border-image: url(:/return.png);"));
    QPixmap pixmap(":/return.png");
    returnBtn.setFixedSize(pixmap.width(), pixmap.height());

    QFont font;
    title.setText("Setting");
    font.setPixelSize(availableGeometry.height()/20);
    title.setFont(font);
    title.setAlignment(Qt::AlignLeft);


    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(&returnBtn);
    hlayout->addWidget(&title);
    hlayout->addWidget(&subTitle);
    hlayout->addWidget(&toggleBtn);
    font.setPixelSize(pixmap.height()*1/3);
    subTitle.setFont(font);
    subTitle.setAlignment(Qt::AlignCenter);
    subTitle.setVisible(false);
    font.setPixelSize(pixmap.height()/2);
    toggleBtn.setFont(font);
    toggleBtn.setFixedSize(pixmap.width(), pixmap.height());
    toggleBtn.setVisible(false);

    font.setPixelSize(availableGeometry.height()/20);
//    QListWidgetItem *audio = new QListWidgetItem(tr("Audio"), &listWidget);
//    audio->setFont(font);
    QListWidgetItem *wifi = new QListWidgetItem(tr("WiFi"), &listWidget);
    wifi->setFont(font);
    QListWidgetItem *bt = new QListWidgetItem(tr("BT"), &listWidget);
    bt->setFont(font);
    QListWidgetItem *update = new QListWidgetItem(tr("Update"), &listWidget);
    update->setFont(font);
    QListWidgetItem *factory = new QListWidgetItem(tr("Factory Reset"), &listWidget);
    factory->setFont(font);
    listWidget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    stack.addWidget(&listWidget);
    stack.setCurrentIndex(0);

    QWidget *hWidget = new QWidget;
    hWidget->setLayout(hlayout);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(hWidget);
    vlayout->addWidget(&stack);

    QWidget *vWidget = new QWidget;
    vWidget->setLayout(vlayout);
    setCentralWidget(vWidget);
    setStyleSheet("background-color:rgb(204,228,247)");
    setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::FramelessWindowHint);

    connect(&listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
    connect(&returnBtn, SIGNAL(clicked(bool)), this, SLOT(on_returnClicked()));
}

MainWindow::~MainWindow()
{

}

QString MainWindow::saveConfig(int volume, int wifi, int bt)
{
    QString path;
    QFileInfo fi("/userdata");
    if(fi.isDir()){
        path = USERDATA_SETTING_CFG;
    }else {
        path = HOME_SETTING_CFG;
    }
    QFile file(path);

    if(! file.open(QIODevice::ReadWrite | QIODevice::Text)){
        qDebug() << "open rw setting.cfg failed";
        return nullptr;
    }
    QTextStream out(&file);
    out << "volume = " + QString::number(volume) << endl;
    out << "isWifiOn = " + QString::number(wifi) << endl;
    out << "isBtOn = " + QString::number(bt) << endl;
    out.flush();
    file.close();
    return path;
}

int MainWindow::getValue(QTextStream *in, QString text)
{
    in->seek(0);
    do {
        QString temp = in->readLine();
        if(temp.startsWith(text)){
            int size = text.count();
            QString result = temp.mid(size, temp.count());
            return result.toInt();
        }
    }while (! in->atEnd());

    return -1;
}

void MainWindow::getConfig()
{
    QString path;

    if(QFile::exists(USERDATA_SETTING_CFG)){
        path = USERDATA_SETTING_CFG;
    }else if(QFile::exists(HOME_SETTING_CFG)){
        path = HOME_SETTING_CFG;
    }else {
        path = saveConfig(100, 0, 0);
    }

    QFile file(path);
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "open ro setting.cfg failed";
        return;
    }
    QTextStream in(&file);
    volume = getValue(&in, "volume = ");
    isWifiOn = getValue(&in, "isWifiOn = ");
    isBtOn = getValue(&in, "isBtOn = ");
}


void MainWindow::on_itemClicked(QListWidgetItem *item)
{
    title.setText(item->text());

    if(! item->text().compare("Audio")){
        audio = new qtAudio(this);
        stack.addWidget(audio);
        stack.setCurrentIndex(stack.indexOf(audio));
    } else if(! item->text().compare("WiFi")){
        wifi = qtWifi::getInstance(this, &subTitle, &toggleBtn, isWifiOn);
        stack.addWidget(wifi);
        stack.setCurrentIndex(stack.indexOf(wifi));
    } else if(! item->text().compare("BT")){
        bt = qtBT::getInstance(this, &subTitle, &toggleBtn, isBtOn);
        stack.addWidget(bt);
        stack.setCurrentIndex(stack.indexOf(bt));
    } else if(! item->text().compare("Update")){
        update = new qtUpdate(this);
        stack.addWidget(update);
        stack.setCurrentIndex(stack.indexOf(update));
    } else if(! item->text().compare("Factory Reset")){
        factoryReset = new qtFactoryReset(this);
        stack.addWidget(factoryReset);
        stack.setCurrentIndex(stack.indexOf(factoryReset));
    }
}

void MainWindow::on_returnClicked()
{
    if(title.text() == "Setting"){
        if(bt){
            isBtOn = bt->isOn();
            delete bt;
            bt = nullptr;
        }
        saveConfig(volume, isWifiOn, isBtOn);
        qApp->exit(0);
    }else {
        title.setText("Setting");
        if(audio){
            stack.removeWidget(audio);
            delete audio;
            audio = nullptr;
        }else if(wifi){
            stack.removeWidget(wifi);
            isWifiOn = wifi->isOn();
            delete wifi;
            wifi = nullptr;
        }else if(bt){
            stack.removeWidget(bt);
            isBtOn = bt->isOn();
            delete bt;
            bt = nullptr;
        }else if(update){
            stack.removeWidget(update);
            delete update;
            update = nullptr;
        }else if(factoryReset){
            stack.removeWidget(factoryReset);
            delete factoryReset;
            factoryReset = nullptr;
        }
        stack.setCurrentIndex(stack.indexOf(&listWidget));
    }
}

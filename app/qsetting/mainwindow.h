#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include "qtaudio.h"
#include "qtbt.h"
#include "qtfactory.h"
#include "qtupdate.h"
#include "qtwifi.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QPushButton returnBtn;
    QLabel title;
    QLabel subTitle;
    QPushButton toggleBtn;
    QStackedWidget stack;
    QListWidget listWidget;
    qtAudio *audio;
    qtWifi *wifi;
    qtBT *bt;
    qtUpdate *update;
    qtFactoryReset *factoryReset;
    int volume;
    bool isWifiOn;
    bool isBtOn;
    QString saveConfig(int volume, int wifi, int bt);
    int getValue(QTextStream *in, QString text);
    void getConfig();
private slots:
    void on_itemClicked(QListWidgetItem *item);
    void on_returnClicked();
};

#endif // MAINWINDOW_H

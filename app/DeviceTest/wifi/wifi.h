#ifndef WIFI_H
#define WIFI_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextCodec>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include "common/common.h"
#include "wifithread.h"
#include "connectthread.h"

class Wifi : public QWidget
{
    Q_OBJECT
public:
    explicit Wifi(QWidget *parent = nullptr);
    void initWindow(int w, int h);
    bool testResult=false;
    void closeWindow();

protected:
    Common common;
    QLineEdit *wlanIp;
    QLineEdit *wlanMac;
    WifiThread *thread;
    void showDialog(int row);
    void connectWifi(QString apName, QString passwd);
    void pingTest(QString ip);
    QPushButton *title;
    QTimer *timer;

protected slots:
    void setApMap(QList<QMap<QString, QString> > infoList);
    void getConnectState(QString ip);

signals:
    void testFinish();
};

#endif // WIFI_H

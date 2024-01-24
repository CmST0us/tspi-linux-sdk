#ifndef QTWIFI_H
#define QTWIFI_H

#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QThread>
#include <QTimer>

extern "C" {
#include "Rk_wifi.h"
#include "Rk_softap.h"
}

class qtWifi : public QListWidget
{
    Q_OBJECT

public:
    qtWifi(QWidget *parent = nullptr, QLabel *label = nullptr, QPushButton *btn = nullptr, bool on = false);
    ~qtWifi();

    static qtWifi* getInstance(QWidget *parent, QLabel *label, QPushButton *btn,  bool on = false)
    {
        if (!_instance) {
            _instance = new qtWifi(parent, label, btn, on);
        }
        return _instance;
    }

    static qtWifi* getInstance(void)
    {
        return _instance;
    }

    bool isOn();
    void turnOn();
    void turnOff();
private:
    static int wifi_callback(RK_WIFI_RUNNING_State_e state,
                             RK_WIFI_INFO_Connection_s *info);
    static qtWifi* _instance;
    QLabel *text;
    QPushButton *switchBtn;
    QTimer *Timer;
    QString ssid;

public slots:
    void on_btnClicked();
    void on_itemClicked(QListWidgetItem *item);
    void on_timer_timeout();
};

#endif /* QTWIFI_H */

#ifndef QTBT_H
#define QTBT_H

#include <QDebug>
#include <QLabel>
#include <QListWidget>
#include <QMutex>
#include <QPushButton>
#include <QProcess>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QWidget>
#ifdef RKWIFIBTAPP
#include <RkBtSource.h>
#include <RkBtBase.h>
#else

typedef enum {
        RK_BT_STATE_OFF,
        RK_BT_STATE_ON,
        RK_BT_STATE_TURNING_ON,
        RK_BT_STATE_TURNING_OFF,
} RK_BT_STATE;

typedef enum {
        RK_BT_BOND_STATE_NONE,
        RK_BT_BOND_STATE_BONDING,
        RK_BT_BOND_STATE_BONDED,
} RK_BT_BOND_STATE;

typedef enum {
        RK_BT_DISC_STARTED,
        RK_BT_DISC_STOPPED_AUTO,
        RK_BT_DISC_START_FAILED,
        RK_BT_DISC_STOPPED_BY_USER,
} RK_BT_DISCOVERY_STATE;

typedef enum {
        BT_SOURCE_EVENT_CONNECT_FAILED,
        BT_SOURCE_EVENT_CONNECTED,
        BT_SOURCE_EVENT_DISCONNECTED,
        BT_SOURCE_EVENT_RC_PLAY,    /* remote control command */
        BT_SOURCE_EVENT_RC_STOP,
        BT_SOURCE_EVENT_RC_PAUSE,
        BT_SOURCE_EVENT_RC_FORWARD,
        BT_SOURCE_EVENT_RC_BACKWARD,
        BT_SOURCE_EVENT_RC_VOL_UP,
        BT_SOURCE_EVENT_RC_VOL_DOWN,
} RK_BT_SOURCE_EVENT;
#endif

class qtBT : public QListWidget
{
    Q_OBJECT
    struct bt_dev_info{
        const char *address;
        const char *name;
        unsigned int bt_class;
        int rssi;
    };
public:
    qtBT(QWidget *parent = nullptr, QLabel *label = nullptr, QPushButton *btn = nullptr, bool on = false);
    ~qtBT();
    static qtBT* getInstance(QWidget *parent = nullptr, QLabel *label = nullptr, QPushButton *btn = nullptr, bool on = false)
    {
        if (!_instance) {
            _instance = new qtBT(parent, label, btn, on);
        }
        return _instance;
    }
    static void state_cb(RK_BT_STATE state);
    static void bond_cb(const char *bd_addr, const char *name, RK_BT_BOND_STATE state);
    static void scan_status_cb(RK_BT_DISCOVERY_STATE status);
    static void source_connect_cb(void *userdata, const char *bd_addr, const char *name, const RK_BT_SOURCE_EVENT enEvent);
    static void scan_cb(const char *address,const char *name, unsigned int bt_class, int rssi, int change);
    void open();
    void close();
    bool isOn();
    void turnOn();
    void turnOff();

public slots:
    void on_btnClicked();
    void on_itemClicked(QListWidgetItem *item);
private:
#ifdef RKWIFIBTAPP
    RkBtContent bt_content;
#endif
    QList<struct bt_dev_info*> dev_list;
    QMutex btmutex;
    static class qtBT* _instance;
    QLabel *text;
    QPushButton *switchBtn;
};

#endif // QTBT_H

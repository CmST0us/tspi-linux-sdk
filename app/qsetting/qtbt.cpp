
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QProcess>
#include "qtbt.h"
#include <unistd.h>
#include <sys/prctl.h>

qtBT* qtBT::_instance = nullptr;
void qtBT::state_cb(RK_BT_STATE state)
{
    switch(state) {
            case RK_BT_STATE_TURNING_ON:
                    qDebug() << "RK_BT_STATE_TURNING_ON";
                    break;
            case RK_BT_STATE_ON:
                    qDebug() << "RK_BT_STATE_ON";
                    break;
            case RK_BT_STATE_TURNING_OFF:
                    qDebug() << "RK_BT_STATE_TURNING_OFF";
                    break;
            case RK_BT_STATE_OFF:
                    qDebug() << "RK_BT_STATE_OFF";
                    break;
    }
}

void qtBT::bond_cb(const char *bd_addr, const char *name, RK_BT_BOND_STATE state)
{
    switch(state) {
            case RK_BT_BOND_STATE_NONE:
            case RK_BT_BOND_STATE_BONDING:
                break;
            case RK_BT_BOND_STATE_BONDED:
                qtBT *btlist = getInstance();
                qDebug() << "RK_BT_BOND_STATE_BONDED" << name << bd_addr;
                for(int i=0; i<btlist->count(); i++){
                    QListWidgetItem *ii = btlist->item(i);
                    QString str = ii->text();
                    QStringList sl  = str.split(" ");
                    QString addr = sl.at(1);
                    QString name = sl.at(2);
                    if(!addr.isEmpty() && !addr.compare(QString(bd_addr))){
                        QString str = "Paired " + QString(bd_addr) + " " + QString(name);
                        QListWidgetItem *iitem = btlist->takeItem(i);
                        iitem->setText(str);
                        btlist->insertItem(0, iitem);
                        return;
                    }
                }

                break;
    }
}

void qtBT::scan_status_cb(RK_BT_DISCOVERY_STATE status)
{
    switch(status) {
            case RK_BT_DISC_STARTED:
            case RK_BT_DISC_START_FAILED:
            case RK_BT_DISC_STOPPED_BY_USER:
                    break;
            case RK_BT_DISC_STOPPED_AUTO:
                    #ifdef RKDEVICEIO
                    rk_bt_start_discovery(1000, SCAN_TYPE_AUTO);
                    #endif
                    break;

    }
}

void qtBT::source_connect_cb(void *userdata, const char *bd_addr, const char *name, const RK_BT_SOURCE_EVENT enEvent)
{
    switch(enEvent)
    {
            case BT_SOURCE_EVENT_CONNECT_FAILED:
                    qDebug() << "BT_SOURCE_EVENT_CONNECT_FAILED" << name << bd_addr;
                    break;
            case BT_SOURCE_EVENT_CONNECTED:
                    qDebug() << "BT_SOURCE_EVENT_CONNECTED" << name << bd_addr;
                    break;
            case BT_SOURCE_EVENT_DISCONNECTED:
                    qDebug() << "BT_SOURCE_EVENT_DISCONNECTED" << name << bd_addr;
                    break;
            case BT_SOURCE_EVENT_RC_PLAY:
            case BT_SOURCE_EVENT_RC_STOP:
            case BT_SOURCE_EVENT_RC_PAUSE:
            case BT_SOURCE_EVENT_RC_FORWARD:
            case BT_SOURCE_EVENT_RC_BACKWARD:
            case BT_SOURCE_EVENT_RC_VOL_UP:
            case BT_SOURCE_EVENT_RC_VOL_DOWN:
                    qDebug() << "BT_SOURCE_EVENT_RC: " << BT_SOURCE_EVENT_RC_VOL_DOWN << name << bd_addr;
                    break;
    }
}


void qtBT::scan_cb(const char *address,const char *name, unsigned int bt_class, int rssi)
{
    struct bt_dev_info *dev = new bt_dev_info;
    qtBT *btlist = getInstance();
    dev->address = address;
    dev->name = name;
    dev->bt_class = bt_class;
    dev->rssi = rssi;
    btlist->dev_list.append(dev);

//    qDebug() << "address: " << address << "name: " << name << "get name: " << dev->name;

    for(int i=0; i<btlist->count(); i++){
        QListWidgetItem *ii = btlist->item(i);
        QString str = ii->text();
        QStringList sl  = str.split(" ");
        QString addr = sl.at(1);
        QString name = sl.at(2);
        if(!addr.isEmpty() && !addr.compare(QString(dev->address))){
            return;
        }
    }

    if(dev->address){
        QString str = "Unpaired " + QString(dev->address) + " " + QString(dev->name);
        btlist->addItem(new QListWidgetItem(str, btlist));
    }
}

void qtBT::open()
{
#ifdef RKDEVICEIO
    int count;
    RkBtScanedDevice *dev = NULL;
    static RkBtScanedDevice *g_dev_list_test;

    memset(&bt_content, 0, sizeof(RkBtContent));
    bt_content.bt_name = "ROCKCHIP_AUDIO";
    bt_content.bt_addr = "11:22:33:44:55:66";
    if(rk_bt_init(&bt_content) < 0) {
            qDebug() << "rk_bt_init error";
            return;
    }
    rk_bt_register_state_callback(qtBT::state_cb);
    rk_bt_register_bond_callback(qtBT::bond_cb);
    rk_bt_register_discovery_callback(qtBT::scan_status_cb);
    rk_bt_register_dev_found_callback(qtBT::scan_cb);
    rk_bt_source_register_status_cb(NULL, source_connect_cb);
    rk_bt_set_device_name("Rockchip Linux BT");
    rk_bt_start_discovery(1000, SCAN_TYPE_AUTO);
    rk_bt_enable_reconnect(0);
    rk_bt_source_open();
    rk_bt_get_paired_devices(&g_dev_list_test, &count);

    qDebug() << "current paired devices count: " << count;
    dev = g_dev_list_test;
    for(int i = 0; i < count; i++) {
        qDebug() << i << ": " << dev->remote_address << " " << dev->remote_name << " is_connected: " << dev->is_connected;
        QString str;
        if(dev->is_connected)
            str += "Connected ";
        else
            str += "Paired ";
        str += QString(dev->remote_address) + " " + QString(dev->remote_name);
        addItem(new QListWidgetItem(str, this));
        dev = dev->next;
    }
#endif
}

void qtBT::close()
{
    qDebug() << "bt close";
#ifdef RKDEVICEIO
    rk_bt_cancel_discovery();
    rk_bt_deinit();
#endif
}

qtBT::qtBT(QWidget *parent, QLabel *label, QPushButton *btn, bool on)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(parent);
    resize(availableGeometry.width(), availableGeometry.height());
    QFont font;
    font.setPixelSize(availableGeometry.height()/40);
    if(btn){
        switchBtn = btn;
        switchBtn->setCheckable(true);
        switchBtn->setVisible(true);
        switchBtn->setStyleSheet("QPushButton{background-color:green;}");
        switchBtn->setStyleSheet("QPushButton:checked{background-color:red;}");
        if (on){
            switchBtn->setChecked(true);
            switchBtn->setText("on");
        } else {
            switchBtn->setChecked(false);
            switchBtn->setText("off");
        }
        connect(switchBtn, SIGNAL(clicked(bool)), this, SLOT(on_btnClicked()));
    }

    setObjectName("BT");
    setFont(font);
    connect(this, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
    show();
    if(on)
        turnOn();
}

qtBT::~qtBT()
{
    #ifdef RKDEVICEIO
    rk_bt_cancel_discovery();
    #endif
    if(switchBtn){
        switchBtn->setVisible(false);
    }
    _instance = nullptr;
}

bool qtBT::isOn()
{
    if(switchBtn){
        if (! switchBtn->text().compare("on")){
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void qtBT::turnOn()
{
    if(QFile::exists("/userdata")){
        open();
    } else {
        QStringList list;
        list << "bt1" << "bt2" << "bt3" << "bt4" << "bt5" << "bt6" << "bt7";
        addItems(list);
    }
}

void qtBT::turnOff()
{
    if(QFile::exists("/userdata")){
        close();
    }
    clear();
}

void qtBT::on_btnClicked()
{
    if(switchBtn){
        if (! switchBtn->text().compare("on")){
            switchBtn->setText("off");
            turnOff();
        } else if (! switchBtn->text().compare("off")){
            switchBtn->setText("on");
            turnOn();
        }
    }
}

void qtBT::on_itemClicked(QListWidgetItem *item)
{
    QString str = item->text();
    QStringList sl  = str.split(" ");
    QString pair = sl.at(0);
    QString addr = sl.at(1);
    QString name = sl.at(2);

    if(!addr.isEmpty()){
        if(!pair.compare("Paired")){
            qDebug() << "connectint to " << addr.toLatin1().data();
            #ifdef RKDEVICEIO
            rk_bt_source_connect_by_addr(addr.toLatin1().data());
            #endif
        }else if(!pair.compare("Connected")){
            qDebug() << "disconnecting " << addr << name;
            #ifdef RKDEVICEIO
            rk_bt_source_disconnect_by_addr(addr.toLatin1().data());
            #endif
            takeItem(row(item));
        }else{
            qDebug() << "connectint to " << addr.toLatin1().data();
            #ifdef RKDEVICEIO
            rk_bt_source_connect_by_addr(addr.toLatin1().data());
            #endif
        }
    }
}


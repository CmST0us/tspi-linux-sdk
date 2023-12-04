#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QProcess>
#include <QTextStream>
#include "qtkeyboard.h"
#include "qtinputdialog.h"
#include "qtwifi.h"

qtWifi* qtWifi::_instance = nullptr;

qtWifi::qtWifi(QWidget *parent, QLabel *label, QPushButton *btn, bool on)
{
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(parent);
    resize(availableGeometry.width(), availableGeometry.height());

    QFont font;
    font.setPixelSize(availableGeometry.height()/20);

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

    if(label){
        text = label;
        text->setText("");
        text->setVisible(true);
    }else {
        text = nullptr;
    }
    setObjectName("WiFi");
    setFont(font);

    Timer = new QTimer(this);

    connect(this, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(on_itemClicked(QListWidgetItem *)));
    show();
    if (on)
        turnOn();
}

qtWifi::~qtWifi()
{
    if (switchBtn)
        switchBtn->setVisible(false);

    if (text)
        text->setVisible(false);
    _instance = nullptr;
}

void qtWifi::turnOn()
{
    RK_wifi_register_callback(wifi_callback);
    if (RK_wifi_enable(1) < 0)
        printf("[%s] Rk_wifi_enable 1 fail!\n", __func__);
    Timer->stop();
    Timer->setInterval(10000);
    connect(Timer, SIGNAL(timeout()), this, SLOT(on_timer_timeout()));

    if (text)
        text->setVisible(true);

    Timer->start();
    text->setText("Scaning");
}

void qtWifi::turnOff()
{
    if (RK_wifi_enable(0) < 0)
        printf("RK_wifi_enable 0 fail!\n");
    Timer->stop();
    clear();
    if (text)
        text->setVisible(false);
}

static int search_for_ssid(const char *str)
{
    const char key[] = "\"ssid\"";
    int i;

    if (strlen(str) < strlen(key))
        return -1;

    for (i = 0; i < (strlen(str) - strlen(key)); i++) {
        if (!strncmp(key, &str[i], strlen(key)))
            return i;
    }
    return -1;
}

static char *get_string(const char *str)
{
    int i, begin = -1, count;
    char *dst;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == '\"') {
            if (begin == -1) {
                begin = i;
                continue;
            } else {
                count = i - begin -1;
                if (!count)
                    return NULL;
                dst = strndup(&str[begin + 1], count);
                return dst;
            }
        }
    }
    return NULL;
}

int qtWifi::wifi_callback(RK_WIFI_RUNNING_State_e state,
                      RK_WIFI_INFO_Connection_s *info)
{
    qtWifi *wifi = qtWifi::getInstance();

    if (state == RK_WIFI_State_CONNECTED) {
        printf("RK_WIFI_State_CONNECTED\n");
        //wifi->ssid = QLatin1String(info->ssid);
        wifi->ssid = QString(info->ssid);
        wifi->text->setText(wifi->ssid + " Connected");
    } else if (state == RK_WIFI_State_CONNECTFAILED) {
        printf("RK_WIFI_State_CONNECTFAILED\n");
    } else if (state == RK_WIFI_State_CONNECTFAILED_WRONG_KEY) {
        printf("RK_WIFI_State_CONNECTFAILED_WRONG_KEY\n");
    } else if (state == RK_WIFI_State_OPEN) {
        printf("RK_WIFI_State_OPEN\n");
    } else if (state == RK_WIFI_State_OFF) {
        printf("RK_WIFI_State_OFF\n");
    } else if (state == RK_WIFI_State_DISCONNECTED) {
        printf("RK_WIFI_State_DISCONNECTED\n");
        wifi->text->setText("Scaning");
    } else if (state == RK_WIFI_State_SCAN_RESULTS) {
        char *scan_r, *str = nullptr;
        int cnt = 0, tmp = 0;
        QString line;
        QStringList list;

        if (wifi == nullptr)
                return 0;
        scan_r = strdup(RK_wifi_scan_r());
        wifi->clear();
        while (1) {
            tmp = search_for_ssid(&scan_r[cnt]);
            if (tmp == -1)
                break;
            str = get_string(&scan_r[cnt + tmp + 6]);
            if (str == NULL) {
                line = QString("NULL");
            } else {
                line = QString(str);
                free(str);
            }
            list << line;
            cnt += tmp + 6;
        }
        wifi->addItems(list);
        free(scan_r);
    }
    return 0;
}

bool qtWifi::isOn()
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

void qtWifi::on_itemClicked(QListWidgetItem *item)
{
    QKeyBoard::getInstance();
    inputDialog *dialog = inputDialog::getInstance(this);
    const char *c_ssid, *pswd;

    ssid = item->text();
    dialog->setText("Connect", "Cancel", "Password of " + item->text());
    if (dialog->isRunning())
        dialog->exit(false);

    int result = dialog->exec();
    if(result){
        QString str = dialog->getEditText();
        QProcess p;
        QStringList arguments;

        std::string s_ssid = ssid.toStdString();
        c_ssid = s_ssid.c_str();

        std::string s_pswd = str.toStdString();
        pswd = s_pswd.c_str();

        printf("ssid: %s, %s\n", c_ssid, pswd);
        if (RK_wifi_connect((char *)c_ssid, pswd) < 0)
            printf("RK_wifi_connect1 fail!\n");
    }
}

void qtWifi::on_btnClicked()
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

void qtWifi::on_timer_timeout()
{
    printf("refresh\n");
    if (RK_wifi_scan() < 0)
        printf("RK_wifi_scan fail!\n");
    Timer->start();
}

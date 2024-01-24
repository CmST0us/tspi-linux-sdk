#ifndef WIFISTRESS_H
#define WIFISTRESS_H

#include <QObject>
#include <QWidget>
#include <QNetworkInterface>
#include <QDialog>
#include <QProgressBar>
#include "common/common.h"
#include "ScanThread.h"
#include "WifiConnectThread.h"
#include "FileTransferThread.h"
#include "NetCheckThread.h"

class WifiStress : public QWidget
{
    Q_OBJECT
public:
    explicit WifiStress(QWidget *parent = nullptr);
    void initWindow(int w, int h);

    ~WifiStress();

private:
    Common common;
    QLabel *chnLabel;
    QLabel *freqLabel;
    QLabel *wlanMacLabel;
    QLineEdit *wlanMac;
    QComboBox *chnCombo;
    QComboBox *freqCombo;
    QPushButton *startClient;
    QPushButton *startServer;
    QPushButton *closeWindow;
    int scanTimes = 1;

    //sub window
    QTableWidget *apTable;
    QDialog *dialog;
    QLabel *stateLabel;
    QLabel *stateTxt;
    QProgressBar *processBar;
    QPushButton *historyBtn;
    QPushButton *closeDialogBtn;

    //history window
    QTextEdit *history = common.getTextEdit();

    QString chn;
    QString freq;
    QString mac;

    ScanThread *scanThread;
    WifiConnectThread * connectThread;
    QThread *m_scanThread;
    FileTransferThread *fileTransferThread;
    QThread *m_fileThread;
    NetWorkCheckThread *netCheckThread;
    QThread *m_netCheckThread;

    void initData();

private slots:
    void openHotPoint();
    void showConnectWindow();
    void handleApList(QList<QString> apList);
    void getConnectState(QString apName, bool state);
    void initProgressBar(qint64 fileSize);
    void updateProgressBar(qint64 reciveSize);
    void showHistory();
    void startThreads();
    void exitThreads();
    void initThreads();
    void fileDownSuccess();
    void socketTimeout();
    void socketDisconnect();
    void socketBreak();
    void changeFreq(QString freq);

signals:
    void connectWifi();
};

#endif // WIFISTRESS_H

#ifndef FILETRANSFERTHREAD_H
#define FILETRANSFERTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QFile>
#include <QThread>
#include "NetCheckThread.h"

class FileTransferThread : public QObject
{
    Q_OBJECT
public:
    explicit FileTransferThread(QObject *parent = nullptr);
    ~FileTransferThread();

private:
    void fileDownload();
    void handleFileDownload(QTcpSocket *socket);

private:
    bool isFileHead = true;
    QFile file;
    qint64 totalSize = 0;
    qint64 recvSize;
    QTcpSocket *tcpSocket;
    NetWorkCheckThread *netCheck;
    QThread *checkThread;

public slots:
    void startDownLoad();
    void handleNetDisconnected();

signals:
    void downLoadFinished();
    void fileSize(qint64 size);
    void reciveSize(qint64 size);
    void checkNetState();

    void socketConnectTimeout();
    void socketConnectSuccess();
    void socketDisconnect();
};

#endif // FILETRANSFERTHREAD_H

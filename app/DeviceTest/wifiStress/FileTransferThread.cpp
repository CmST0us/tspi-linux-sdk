#include "FileTransferThread.h"

#define SERVER_IP "192.168.12.1"
#define SERVER_PORT 6666
#define RECIVE_FILE "/socket_recive"

FileTransferThread::FileTransferThread(QObject *parent) : QObject(parent)
{

}

FileTransferThread::~FileTransferThread()
{

}

void FileTransferThread::startDownLoad()
{
    qDebug()<<"startFileDownload";
    this->fileDownload();
    isFileHead = true;
    totalSize = 0;
    recvSize = 0;
}

void FileTransferThread::fileDownload()
{
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::readyRead, [=] {this->handleFileDownload(tcpSocket);});
    connect(tcpSocket, &QTcpSocket::disconnected, [=] {
        qDebug()<<"socket disconnect!!!!!";
        emit reciveSize(0);
        emit socketDisconnect();
    });

    connect(tcpSocket, &QTcpSocket::connected, [=] {
        qDebug()<<"socket connect server success!";
        emit socketConnectSuccess();
    });

    tcpSocket->connectToHost(QHostAddress(SERVER_IP), SERVER_PORT);

    if(!tcpSocket->waitForConnected(10000)){
        qDebug()<<"socket connect timeout!!";
        if(tcpSocket->isOpen())
            tcpSocket->close();
        emit socketConnectTimeout();
    }
}

void FileTransferThread::handleFileDownload(QTcpSocket *socket)
{
    QByteArray buf = socket->readAll();
    if(isFileHead){
        totalSize = QString(buf).section("##", 0, 0).toInt();
        qDebug()<<"filesize:"<<totalSize;
        if(totalSize > 0){
            socket->write(QString::number(totalSize).toLocal8Bit());
            file.setFileName(RECIVE_FILE);
            file.remove();//not need save file

            if(!file.open(QIODevice::ReadWrite))
                qDebug()<<"recive file open fialed!";

            isFileHead = false;
            emit fileSize(totalSize);
        }
    }else {
        qint64 len = file.write(buf);
        recvSize += len;
        emit reciveSize(recvSize);
//        qDebug()<<recvSize;
        if(recvSize == totalSize){
            emit downLoadFinished();
            file.close();
            socket->disconnectFromHost();
            socket->close();
        }
    }
}

void FileTransferThread::handleNetDisconnected()
{
    if(file.isOpen())
        file.close();

    if(tcpSocket->state() == QTcpSocket::ConnectedState){
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
    }
}

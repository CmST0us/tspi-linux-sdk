#include "tfcardthread.h"

TFCardThread::TFCardThread()
{

}

void TFCardThread::run()
{
    while(checkFlag){
        checkTFCard();
        msleep(500);
    }
}

void TFCardThread::checkTFCard()
{
    QDir dir("/sys/bus/mmc/devices/");

    if(dir.exists()){
        dir.setFilter(QDir::Dirs);
        for(QFileInfo info : dir.entryInfoList()){
            if(info.fileName() == "." || info.fileName() == "..")
                continue;

//            qDebug()<<info.fileName();
            QFile mmcFile(info.filePath()+"/type");
            mmcFile.open(QIODevice::ReadOnly);
            QString type = mmcFile.readAll();
            if(type == "SD\n"){
                //success
                result = true;
                emit getTFCardState(result);
                return;
            }
        }

        result = false;

    }else{
        //faild
        result = false;
    }

    emit getTFCardState(result);
}

#include "adcthread.h"

AdcThread::AdcThread()
{

}

void AdcThread::run()
{
    while (readFlag) {
        readeAllAdcValue();
        msleep(100);
    }
}

void AdcThread::readeAllAdcValue()
{
    QDir dir(ADC_PATH);
    if(!dir.exists()){
        QDir allWinerDir(GPADC_PATH);
        if(!allWinerDir.exists()){
            qDebug()<<"not found any adc group!";
        }else{
            QFile file(QString(GPADC_PATH) + "data");
            file.open(QIODevice::ReadOnly);
            QString value = file.readAll();
            emit getAdcValue(value, "GPADC");
            file.close();
        }
    }else{
        QFileInfoList list = dir.entryInfoList();
        foreach (QFileInfo groups, list) {
            if(groups.fileName().startsWith("."))
                continue;

            QDir chnnels(QString(ADC_PATH + groups.fileName()));
            QFileInfoList chnnelList = chnnels.entryInfoList();
            foreach (QFileInfo chnnel, chnnelList) {
                if(chnnel.fileName().endsWith("raw")){
//                    qDebug()<<chnnel.filePath();
//                    qDebug()<<chnnel.fileName();
                    QFile file(chnnel.filePath());
                    file.open(QIODevice::ReadOnly);
                    QString value = file.readAll();
                    emit getAdcValue(value, groups.fileName()+chnnel.fileName());
                }
            }
        }
    }
}

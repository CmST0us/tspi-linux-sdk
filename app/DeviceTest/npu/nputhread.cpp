#include "nputhread.h"

NPUThread::NPUThread()
{

}

void NPUThread::run()
{
    msleep(200);
    checkPCIdev();
}

void NPUThread::checkPCIdev()
{
    QString pciDev = common.execLinuxCmd("lspci");
    if( pciDev == "" ){
        qDebug()<<"PCI connect failed!";
        this->connectState = false;
    }else{
        this->connectState = true;
    }

    emit checkConnect(connectState);

    if(connectState){
        this->runNPUDemo();
    }
}

void NPUThread::runNPUDemo()
{
    //rp_test/rockx_carplate_demo/rockx_carplate /rp_test/rockx_carplate_demo/carplate_det_test1.jpg
    QString cmd = "rp_test/rockx_carplate_demo/rockx_carplate /rp_test/rockx_carplate_demo/carplate_det_test1.jpg > /tmp/npu_run.log";

    ::system(cmd.toLocal8Bit());

    QFile npuRunLog("/tmp/npu_run.log");
    if(!npuRunLog.exists()){
        emit runDemoState(false, "");
        return;
    }

    npuRunLog.open(QIODevice::ReadOnly);
    QString strResult = npuRunLog.readAll();

    if(strResult.indexOf("苏JAY888") >= 0){
        emit runDemoState(true, strResult);
    }else{
        emit runDemoState(false, strResult);
    }

    npuRunLog.close();
}

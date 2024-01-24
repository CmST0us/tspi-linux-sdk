#include "stressthread.h"

#define CPU_TEMP_PATH "/sys/devices/virtual/thermal/thermal_zone0/temp"
#define CPU_FREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq"

#define DDR_FREQ_PATH "/sys/kernel/debug/clk/clk_summary"

StressThread::StressThread()
{

}

void StressThread::run()
{
    while(testStart){
        this->getCpu();
        this->getDDR();
        msleep(1000);
    }
}

void StressThread::getCpu()
{
    QString cpuFreq = this->getCpuFreq();
    QString cpuTemp = this->getCpuTemp();
    QString cpuLoad = this->getCpuLoad();

    emit getCpuInfo(cpuLoad, cpuFreq, cpuTemp);
}

QString StressThread::getCpuFreq()
{
    QFile freqFile(CPU_FREQ_PATH);
    if(freqFile.exists()){
        freqFile.open(QIODevice::ReadOnly);
        QString freq = freqFile.readAll();
        double freq_double = freq.toDouble()/1000;
        freqFile.close();
        return QString::number(freq_double, '.', 3) + "M";
    }
    return "获取失败";
}

QString StressThread::getCpuTemp()
{
    QFile tempFile(CPU_TEMP_PATH);
    if(tempFile.exists()){
        tempFile.open(QIODevice::ReadOnly);
        QString temp = tempFile.readAll();
        double temp_double = temp.toDouble()/1000;
        tempFile.close();
        return QString::number(temp_double, '.', 3) + "C";
    }
    return "获取失败";
}

QString StressThread::getCpuLoad()
{
    QString cpuTxt = common.execLinuxCmd("busybox top -n 1");
//    qDebug()<<cpuTxt;
    QString cpuLoadTxt = cpuTxt.split("\n").at(1);
    QString cpuUsrLoad = cpuLoadTxt.replace(QRegExp("\\s{1,}"), " ").split(" ").at(1);
    QString cpuSysLoad = cpuLoadTxt.replace(QRegExp("\\s{1,}"), " ").split(" ").at(3);

    cpuUsrLoad = cpuUsrLoad.replace("%", "");
    cpuSysLoad = cpuSysLoad.replace("%", "");


    return QString("user: "+cpuUsrLoad+"%"+", "+
                   "sys: "+ cpuSysLoad+"%");
}

void StressThread::getDDR()
{
    QString ddrfreq = this->getDDRFreq();
    QString ddrload = this->getDDRLoad();
    emit getDdrInfo(ddrload, ddrfreq);
}

QString StressThread::getDDRFreq()
{
//    QFile freqFile(DDR_FREQ_PATH);
//    if(freqFile.exists()){
//        freqFile.open(QIODevice::ReadOnly);
//        QString freqStr = freqFile.readAll();
//        foreach (QString freq, freqStr.split("\n")) {
//            freq = freq.replace(QRegExp("\\s{1,}"), " ");
//            if(freq.startsWith("sclk_ddrc")){
//                freqFile.close();
//                return freq.split(" ").at(3);
//            }
//        }
//    }
    //rk3399 is 856M
    return "856M";
}

QString StressThread::getDDRLoad()
{
    QString detial = common.execLinuxCmd("cat /proc/meminfo");
    QStringList strlist = detial.split("\n");
    QString DDRSize;
    QString DDRFree;
    foreach (QString str, strlist){
        if (str.startsWith("MemTotal")){
            DDRSize = str.replace(QRegExp("\\s{1,}"), " ").split(" ").at(1);
        }

        if (str.startsWith("MemFree")){
            DDRFree = str.replace(QRegExp("\\s{1,}"), " ").split(" ").at(1);
        }
    }

    double ddrsize = DDRSize.toDouble();
    double ddrfree = DDRFree.toDouble();

    double ddrload = 1 - (ddrfree/ddrsize);
    return QString::number(ddrload*100, '.', 2) + "%";
}

#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include <memory>
#include <thread>

#include "logger/log.h"
#include "rkaiq_adpcc.h"
#include "rkaiq_ae.h"
#include "rkaiq_agamma.h"
#include "rkaiq_ahdr.h"
#include "rkaiq_anr.h"
#include "rkaiq_awb.h"
#include "rkaiq_ccm.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_cproc.h"
#include "rkaiq_dehaze.h"
#include "rkaiq_engine.h"
#include "rkaiq_imgproc.h"
#include "rkaiq_sharp.h"
#include "rkaiq_sysctl.h"

class RKAiqToolManager
{
  public:
    RKAiqToolManager();
    virtual ~RKAiqToolManager();
    int AeIoCtrl(int id, void* data, int size);
    int ImgProcIoCtrl(int id, void* data, int size);
    int AnrIoCtrl(int id, void* data, int size);
    int SharpIoCtrl(int id, void* data, int size);
    int SysCtlIoCtrl(int id, void* data, int size);
    int AHDRIoCtrl(int id, void* data, int size);
    int AGamamIoCtrl(int id, void* data, int size);
    int ADPCCIoCtrl(int id, void* data, int size);
    int DEHAZEIoCtrl(int id, void* data, int size);
    int RkMediaCtrl(int id, void* data, int size);
    int CCMIoCtrl(int id, void* data, int size);
    int AWBIoCtrl(int id, void* data, int size);
    int CPROCIoCtrl(int id, void* data, int size);
    int IoCtrl(int id, void* data, int size);
    void SaveExit();

  private:
    std::string iqfiles_path_;
    std::string sensor_name_;
    std::thread* rkaiq_engine_thread_;
    static int thread_quit_;
    std::unique_ptr<RKAiqEngine> engine_;
    std::unique_ptr<RKAiqToolImgProc> imgproc_;
    std::unique_ptr<RKAiqToolAE> ae_;
    std::unique_ptr<RKAiqToolANR> anr_;
    std::unique_ptr<RKAiqToolSharp> asharp_;
    std::unique_ptr<RKAiqToolSysCtl> sysctl_;
    std::unique_ptr<RKAiqToolAHDR> ahdr_;
    std::unique_ptr<RKAiqToolADPCC> dpcc_;
    std::unique_ptr<RKAiqToolAGamma> gamma_;
    std::unique_ptr<RKAiqToolDehaze> dehaze_;
    std::unique_ptr<RKAiqToolCCM> ccm_;
    std::unique_ptr<RKAiqToolAWB> awb_;
    std::unique_ptr<RKAiqToolCPROC> cproc_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_

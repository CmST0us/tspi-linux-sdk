#include "rkaiq_manager.h"

#include "image.h"
#include "media_config.h"
#include "rkaiq_media.h"
#include "rtsp_server.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_device_id;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern std::shared_ptr<easymedia::Flow> g_video_rtsp_flow;
extern std::shared_ptr<easymedia::Flow> g_video_enc_flow;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;
extern std::string iqfile;
extern std::string g_sensor_name;

RKAiqToolManager::RKAiqToolManager()
{
    imgproc_.reset(new RKAiqToolImgProc());
    ae_.reset(new RKAiqToolAE());
    anr_.reset(new RKAiqToolANR());
    asharp_.reset(new RKAiqToolSharp());
    sysctl_.reset(new RKAiqToolSysCtl());
    ahdr_.reset(new RKAiqToolAHDR());
    dpcc_.reset(new RKAiqToolADPCC());
    gamma_.reset(new RKAiqToolAGamma());
    dehaze_.reset(new RKAiqToolDehaze());
    ccm_.reset(new RKAiqToolCCM());
    awb_.reset(new RKAiqToolAWB());
    cproc_.reset(new RKAiqToolCPROC());

    rk_aiq_ver_info_t vers;
    sysctl_->GetVersionInfo(&vers, ENUM_ID_SYSCTL_GETVERSIONINFO);
    LOG_ERROR("vers aiq_ver %s iq_parser_ver %s\n", vers.aiq_ver, vers.iq_parser_ver);
}

RKAiqToolManager::~RKAiqToolManager()
{
    ae_.reset(nullptr);
    anr_.reset(nullptr);
    asharp_.reset(nullptr);
    sysctl_.reset(nullptr);
    ahdr_.reset(nullptr);
    dpcc_.reset(nullptr);
    gamma_.reset(nullptr);
    dehaze_.reset(nullptr);
    ccm_.reset(nullptr);
    awb_.reset(nullptr);
    cproc_.reset(nullptr);
    imgproc_.reset(nullptr);
    engine_.reset(nullptr);
}

void RKAiqToolManager::SaveExit()
{
}

int RKAiqToolManager::IoCtrl(int id, void* data, int size)
{
    int retValue = 0;

    LOG_INFO("IoCtrl id: 0x%x\n", id);
    if (id > ENUM_ID_AE_START && id < ENUM_ID_AE_END) {
        retValue = AeIoCtrl(id, data, size);
    } else if (id > ENUM_ID_IMGPROC_START && id < ENUM_ID_IMGPROC_END) {
        retValue = ImgProcIoCtrl(id, data, size);
    } else if (id > ENUM_ID_ANR_START && id < ENUM_ID_ANR_END) {
        retValue = AnrIoCtrl(id, data, size);
    } else if (id > ENUM_ID_SHARP_START && id < ENUM_ID_SHARP_END) {
        retValue = SharpIoCtrl(id, data, size);
    } else if (id > ENUM_ID_SYSCTL_START && id < ENUM_ID_SYSCTL_END) {
        retValue = SysCtlIoCtrl(id, data, size);
    } else if (id > ENUM_ID_AHDR_START && id < ENUM_ID_AHDR_END) {
        retValue = AHDRIoCtrl(id, data, size);
    } else if (id > ENUM_ID_AGAMMA_START && id < ENUM_ID_AGAMMA_END) {
        retValue = AGamamIoCtrl(id, data, size);
    } else if (id > ENUM_ID_ADPCC_START && id < ENUM_ID_ADPCC_END) {
        retValue = ADPCCIoCtrl(id, data, size);
    } else if (id > ENUM_ID_DEHAZE_START && id < ENUM_ID_DEHAZE_END) {
        retValue = DEHAZEIoCtrl(id, data, size);
    } else if (id > ENUM_ID_ACCM_START && id < ENUM_ID_ACCM_END) {
        retValue = CCMIoCtrl(id, data, size);
    } else if (id > ENUM_ID_AWB_START && id < ENUM_ID_AWB_END) {
        retValue = AWBIoCtrl(id, data, size);
    } else if (id > ENUM_ID_CPROC_START && id < ENUM_ID_CPROC_END) {
        retValue = CPROCIoCtrl(id, data, size);
    }
    LOG_INFO("IoCtrl id: 0x%x exit\n", id);
    LOG_INFO("IoCtrl retValue: %d \n", retValue);
    return retValue;
}

int RKAiqToolManager::AeIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("AeIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_AE_SETEXPSWATTR:
            CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
            retValue = ae_->setExpSwAttr(*(Uapi_ExpSwAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETEXPSWATTR:
            CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
            retValue = ae_->getExpSwAttr((Uapi_ExpSwAttr_t*)data, id);
            break;
        case ENUM_ID_AE_SETLINAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            retValue = ae_->setLinAeDayRouteAttr(*(Uapi_LinAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETLINAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            retValue = ae_->getLinAeDayRouteAttr((Uapi_LinAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_SETLINAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            retValue = ae_->setLinAeNightRouteAttr(*(Uapi_LinAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETLINAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            retValue = ae_->getLinAeNightRouteAttr((Uapi_LinAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_SETHDRAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            retValue = ae_->setHdrAeDayRouteAttr(*(Uapi_HdrAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETHDRAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            retValue = ae_->getHdrAeDayRouteAttr((Uapi_HdrAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_SETHDRAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            retValue = ae_->setHdrAeNightRouteAttr(*(Uapi_HdrAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETHDRAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            retValue = ae_->getHdrAeNightRouteAttr((Uapi_HdrAeRouteAttr_t*)data, id);
            break;
        case ENUM_ID_AE_QUERYEXPRESINFO:
            CHECK_PARAM_SIZE(Uapi_ExpQueryInfo_t, size);
            retValue = ae_->queryExpResInfo((Uapi_ExpQueryInfo_t*)data, id);
            break;
        case ENUM_ID_AE_SETLINEXPATTR:
            CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
            retValue = ae_->setLinExpAttr(*(Uapi_LinExpAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETLINEXPATTR:
            CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
            retValue = ae_->getLinExpAttr((Uapi_LinExpAttr_t*)data, id);
            break;
        case ENUM_ID_AE_SETHDREXPATTR:
            CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
            retValue = ae_->setHdrExpAttr(*(Uapi_HdrExpAttr_t*)data, id);
            break;
        case ENUM_ID_AE_GETHDREXPATTR:
            CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
            retValue = ae_->getHdrExpAttr((Uapi_HdrExpAttr_t*)data, id);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::ImgProcIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("ImgProcIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_IMGPROC_SETGRAYMODE:
            CHECK_PARAM_SIZE(rk_aiq_gray_mode_t, size);
            retValue = imgproc_->SetGrayMode(*(rk_aiq_gray_mode_t*)data, id);
            break;
        case ENUM_ID_IMGPROC_GETGRAYMODE:
            CHECK_PARAM_SIZE(rk_aiq_gray_mode_t, size);
            retValue = imgproc_->GetGrayMode((rk_aiq_gray_mode_t*)data, id);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::AnrIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("AnrIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_ANR_SETBAYERNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
            retValue = anr_->SetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_SETMFNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
            retValue = anr_->SetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_SETUVNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
            retValue = anr_->SetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_SETYNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
            retValue = anr_->SetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_GETBAYERNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
            retValue = anr_->GetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_GETMFNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
            retValue = anr_->GetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_GETUVNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
            retValue = anr_->GetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_GETYNRATTR: {
            CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
            retValue = anr_->GetIQPara(data, id);
            break;
        }
        case ENUM_ID_ANR_SETATTRIB:
            CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
            retValue = anr_->SetAttrib((rk_aiq_nr_attrib_t*)data, id);
            break;
        case ENUM_ID_ANR_GETATTRIB:
            CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
            retValue = anr_->GetAttrib((rk_aiq_nr_attrib_t*)data, id);
            break;
        case ENUM_ID_ANR_SETLUMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->SetLumaSFStrength(*(float*)data, id);
            break;
        case ENUM_ID_ANR_SETLUMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->SetLumaTFStrength(*(float*)data, id);
            break;
        case ENUM_ID_ANR_GETLUMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->GetLumaSFStrength((float*)data, id);
            break;
        case ENUM_ID_ANR_GETLUMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->GetLumaTFStrength((float*)data, id);
            break;
        case ENUM_ID_ANR_SETCHROMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->SetChromaSFStrength(*(float*)data, id);
            break;
        case ENUM_ID_ANR_SETCHROMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->SetChromaTFStrength(*(float*)data, id);
            break;
        case ENUM_ID_ANR_GETCHROMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->GetChromaSFStrength((float*)data, id);
            break;
        case ENUM_ID_ANR_GETCHROMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->GetChromaTFStrength((float*)data, id);
            break;
        case ENUM_ID_ANR_SETRAWNRSFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->SetRawnrSFStrength(*(float*)data, id);
            break;
        case ENUM_ID_ANR_GETRAWNRSFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = anr_->GetRawnrSFStrength((float*)data, id);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::SharpIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("SharpIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_SHARP_SET_ATTR:
            CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
            retValue = asharp_->SetAttrib((rk_aiq_sharp_attrib_t*)data, id);
            break;
        case ENUM_ID_SHARP_GET_ATTR:
            CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
            retValue = asharp_->GetAttrib((rk_aiq_sharp_attrib_t*)data, id);
            break;
        case ENUM_ID_SHARP_SET_IQPARA:
            CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
            retValue = asharp_->SetIQPara(data, id);
            break;
        case ENUM_ID_SHARP_GET_IQPARA:
            CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
            retValue = asharp_->GetIQPara(data, id);
            break;
        case ENUM_ID_SHARP_SET_EF_IQPARA:
            CHECK_PARAM_SIZE(CalibDb_EdgeFilter_t, size);
            retValue = asharp_->SetIQPara(data, id);
            break;
        case ENUM_ID_SHARP_GET_EF_IQPARA:
            CHECK_PARAM_SIZE(CalibDb_EdgeFilter_t, size);
            retValue = asharp_->GetIQPara(data, id);
            break;
        case ENUM_ID_SHARP_SET_STRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = asharp_->SetStrength(*(float*)data, id);
            break;
        case ENUM_ID_SHARP_GET_STRENGTH:
            CHECK_PARAM_SIZE(float, size);
            retValue = asharp_->GetStrength((float*)data, id);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::SysCtlIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("SysCtlIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_SYSCTL_GETVERSIONINFO:
            retValue = sysctl_->GetVersionInfo((rk_aiq_ver_info_t*)data, id);
            break;
        case ENUM_ID_SYSCTL_SETCPSLTCFG:
            retValue = sysctl_->SetCpsLtCfg((rk_aiq_cpsl_cfg_t*)data, id);
            break;
        case ENUM_ID_SYSCTL_GETCPSLTINFO:
            retValue = sysctl_->GetCpsLtInfo((rk_aiq_cpsl_info_t*)data, id);
            break;
        case ENUM_ID_SYSCTL_QUERYCPSLTCAP:
            retValue = sysctl_->QueryCpsLtCap((rk_aiq_cpsl_cap_t*)data, id);
            break;
        case ENUM_ID_SYSCTL_SETWORKINGMODE:
            retValue = sysctl_->SetWorkingModeDyn(*(rk_aiq_working_mode_t*)data, id);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::AHDRIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("AHDRIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_AHDR_SETATTRIB: {
            CHECK_PARAM_SIZE(ahdr_attrib_t, size);
            retValue = ahdr_->SetAttrib(*(ahdr_attrib_t*)data, id);
        } break;
        case ENUM_ID_AHDR_GETATTRIB: {
            CHECK_PARAM_SIZE(ahdr_attrib_t, size);
            retValue = ahdr_->GetAttrib((ahdr_attrib_t*)data, id);
        } break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::AGamamIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("AGamamIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_AGAMMA_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_gamma_attrib_t, size);
            retValue = gamma_->SetAttrib(*(rk_aiq_gamma_attrib_t*)data, id);
        } break;
        case ENUM_ID_AGAMMA_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_gamma_attrib_t, size);
            retValue = gamma_->GetAttrib((rk_aiq_gamma_attrib_t*)data, id);
        } break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::ADPCCIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("ADPCCIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_ADPCC_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_dpcc_attrib_t, size);
            retValue = dpcc_->SetAttrib((rk_aiq_dpcc_attrib_t*)data, id);
        } break;
        case ENUM_ID_ADPCC_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_dpcc_attrib_t, size);
            retValue = dpcc_->GetAttrib((rk_aiq_dpcc_attrib_t*)data, id);
        } break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::DEHAZEIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("DEHAZEIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_DEHAZE_SETATTRIB: {
            CHECK_PARAM_SIZE(adehaze_sw_t, size);
            retValue = dehaze_->SetAttrib(*(adehaze_sw_t*)data, id);
        } break;
        case ENUM_ID_DEHAZE_GETATTRIB: {
            CHECK_PARAM_SIZE(adehaze_sw_t, size);
            retValue = dehaze_->GetAttrib((adehaze_sw_t*)data, id);
        } break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            retValue = 1;
            break;
    }
    return retValue;
}

int RKAiqToolManager::CCMIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("CCMIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_ACCM_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_attrib_t, size);
            retValue = ccm_->SetAttrib(*(rk_aiq_ccm_attrib_t*)data, id);
        } break;
        case ENUM_ID_ACCM_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_attrib_t, size);
            retValue = ccm_->GetAttrib((rk_aiq_ccm_attrib_t*)data, id);
        } break;
        case ENUM_ID_ACCM_QUERYCCMINFO: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_querry_info_t, size);
            retValue = ccm_->QueryCCMInfo((rk_aiq_ccm_querry_info_t*)data, id);
        } break;
        default:
            retValue = 1;
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return retValue;
}

int RKAiqToolManager::AWBIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("AWBIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_AWB_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_wb_attrib_t, size);
            retValue = awb_->SetAttrib(*(rk_aiq_wb_attrib_t*)data, id);
        } break;
        case ENUM_ID_AWB_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_wb_attrib_t, size);
            retValue = awb_->GetAttrib((rk_aiq_wb_attrib_t*)data, id);
        } break;
        case ENUM_ID_AWB_QUERYWBINFO: {
            CHECK_PARAM_SIZE(rk_aiq_wb_querry_info_t, size);
            retValue = awb_->QueryWBInfo((rk_aiq_wb_querry_info_t*)data, id);
        } break;
        default:
            retValue = 1;
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return retValue;
}

int RKAiqToolManager::CPROCIoCtrl(int id, void* data, int size)
{
    int retValue = 0;
    LOG_INFO("CPROCIoCtrl id: 0x%x\n", id);
    switch (id) {
        case ENUM_ID_CPROC_SETATTRIB: {
            CHECK_PARAM_SIZE(acp_attrib_t, size);
            retValue = cproc_->SetAttrib(*(acp_attrib_t*)data, id);
        } break;
        case ENUM_ID_CPROC_GETATTRIB: {
            CHECK_PARAM_SIZE(acp_attrib_t, size);
            retValue = cproc_->GetAttrib((acp_attrib_t*)data, id);
        } break;
        default:
            retValue = 1;
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return retValue;
}

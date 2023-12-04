#ifndef _RK_AIQ_USER_API_FUN_MAP_H_
#define _RK_AIQ_USER_API_FUN_MAP_H_

struct FunMap map[] = {
#if 1
    //rk_aiq_upi_imgproc_ipc
    {"rk_aiq_uapi_setExpMode", &rk_aiq_uapi_setExpMode_ipc},
    {"rk_aiq_uapi_getExpMode", &rk_aiq_uapi_getExpMode_ipc},
    {"rk_aiq_uapi_setAeMode", &rk_aiq_uapi_setAeMode_ipc},
    {"rk_aiq_uapi_getAeMode", &rk_aiq_uapi_getAeMode_ipc},
    {"rk_aiq_uapi_setExpGainRange", &rk_aiq_uapi_setExpGainRange_ipc},
    {"rk_aiq_uapi_getExpGainRange", &rk_aiq_uapi_getExpGainRange_ipc},
    {"rk_aiq_uapi_setExpTimeRange", &rk_aiq_uapi_setExpTimeRange_ipc},
    {"rk_aiq_uapi_getExpTimeRange", &rk_aiq_uapi_getExpTimeRange_ipc},
    {"rk_aiq_uapi_setHLCMode", &rk_aiq_uapi_setHLCMode_ipc},
    {"rk_aiq_uapi_setLExpMode", &rk_aiq_uapi_setLExpMode_ipc},
    {"rk_aiq_uapi_getLExpMode", &rk_aiq_uapi_getLExpMode_ipc},
    {"rk_aiq_uapi_setMLExp", &rk_aiq_uapi_setMLExp_ipc},
    {"rk_aiq_uapi_getMLExp", &rk_aiq_uapi_getMLExp_ipc},
    {"rk_aiq_uapi_setAntiFlickerMode", &rk_aiq_uapi_setAntiFlickerMode_ipc},
    {"rk_aiq_uapi_getAntiFlickerMode", &rk_aiq_uapi_getAntiFlickerMode_ipc},
    {"rk_aiq_uapi_setExpPwrLineFreqMode", &rk_aiq_uapi_setExpPwrLineFreqMode_ipc},
    {"rk_aiq_uapi_getExpPwrLineFreqMode", &rk_aiq_uapi_getExpPwrLineFreqMode_ipc},
    {"rk_aiq_uapi_setDayNSwMode", &rk_aiq_uapi_setDayNSwMode_ipc},
    {"rk_aiq_uapi_getDayNSwMode", &rk_aiq_uapi_getDayNSwMode_ipc},
    {"rk_aiq_uapi_setMDNScene", &rk_aiq_uapi_setMDNScene_ipc},
    {"rk_aiq_uapi_getMDNScene", &rk_aiq_uapi_getMDNScene_ipc},
    {"rk_aiq_uapi_setADNSens", &rk_aiq_uapi_setADNSens_ipc},
    {"rk_aiq_uapi_getADNSens", &rk_aiq_uapi_getADNSens_ipc},
    {"rk_aiq_uapi_setFLightMode", &rk_aiq_uapi_setFLightMode_ipc},
    {"rk_aiq_uapi_getFLightMode", &rk_aiq_uapi_getFLightMode_ipc},
    {"rk_aiq_uapi_setMFLight", &rk_aiq_uapi_setMFLight_ipc},
    {"rk_aiq_uapi_getMFLight", &rk_aiq_uapi_getMFLight_ipc},
    {"rk_aiq_uapi_setWBMode", &rk_aiq_uapi_setWBMode_ipc},
    {"rk_aiq_uapi_getWBMode", &rk_aiq_uapi_getWBMode_ipc},
    {"rk_aiq_uapi_lockAWB", &rk_aiq_uapi_lockAWB_ipc},
    {"rk_aiq_uapi_unlockAWB", &rk_aiq_uapi_unlockAWB_ipc},
    //{"rk_aiq_uapi_setAWBRange", &rk_aiq_uapi_setAWBRange_ipc},
    //{"rk_aiq_uapi_getAWBRange", &rk_aiq_uapi_getAWBRange_ipc},
    {"rk_aiq_uapi_setMWBScene", &rk_aiq_uapi_setMWBScene_ipc},
    {"rk_aiq_uapi_getMWBScene", &rk_aiq_uapi_getMWBScene_ipc},
    {"rk_aiq_uapi_setMWBGain", &rk_aiq_uapi_setMWBGain_ipc},
    {"rk_aiq_uapi_getMWBGain", &rk_aiq_uapi_getMWBGain_ipc},
    {"rk_aiq_uapi_setMWBCT", &rk_aiq_uapi_setMWBCT_ipc},
    {"rk_aiq_uapi_getMWBCT", &rk_aiq_uapi_getMWBCT_ipc},
    {"rk_aiq_uapi_setCrSuppsn", &rk_aiq_uapi_setCrSuppsn_ipc},
    {"rk_aiq_uapi_getCrSuppsn", &rk_aiq_uapi_getCrSuppsn_ipc},
    {"rk_aiq_uapi_setFocusMode"    ,&rk_aiq_uapi_setFocusMode_ipc      },
    {"rk_aiq_uapi_getFocusMode"    ,&rk_aiq_uapi_getFocusMode_ipc      },
    {"rk_aiq_uapi_setMinFocusDis",&rk_aiq_uapi_setMinFocusDis_ipc  },
    {"rk_aiq_uapi_getMinFocusDis"    ,&rk_aiq_uapi_getMinFocusDis_ipc  },
    {"rk_aiq_uapi_setOpZoomRange"    ,&rk_aiq_uapi_setOpZoomRange_ipc  } ,                    
    {"rk_aiq_uapi_getOpZoomRange"    ,&rk_aiq_uapi_getOpZoomRange_ipc  },                     
    {"rk_aiq_uapi_setOpZoomSpeed"    ,&rk_aiq_uapi_setOpZoomSpeed_ipc  },
    {"rk_aiq_uapi_getOpZoomSpeed"    ,&rk_aiq_uapi_getOpZoomSpeed_ipc  },
    {"rk_aiq_uapi_setHDRMode"    ,&rk_aiq_uapi_setHDRMode_ipc          },
    {"rk_aiq_uapi_getHDRMode"    ,&rk_aiq_uapi_getHDRMode_ipc          },
    {"rk_aiq_uapi_setMHDRStrth"    ,&rk_aiq_uapi_setMHDRStrth_ipc      },
    {"rk_aiq_uapi_getMHDRStrth"    ,&rk_aiq_uapi_getMHDRStrth_ipc      },
    {"rk_aiq_uapi_setNRMode"    ,&rk_aiq_uapi_setNRMode_ipc            },
    {"rk_aiq_uapi_getNRMode"    ,&rk_aiq_uapi_getNRMode_ipc            },
    {"rk_aiq_uapi_setANRStrth"    ,&rk_aiq_uapi_setANRStrth_ipc        },
    {"rk_aiq_uapi_getANRStrth"    ,&rk_aiq_uapi_getANRStrth_ipc        },
    {"rk_aiq_uapi_setMSpaNRStrth"    ,&rk_aiq_uapi_setMSpaNRStrth_ipc  },
    {"rk_aiq_uapi_getMSpaNRStrth"    ,&rk_aiq_uapi_getMSpaNRStrth_ipc  },
    {"rk_aiq_uapi_setMTNRStrth"    ,&rk_aiq_uapi_setMTNRStrth_ipc      },
    {"rk_aiq_uapi_getMTNRStrth"    ,&rk_aiq_uapi_getMTNRStrth_ipc      },
    {"rk_aiq_uapi_setDhzMode"    ,&rk_aiq_uapi_setDhzMode_ipc          },
    {"rk_aiq_uapi_getDhzMode"    ,&rk_aiq_uapi_getDhzMode_ipc          },
    {"rk_aiq_uapi_setMDhzStrth"    ,&rk_aiq_uapi_setMDhzStrth_ipc      },
    {"rk_aiq_uapi_getMDhzStrth"    ,&rk_aiq_uapi_getMDhzStrth_ipc      },
    {"rk_aiq_uapi_setContrast"    ,&rk_aiq_uapi_setContrast_ipc        },
    {"rk_aiq_uapi_getContrast"    ,&rk_aiq_uapi_getContrast_ipc        },
    {"rk_aiq_uapi_setBrightness"    ,&rk_aiq_uapi_setBrightness_ipc    },
    {"rk_aiq_uapi_getBrightness"    ,&rk_aiq_uapi_getBrightness_ipc    },
    {"rk_aiq_uapi_setSaturation"    ,&rk_aiq_uapi_setSaturation_ipc    },
    {"rk_aiq_uapi_getSaturation"    ,&rk_aiq_uapi_getSaturation_ipc    },
    {"rk_aiq_uapi_setSharpness"    ,&rk_aiq_uapi_setSharpness_ipc      },
    {"rk_aiq_uapi_getSharpness"    ,&rk_aiq_uapi_getSharpness_ipc      },
    {"rk_aiq_uapi_setHue"    ,&rk_aiq_uapi_setHue_ipc      },
    {"rk_aiq_uapi_getHue"    ,&rk_aiq_uapi_getHue_ipc      },

     //rk_aiq_user_api_a3dlut_ipc
    {"rk_aiq_user_api_a3dlut_SetAttrib" , &rk_aiq_user_api_a3dlut_SetAttrib_ipc            },
    {"rk_aiq_user_api_a3dlut_GetAttrib" , &rk_aiq_user_api_a3dlut_GetAttrib_ipc            },
    {"rk_aiq_user_api_a3dlut_Query3dlutInfo" , &rk_aiq_user_api_a3dlut_Query3dlutInfo_ipc  },

    //rk_aiq_user_api_ablc_ipc
    {"rk_aiq_user_api_ablc_SetAttrib" ,  &rk_aiq_user_api_ablc_SetAttrib_ipc},
    {"rk_aiq_user_api_ablc_GetAttrib" ,   &rk_aiq_user_api_ablc_GetAttrib_ipc  },

   //rk_aiq_user_api_accm_ipc
    {"rk_aiq_user_api_accm_SetAttrib"  ,   &rk_aiq_user_api_accm_SetAttrib_ipc},
    {"rk_aiq_user_api_accm_GetAttrib"  ,   &rk_aiq_user_api_accm_GetAttrib_ipc},
    {"rk_aiq_user_api_accm_QueryCcmInfo" , &rk_aiq_user_api_accm_GetAttrib_ipc},
     
   //rk_aiq_user_api_adebayer_ipc
    {"rk_aiq_user_api_adebayer_SetAttrib", &rk_aiq_user_api_adebayer_SetAttrib_ipc},
    //rk_aiq_user_api_adehaze_ipc
    {"rk_aiq_user_api_adehaze_setSwAttrib"  ,   &rk_aiq_user_api_adehaze_setSwAttrib_ipc},
    {"rk_aiq_user_api_adehaze_getSwAttrib" , &rk_aiq_user_api_adehaze_getSwAttrib_ipc},

    //rk_aiq_user_api_adpcc_ipc
    {"rk_aiq_user_api_adpcc_SetAttrib"  ,   &rk_aiq_user_api_adpcc_SetAttrib_ipc},
    {"rk_aiq_user_api_adpcc_GetAttrib" , &rk_aiq_user_api_adpcc_GetAttrib_ipc},
    
   //rk_aiq_user_api_ae_ipc
    {"rk_aiq_user_api_adpcc_SetAttrib"  ,   &rk_aiq_user_api_adpcc_SetAttrib_ipc},
    {"rk_aiq_user_api_adpcc_GetAttrib" , &rk_aiq_user_api_adpcc_GetAttrib_ipc},
    {"rk_aiq_user_api_ae_setExpSwAttr"  ,   &rk_aiq_user_api_ae_setExpSwAttr_ipc},
    {"rk_aiq_user_api_ae_getExpSwAttr"  ,   &rk_aiq_user_api_ae_getExpSwAttr_ipc},
    {"rk_aiq_user_api_ae_queryExpResInfo"  ,   &rk_aiq_user_api_ae_queryExpResInfo_ipc},
    {"rk_aiq_user_api_ae_setLinExpAttr"  ,   &rk_aiq_user_api_ae_setLinExpAttr_ipc},
    {"rk_aiq_user_api_ae_getLinExpAttr"  ,   &rk_aiq_user_api_ae_getLinExpAttr_ipc},
    {"rk_aiq_user_api_ae_setHdrExpAttr"  ,   &rk_aiq_user_api_ae_setHdrExpAttr_ipc},
    {"rk_aiq_user_api_ae_getHdrExpAttr"  ,   &rk_aiq_user_api_ae_getHdrExpAttr_ipc},
  //rk_aiq_user_api_agamma_ipc
    {"rk_aiq_user_api_agamma_SetAttrib"  ,   &rk_aiq_user_api_agamma_SetAttrib_ipc},
    {"rk_aiq_user_api_agamma_GetAttrib"  ,   &rk_aiq_user_api_agamma_GetAttrib_ipc},

    //rk_aiq_user_api_ahdr_ipc
    {"rk_aiq_user_api_ahdr_SetAttrib"  ,   &rk_aiq_user_api_ahdr_SetAttrib_ipc},
    //rk_aiq_user_api_alsc_ipc
    {"rk_aiq_user_api_alsc_SetAttrib"  ,   &rk_aiq_user_api_alsc_SetAttrib_ipc},
    {"rk_aiq_user_api_alsc_GetAttrib"  ,   &rk_aiq_user_api_alsc_GetAttrib_ipc},
    {"rk_aiq_user_api_alsc_QueryLscInfo"  ,   &rk_aiq_user_api_alsc_QueryLscInfo_ipc},
    //rk_aiq_user_api_anr_ipc
    {"rk_aiq_user_api_anr_SetAttrib"  ,   &rk_aiq_user_api_anr_SetAttrib_ipc},
    {"rk_aiq_user_api_anr_GetAttrib"  ,   &rk_aiq_user_api_anr_GetAttrib_ipc},

    //rk_aiq_user_api_asharp_ipc
    {"rk_aiq_user_api_asharp_SetAttrib"  ,   &rk_aiq_user_api_asharp_SetAttrib_ipc},
    {"rk_aiq_user_api_asharp_GetAttrib"  ,   &rk_aiq_user_api_asharp_GetAttrib_ipc},
    //rk_aiq_user_api_awb_ipc
    {"rk_aiq_user_api_awb_SetAttrib"  ,   &rk_aiq_user_api_awb_SetAttrib_ipc},
    {"rk_aiq_user_api_awb_GetAttrib"  ,   &rk_aiq_user_api_awb_GetAttrib_ipc},
    {"rk_aiq_user_api_awb_GetCCT"  ,   &rk_aiq_user_api_awb_GetCCT_ipc},
    {"rk_aiq_user_api_awb_QueryWBInfo"  ,   &rk_aiq_user_api_awb_QueryWBInfo_ipc},
#endif
    //rk_aiq_user_api_sysctl_ipc
    {"rk_aiq_uapi_sysctl_init"   ,&rk_aiq_uapi_sysctl_init_ipc},
    {"rk_aiq_uapi_sysctl_deinit"   ,&rk_aiq_uapi_sysctl_deinit_ipc},
    {"rk_aiq_uapi_sysctl_prepare"   ,&rk_aiq_uapi_sysctl_prepare_ipc},
    {"rk_aiq_uapi_sysctl_start"   ,&rk_aiq_uapi_sysctl_start_ipc},
    {"rk_aiq_uapi_sysctl_stop"   ,&rk_aiq_uapi_sysctl_stop_ipc},
    {"rk_aiq_uapi_sysctl_getStaticMetas "   ,&rk_aiq_uapi_sysctl_getStaticMetas_ipc},
    {"rk_aiq_uapi_sysctl_getMetaData"   ,&rk_aiq_uapi_sysctl_getMetaData_ipc},
    {"rk_aiq_uapi_sysctl_setModuleCtl"   ,&rk_aiq_uapi_sysctl_setModuleCtl_ipc},
    {"rk_aiq_uapi_sysctl_getModuleCtl"   ,&rk_aiq_uapi_sysctl_getModuleCtl_ipc},
    {"rk_aiq_uapi_sysctl_regLib"   ,&rk_aiq_uapi_sysctl_regLib_ipc},
    {"rk_aiq_uapi_sysctl_unRegLib"   ,&rk_aiq_uapi_sysctl_unRegLib_ipc},
    {"rk_aiq_uapi_sysctl_enableAxlib"   ,&rk_aiq_uapi_sysctl_enableAxlib_ipc},
    {"rk_aiq_uapi_sysctl_getAxlibStatus"   ,&rk_aiq_uapi_sysctl_getAxlibStatus_ipc},
    {"rk_aiq_uapi_sysctl_getEnabledAxlibCtx",&rk_aiq_uapi_sysctl_getEnabledAxlibCtx_ipc},

};

#endif

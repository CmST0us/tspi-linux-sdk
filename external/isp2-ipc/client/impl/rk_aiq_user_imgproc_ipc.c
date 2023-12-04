/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "config.h"
#include "rk_aiq_user_api_imgproc.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_uapi_imgproc_ptl.h"

/*
#define RKAIQ_IMGPROC_CHECK_RET(ret, format, ...) \
    if (ret) {    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
 \
        LOGE(format, ##__VA_ARGS__); \
        return 0;; \
    }
*/
/*
*****************************
*
* Desc: set exposure control mode
* Argument:
*   mode:  auto: auto exposure
*          manual: manual exposure
*****************************
*/
XCamReturn rk_aiq_uapi_setExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setExpMode);
}

XCamReturn rk_aiq_uapi_getExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    

   opMode_t *attr = mode;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getExpMode);  
}

/*
*****************************
*
* Desc: set auto exposure mode
* Argument:
*   mode:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setAeMode(const rk_aiq_sys_ctx_t* ctx, aeMode_t mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    aeMode_t attr;
    memcpy(&attr, &mode, sizeof(aeMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setAeMode);
}

XCamReturn rk_aiq_uapi_getAeMode(const rk_aiq_sys_ctx_t* ctx, aeMode_t *mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    aeMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getAeMode);  
}


/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*      exposure gain will be adjust between [gain->min, gain->max]；
*    manual exposure mode:
*      gain->min == gain->max
*
*****************************
*/
XCamReturn rk_aiq_uapi_setExpGainRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *gain)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    paRange_t *attr = gain;
    CLIENT_CALL_SET_AIQ_P(rk_aiq_uapi_setExpGainRange);
}

XCamReturn rk_aiq_uapi_getExpGainRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *gain)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    paRange_t *attr = gain;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getExpGainRange);  
}
/*
*****************************
*
* Desc: set exposure parameter
* Argument:
*    auto exposure mode:
*       exposure time will be adjust between [time->min, time->max]；
*    manual exposure mode:
*       exposure time will be set gain->min == gain->max;
*
*****************************
*/
XCamReturn rk_aiq_uapi_setExpTimeRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *time)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

   paRange_t *attr = time;
   CLIENT_CALL_SET_AIQ_P(rk_aiq_uapi_setExpGainRange);
}

XCamReturn rk_aiq_uapi_getExpTimeRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *time)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

   paRange_t *attr = time;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getExpTimeRange);
}

/*
**********************************************************
* Auto exposure advanced features
**********************************************************
*/
/*
*****************************
*
* Desc: blacklight compensation
* Argument:
*      on:  1  on
*           0  off
*      rect: blacklight compensation area
*
*****************************
*/
XCamReturn rk_aiq_uapi_setBLCMode(const rk_aiq_sys_ctx_t* ctx, bool on, aeMeasAreaType_t areaType)
{   
    DBG("enter %s, line=%d\n",__FUNCTION__,__LINE__);
    rk_aiq_uapi_setBLCMode_t  para; 
    para.sys_ctx = ctx;
    para.on = on;
    memcpy(&para.areaType, (void*)&areaType, sizeof(para.areaType)); 
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1);
    return para.returnvalue;
}

/*
*****************************
*
* Desc: highlight compensation
* Argument:
*      on:  1  on
*           0  off
*      rect: blacklight compensation area
*
*****************************
*/
XCamReturn rk_aiq_uapi_setHLCMode(const rk_aiq_sys_ctx_t* ctx, bool on)
{    
    DBG("enter %s, line=%d\n",__FUNCTION__,__LINE__);
    rk_aiq_uapi_setBLCMode_t  para; 
    para.sys_ctx = ctx;
    para.on = on;
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1);
    return 0;;

}
/*
*****************************
*
* Desc: set lowlight exposure mode
* Argument:
*    mode:
*       auto: auto lowlight mode
*       manual: manual lowlight mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setLExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));   
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setLExpMode);
}

XCamReturn rk_aiq_uapi_getLExpMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
   const rk_aiq_sys_ctx_t* sys_ctx = ctx;
   opMode_t *attr = mode;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getLExpMode);
}


/*
*****************************
*
* Desc: set manual lowlight exposure time ratio
* Argument:
*    ratio:  [1.0, 128.0]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMLExp(const rk_aiq_sys_ctx_t* ctx, unsigned int ratio)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int level = ratio;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMLExp);
}

XCamReturn rk_aiq_uapi_getMLExp(const rk_aiq_sys_ctx_t* ctx, unsigned int* ratio)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int* level = ratio;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getMLExp);
}

/*
*****************************
*
* Desc: set power line frequence
* Argument:
*    freq
*
*****************************
*/
XCamReturn rk_aiq_uapi_setAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t mode)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    antiFlickerMode_t attr;
    memcpy(&attr, &mode, sizeof(antiFlickerMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setAntiFlickerMode);
}

XCamReturn rk_aiq_uapi_getAntiFlickerMode(const rk_aiq_sys_ctx_t* ctx, antiFlickerMode_t *mode)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
   antiFlickerMode_t *attr = mode;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getAntiFlickerMode);
}

XCamReturn rk_aiq_uapi_setExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t freq)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    expPwrLineFreq_t attr;
    memcpy(&attr, &freq, sizeof(expPwrLineFreq_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setExpPwrLineFreqMode);
}

XCamReturn rk_aiq_uapi_getExpPwrLineFreqMode(const rk_aiq_sys_ctx_t* ctx, expPwrLineFreq_t *freq)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
   expPwrLineFreq_t *attr = freq;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getExpPwrLineFreqMode);
}



/*
*****************************
*
* Desc: set day night switch mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setDayNSwMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

  opMode_t attr;
  memcpy(&attr, &mode, sizeof(opMode_t));
  CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setDayNSwMode);
}


XCamReturn rk_aiq_uapi_getDayNSwMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    opMode_t *attr = mode;
   CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getDayNSwMode);
}

/*
*****************************
*
* Desc: set manual day night scene
*    this function is active for DayNSw is manual mode
* Argument:
*    scene
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMDNScene(const rk_aiq_sys_ctx_t* ctx, dayNightScene_t scene)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    dayNightScene_t attr;
    memcpy(&attr, &scene, sizeof(dayNightScene_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setMDNScene);
}

XCamReturn rk_aiq_uapi_getMDNScene(const rk_aiq_sys_ctx_t* ctx, dayNightScene_t *scene)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    dayNightScene_t *attr = scene;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getMDNScene);
}


/*
*****************************
*
* Desc: set auto day night switch sensitivity
*    this function is active for DayNSw is auto mode
* Argument:
*    level: [1, 3]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setADNSens(const rk_aiq_sys_ctx_t* ctx, unsigned    int level)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setADNSens);
}

XCamReturn rk_aiq_uapi_getADNSens(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getADNSens);
}

/*
*****************************
*
* Desc: set fill light mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFLightMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setFLightMode);
}

XCamReturn rk_aiq_uapi_getFLightMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getFLightMode);
}



/*
*****************************
*
* Desc: set maual fill light mode
* Argument:
*    on:  1: on
*         0: off
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMFLight(const rk_aiq_sys_ctx_t* ctx, bool on)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    bool level = on;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMFLight);
}

XCamReturn rk_aiq_uapi_getMFLight(const rk_aiq_sys_ctx_t* ctx, bool *on)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    bool *level = on;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getMFLight);
}



/*
**********************************************************
* White balance & Color
**********************************************************
*/

/*
*****************************
*
* Desc: set white balance mode
* Argument:
*   mode:  auto: auto white balance
*          manual: manual white balance
*****************************
*/
XCamReturn rk_aiq_uapi_setWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{ 
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setWBMode);
}

XCamReturn rk_aiq_uapi_getWBMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getWBMode);
}



/*
*****************************
*
* Desc: lock/unlock auto white balance
* Argument:
*
*
*****************************
*/
XCamReturn rk_aiq_uapi_lockAWB(const rk_aiq_sys_ctx_t* ctx)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return 0;;

}

XCamReturn rk_aiq_uapi_unlockAWB(const rk_aiq_sys_ctx_t* ctx)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return 0;;
}



/*
*****************************
*
* Desc: set auto white balance mode
* Argument:
*
*
*****************************
*/

XCamReturn rk_aiq_uapi_setAWBRange(const rk_aiq_sys_ctx_t* ctx, awbRange_t range)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    awbRange_t attr;
    memcpy(&attr, &range, sizeof(awbRange_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setAWBRange);
}

XCamReturn rk_aiq_uapi_getAWBRange(const rk_aiq_sys_ctx_t* ctx, awbRange_t *range)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    awbRange_t *attr = range;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getAWBRange);
}


/*
*****************************
*
* Desc: set manual white balance scene mode
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t scene)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    rk_aiq_wb_scene_t attr;
    memcpy(&attr, &scene, sizeof(scene));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setMWBScene);
}

XCamReturn rk_aiq_uapi_getMWBScene(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_scene_t *scene)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    rk_aiq_wb_scene_t *attr = scene;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getMWBScene);
}


/*
*****************************
*
* Desc: set manual white balance r/b gain
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    rk_aiq_wb_gain_t *attr = gain;
    CLIENT_CALL_SET_AIQ_P(rk_aiq_uapi_setAWBRange);
}

XCamReturn rk_aiq_uapi_getMWBGain(const rk_aiq_sys_ctx_t* ctx, rk_aiq_wb_gain_t *gain)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    rk_aiq_wb_gain_t *attr = gain;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getMWBScene);
}

/*
*****************************
*
* Desc: set manual white balance color temperature
* Argument:
*   ct: color temperature value [2800, 7500]K
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int ct)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int attr = &ct;
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setMWBCT);

}

XCamReturn rk_aiq_uapi_getMWBCT(const rk_aiq_sys_ctx_t* ctx, unsigned int *ct)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int *attr = ct;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getMWBCT);
}


/*
*****************************
*
* Desc: set color supperssion level
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setCrSuppsn(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setCrSuppsn);
}


XCamReturn rk_aiq_uapi_getCrSuppsn(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_setCrSuppsn);
}


/*
**********************************************************
* Focus & Zoom
**********************************************************
*/
/*
*****************************
*
* Desc: set focus mode
* Argument:
*   mode:  auto: auto focus
*          manual: manual focus
*          semi-auto: semi-auto focus
*****************************
*/
XCamReturn rk_aiq_uapi_setFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setFocusMode);
}

XCamReturn rk_aiq_uapi_getFocusMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;


    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getFocusMode);
}



/*
*****************************
*
* Desc: set minimum focus distance
* Argument:
*   disrance:  unint is cm
*****************************
*/
XCamReturn rk_aiq_uapi_setMinFocusDis(const rk_aiq_sys_ctx_t* ctx, unsigned int distance)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int level = distance;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMinFocusDis);

}

XCamReturn rk_aiq_uapi_getMinFocusDis(const rk_aiq_sys_ctx_t* ctx, unsigned int *distance)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    unsigned int *level = distance;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getMinFocusDis);    
}


/*
*****************************
*
* Desc: set optical zoom range
* Argument:
*   range:  [1.0, 100.0]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *range)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    paRange_t *attr = range;
    CLIENT_CALL_SET_AIQ_P(rk_aiq_uapi_setOpZoomRange);
}

XCamReturn rk_aiq_uapi_getOpZoomRange(const rk_aiq_sys_ctx_t* ctx, paRange_t *range)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    paRange_t *attr = range;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getOpZoomRange);
}

/*
*****************************
*
* Desc: set optical zoom speed
* Argument:
*   level:  [1, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomSpeed(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setOpZoomSpeed);
}

XCamReturn rk_aiq_uapi_getOpZoomSpeed(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getOpZoomSpeed);
}

/*
**********************************************************
* HDR
**********************************************************
*/
/*
*****************************
*
* Desc: set hdr mode
* Argument:
*   mode:
*     auto: auto hdr mode
*     manual：manual hdr mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setHDRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setHDRMode);
}

XCamReturn rk_aiq_uapi_getHDRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getHDRMode);
}

/*
*****************************
*
* Desc: set manual hdr strength
*    this function is active for HDR is manual mode
* Argument:
*   on:   1: on
*         0: off
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{  
    DBG("enter %s, line=%d\n",__FUNCTION__,__LINE__);
    rk_aiq_uapi_setMHDRStrth_t  para; 
    para.sys_ctx = ctx;
    para.on = on;
    para.level = level;
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1);
    return 0;;
}

XCamReturn rk_aiq_uapi_getMHDRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{  
    DBG("enter %s,line=%d\n",__FUNCTION__,__LINE__);
    if (level == NULL) { 
        return -1; 
    } 
    rk_aiq_uapi_getMHDRStrth_t  para; 
    para.sys_ctx = ctx; 
    call_fun_ipc_call((char *)__func__, &para, sizeof(para), 1); 
    *level = para.level;
    *on = para.on; 
    return para.returnvalue;
}

/*
**********************************************************
* Noise reduction
**********************************************************
*/
/*
*****************************
*
* Desc: set noise reduction mode
* Argument:
*   mode:
*     auto: auto noise reduction
*     manual：manual noise reduction
*
*****************************
*/
XCamReturn rk_aiq_uapi_setNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setNRMode);
}

XCamReturn rk_aiq_uapi_getNRMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getNRMode);
}

/*
*****************************
*
* Desc: set auto noise reduction strength
* Argument:
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setANRStrth);

}

XCamReturn rk_aiq_uapi_getANRStrth(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
     CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getANRStrth);
}

/*
*****************************
*
* Desc: set manual spatial noise reduction strength
*    this function is active for NR is manual mode
* Argument:
*   on: 1:on
*      0: off
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMSpaNRStrth);
}

XCamReturn rk_aiq_uapi_getMSpaNRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getMSpaNRStrth);
}

/*
*****************************
*
* Desc: set manual time noise reduction strength
*     this function is active for NR is manual mode
* Argument:
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMTNRStrth);
}

XCamReturn rk_aiq_uapi_getMTNRStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getMTNRStrth);
}

/*
**********************************************************
* Dehazer
**********************************************************
*/
/*
*****************************
*
* Desc: set dehaze mode
* Argument:
*   mode:
*     auto: auto dehaze
*     manual：manual dehaze
*
*****************************
*/

XCamReturn rk_aiq_uapi_setDhzMode(const rk_aiq_sys_ctx_t* ctx, opMode_t mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t attr;
    memcpy(&attr, &mode, sizeof(opMode_t));
    CLIENT_CALL_SET_AIQ(rk_aiq_uapi_setDhzMode);
}

XCamReturn rk_aiq_uapi_getDhzMode(const rk_aiq_sys_ctx_t* ctx, opMode_t *mode)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    opMode_t *attr = mode;
    CLIENT_CALL_GET_AIQ(rk_aiq_uapi_getDhzMode);
}

/*
*****************************
*
* Desc: set manual dehaze strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMDhzStrth(const rk_aiq_sys_ctx_t* ctx, bool on, unsigned int level)
{   
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setMDhzStrth);
    return 0;;
}

XCamReturn rk_aiq_uapi_getMDhzStrth(const rk_aiq_sys_ctx_t* ctx, bool *on, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_getMDhzStrth);

}

/*
**********************************************************
* Image adjust
**********************************************************
*/

/*
*****************************
*
* Desc: Adjust image contrast level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setContrast);
}

XCamReturn rk_aiq_uapi_getContrast(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getContrast);
}

/*
*****************************
*
* Desc: Adjust image brightness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setBrightness);
}

XCamReturn rk_aiq_uapi_getBrightness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getBrightness);
}

/*
*****************************
*
* Desc: Adjust image saturation level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{   
     const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setSaturation);
}

XCamReturn rk_aiq_uapi_getSaturation(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getSaturation);

}

/*
*****************************
*
* Desc: Adjust image sharpness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

     CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setSharpness);

}

XCamReturn rk_aiq_uapi_getSharpness(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{    
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getSharpness);
}

/*
*****************************
*
* Desc: Adjust image hue level
* Argument:
*    level: hue level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setHue(const rk_aiq_sys_ctx_t* ctx, unsigned int level)
{    const rk_aiq_sys_ctx_t* sys_ctx = ctx;

     CLIENT_CALL_SET_AIQ_EXT(rk_aiq_uapi_setHue);

}

XCamReturn rk_aiq_uapi_getHue(const rk_aiq_sys_ctx_t* ctx, unsigned int *level)
{
    const rk_aiq_sys_ctx_t* sys_ctx = ctx;
    CLIENT_CALL_GET_AIQ_EXT(rk_aiq_uapi_getHue);
}


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

#ifndef _RK_AIQ_UAPI_IMGPROC_IPC_IPC_H_
#define _RK_AIQ_UAPI_IMGPROC_IPC_IPC_H_
#include "base/xcam_common.h"
#include "rk_aiq_user_api_sysctl.h"
#include "rk_aiq_user_api_awb.h"
#include "rk_aiq_user_api_ae.h"
//#include "rk_aiq_uapi_af.h"
#include "rk_aiq_user_api_anr.h"
#include "rk_aiq_user_api_ahdr.h"
#include "rk_aiq_user_api_adehaze.h"
#include "rk_aiq_user_api_alsc.h"
#include "rk_aiq_user_api_accm.h"
#include "rk_aiq_user_api_a3dlut.h"
#include "rk_aiq_user_api_asharp.h"
#include "rk_aiq_user_api_agamma.h"

RKAIQ_BEGIN_DECLARE

/*
**********************************************************
* Exposure
**********************************************************
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
XCamReturn rk_aiq_uapi_setExpMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getExpMode_ipc(void *args);

/*
*****************************
*
* Desc: set auto exposure mode
* Argument:
*   mode:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setAeMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getAeMode_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setExpGainRange_ipc(void *args);
XCamReturn rk_aiq_uapi_getExpGainRange_ipc(void *args);
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
XCamReturn rk_aiq_uapi_setExpTimeRange_ipc(void *args);
XCamReturn rk_aiq_uapi_getExpTimeRange_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setBLCMode_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setHLCMode_ipc(void *args);
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
XCamReturn rk_aiq_uapi_setLExpMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getLExpMode_ipc(void *args);

/*
*****************************
*
* Desc: set manual lowlight exposure time ratio
* Argument:
*    ratio:  [1.0, 128.0]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMLExp_ipc(void *args);
XCamReturn rk_aiq_uapi_getMLExp_ipc(void *args);

XCamReturn rk_aiq_uapi_setAntiFlickerMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getAntiFlickerMode_ipc(void *args);
/*
*****************************
*
* Desc: set power line frequence
* Argument:
*    freq
*
*****************************
*/
XCamReturn rk_aiq_uapi_setExpPwrLineFreqMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getExpPwrLineFreqMode_ipc(void *args);


/*
*****************************
*
* Desc: set day night switch mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setDayNSwMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getDayNSwMode_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setMDNScene_ipc(void *args);
XCamReturn rk_aiq_uapi_getMDNScene_ipc(void *args);


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
XCamReturn rk_aiq_uapi_setADNSens_ipc(void *args);
XCamReturn rk_aiq_uapi_getADNSens_ipc(void *args);


/*
*****************************
*
* Desc: set fill light mode
* Argument:
*    mode
*
*****************************
*/
XCamReturn rk_aiq_uapi_setFLightMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getFLightMode_ipc(void *args);


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
XCamReturn rk_aiq_uapi_setMFLight_ipc(void *args);
XCamReturn rk_aiq_uapi_getMFLight_ipc(void *args);



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
XCamReturn rk_aiq_uapi_setWBMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getWBMode_ipc(void *args);


/*
*****************************
*
* Desc: lock/unlock auto white balance
* Argument:
*
*
*****************************
*/
XCamReturn rk_aiq_uapi_lockAWB_ipc(void *args);
XCamReturn rk_aiq_uapi_unlockAWB_ipc(void *args);


/*
*****************************
*
* Desc: set auto white balance mode
* Argument:
*
*
*****************************
*/
XCamReturn rk_aiq_uapi_setAWBRange_ipc(void *args);
XCamReturn rk_aiq_uapi_getAWBRange_ipc(void *args);


/*
*****************************
*
* Desc: set manual white balance scene mode
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBScene_ipc(void *args);
XCamReturn rk_aiq_uapi_getMWBScene_ipc(void *args);


/*
*****************************
*
* Desc: set manual white balance r/b gain
* Argument:
*   ct_scene:
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBGain_ipc(void *args);
XCamReturn rk_aiq_uapi_getMWBGain_ipc(void *args);

/*
*****************************
*
* Desc: set manual white balance color temperature
* Argument:
*   ct: color temperature value [2800, 7500]K
*
*****************************
*/
XCamReturn rk_aiq_uapi_setMWBCT_ipc(void *args);
XCamReturn rk_aiq_uapi_getMWBCT_ipc(void *args);


/*
*****************************
*
* Desc: set color supperssion level
* Argument:
*   level: [0, 100]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setCrSuppsn_ipc(void *args);
XCamReturn rk_aiq_uapi_getCrSuppsn_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setFocusMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getFocusMode_ipc(void *args);


/*
*****************************
*
* Desc: set minimum focus distance
* Argument:
*   disrance:  unint is cm
*****************************
*/
XCamReturn rk_aiq_uapi_setMinFocusDis_ipc(void *args);
XCamReturn rk_aiq_uapi_getMinFocusDis_ipc(void *args);


/*
*****************************
*
* Desc: set optical zoom range
* Argument:
*   range:  [1.0, 100.0]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomRange_ipc(void *args);
XCamReturn rk_aiq_uapi_getOpZoomRange_ipc(void *args);

/*
*****************************
*
* Desc: set optical zoom speed
* Argument:
*   level:  [1, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setOpZoomSpeed_ipc(void *args);
XCamReturn rk_aiq_uapi_getOpZoomSpeed_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setHDRMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getHDRMode_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setMHDRStrth_ipc(void *args);
XCamReturn rk_aiq_uapi_getMHDRStrth_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setNRMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getNRMode_ipc(void *args);

/*
*****************************
*
* Desc: set auto noise reduction strength
* Argument:
*   level: [0, 10]
*
*****************************
*/
XCamReturn rk_aiq_uapi_setANRStrth_ipc(void *args);
XCamReturn rk_aiq_uapi_getANRStrth_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setMSpaNRStrth_ipc(void *args);
XCamReturn rk_aiq_uapi_getMSpaNRStrth_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setMTNRStrth_ipc(void *args);
XCamReturn rk_aiq_uapi_getMTNRStrth_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setDhzMode_ipc(void *args);
XCamReturn rk_aiq_uapi_getDhzMode_ipc(void *args);

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
XCamReturn rk_aiq_uapi_setMDhzStrth_ipc(void *args);
XCamReturn rk_aiq_uapi_getMDhzStrth_ipc(void *args);
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
XCamReturn rk_aiq_uapi_setContrast_ipc(void *args);
XCamReturn rk_aiq_uapi_getContrast_ipc(void *args);

/*
*****************************
*
* Desc: Adjust image brightness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setBrightness_ipc(void *args);
XCamReturn rk_aiq_uapi_getBrightness_ipc(void *args);
/*
*****************************
*
* Desc: Adjust image saturation level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setSaturation_ipc(void *args);
XCamReturn rk_aiq_uapi_getSaturation_ipc(void *args);
/*
*****************************
*
* Desc: Adjust image sharpness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setSharpness_ipc(void *args);
XCamReturn rk_aiq_uapi_getSharpness_ipc(void *args);

/*
*****************************
*
* Desc: Adjust image hue level
* Argument:
*    level: hue level, [0, 100]
*****************************
*/
XCamReturn rk_aiq_uapi_setHue_ipc(void *args);
XCamReturn rk_aiq_uapi_getHue_ipc(void *args);

RKAIQ_END_DECLARE

#endif

/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License";   XCamReturn returnvalue;};
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

#ifndef _RK_AIQ_UAPI_IMGPROC_PTL_H_
#define _RK_AIQ_UAPI_IMGPROC_PTL_H_
#include "rk_aiq_user_api_imgproc.h"

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
typedef struct rk_aiq_uapi_setExpMode {
   rk_aiq_sys_ctx_t* sys_ctx;
   opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setExpMode_t;

typedef struct rk_aiq_uapi_getExpMode {
   rk_aiq_sys_ctx_t* sys_ctx;
   opMode_t attr;
      XCamReturn returnvalue;
} rk_aiq_uapi_getExpMode_t;

/*
*****************************
*
* Desc: set auto exposure mode
* Argument:
*   mode:
*
*****************************
*/

typedef struct rk_aiq_uapi_setAeMode {
    rk_aiq_sys_ctx_t* sys_ctx;
    aeMode_t attr;
    XCamReturn returnvalue;
} rk_aiq_uapi_setAeMode_t;

typedef struct rk_aiq_uapi_getAeMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    aeMode_t  attr;
      XCamReturn returnvalue;
} rk_aiq_uapi_getAeMode_t;

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
typedef struct rk_aiq_uapi_setExpGainRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setExpGainRange_t;

typedef struct rk_aiq_uapi_getExpGainRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getExpGainRange_t;
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
typedef struct rk_aiq_uapi_setExpTimeRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setExpTimeRange_t;

typedef struct rk_aiq_uapi_getExpTimeRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getExpTimeRange_t;

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
typedef struct rk_aiq_uapi_setBLCMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    bool on;
    aeMeasAreaType_t areaType;
   XCamReturn returnvalue;
} rk_aiq_uapi_setBLCMode_t;

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
typedef struct rk_aiq_uapi_setHLCMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    bool on;
} rk_aiq_uapi_setHLCMode_t;
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
typedef struct rk_aiq_uapi_setLExpMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setLExpMode_t;

typedef struct rk_aiq_uapi_getLExpMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getLExpMode_t;

/*
*****************************
*
* Desc: set manual lowlight exposure time ratio
* Argument:
*    ratio:  [1.0, 128.0]
*
*****************************
*/
typedef struct rk_aiq_uapi_setMLExp {
   rk_aiq_sys_ctx_t* sys_ctx;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMLExp_t;

typedef struct rk_aiq_uapi_getMLExp {
   rk_aiq_sys_ctx_t* sys_ctx;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMLExp_t;

/*
*****************************
*
* Desc: set power line frequence
* Argument:
*    freq
*
*****************************
*/
typedef struct rk_aiq_uapi_setExpPwrLineFreqMode {
   rk_aiq_sys_ctx_t* sys_ctx;
   expPwrLineFreq_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setExpPwrLineFreqMode_t;

typedef struct rk_aiq_uapi_getExpPwrLineFreqMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    expPwrLineFreq_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getExpPwrLineFreqMode_t;

typedef struct rk_aiq_uapi_setAntiFlickerMode {
   rk_aiq_sys_ctx_t* sys_ctx;
   antiFlickerMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setAntiFlickerMode_t;

typedef struct rk_aiq_uapi_getAntiFlickerMode {
   rk_aiq_sys_ctx_t* sys_ctx;
   antiFlickerMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getAntiFlickerMode_t;
/*
*****************************
*
* Desc: set day night switch mode
* Argument:
*    mode
*
*****************************
*/
typedef struct rk_aiq_uapi_setDayNSwMode {

   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setDayNSwMode_t;

typedef struct rk_aiq_uapi_getDayNSwMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getDayNSwMode_t;

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
typedef struct rk_aiq_uapi_setMDNScene {
   rk_aiq_sys_ctx_t* sys_ctx;
    dayNightScene_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMDNScene_t;

typedef struct rk_aiq_uapi_getMDNScene {
   rk_aiq_sys_ctx_t* sys_ctx;
    dayNightScene_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMDNScene_t;


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
typedef struct rk_aiq_uapi_setADNSens {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned    int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setADNSens_t;

typedef struct rk_aiq_uapi_getADNSens {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getADNSens_t;


/*
*****************************
*
* Desc: set fill light mode
* Argument:
*    mode
*
*****************************
*/
typedef struct rk_aiq_uapi_setFLightMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setFLightMode_t;

typedef struct rk_aiq_uapi_getFLightMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getFLightMode_t;


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
typedef struct rk_aiq_uapi_setMFLight {
   rk_aiq_sys_ctx_t* sys_ctx;
   bool level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMFLight_t;

typedef struct rk_aiq_uapi_getMFLight {
   rk_aiq_sys_ctx_t* sys_ctx;
    bool  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMFLight_t;



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
typedef struct rk_aiq_uapi_setWBMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setWBMode_t;

typedef struct rk_aiq_uapi_getWBMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getWBMode_t;


/*
*****************************
*
* Desc: lock/unlock auto white balance
* Argument:
*
*
*****************************
*/
typedef struct rk_aiq_uapi_lockAWB {
   rk_aiq_sys_ctx_t* sys_ctx;
   int abort;
   XCamReturn returnvalue;
} rk_aiq_uapi_lockAWB_t;

typedef struct rk_aiq_uapi_unlockAWB {
   rk_aiq_sys_ctx_t* sys_ctx;
   int  abort;
   XCamReturn returnvalue;
} rk_aiq_uapi_unlockAWB_t;


/*
*****************************
*
* Desc: set auto white balance mode
* Argument:
*
*
*****************************
*/
typedef struct rk_aiq_uapi_setAWBRange {
   rk_aiq_sys_ctx_t* sys_ctx;
   awbRange_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setAWBRange_t;

typedef struct rk_aiq_uapi_getAWBRange {

   rk_aiq_sys_ctx_t* sys_ctx;
    awbRange_t  attr;

   XCamReturn returnvalue;
} rk_aiq_uapi_getAWBRange_t;


/*
*****************************
*
* Desc: set manual white balance scene mode
* Argument:
*   ct_scene:
*
*****************************
*/
typedef struct rk_aiq_uapi_setMWBScene {
   rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_scene_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMWBScene_t;

typedef struct rk_aiq_uapi_getMWBScene {
   rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_scene_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMWBScene_t;


/*
*****************************
*
* Desc: set manual white balance r/b gain
* Argument:
*   ct_scene:
*
*****************************
*/
typedef struct rk_aiq_uapi_setMWBGain {
   rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_gain_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMWBGain_t;

typedef struct rk_aiq_uapi_getMWBGain {
   rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_gain_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMWBGain_t;

/*
*****************************
*
* Desc: set manual white balance color temperature
* Argument:
*   ct: color temperature value [2800, 7500]K
*
*****************************
*/
typedef struct rk_aiq_uapi_setMWBCT {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMWBCT_t;

typedef struct rk_aiq_uapi_getMWBCT {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMWBCT_t;


/*
*****************************
*
* Desc: set color supperssion level
* Argument:
*   level: [0, 100]
*
*****************************
*/
typedef struct rk_aiq_uapi_setCrSuppsn {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setCrSuppsn_t;

typedef struct rk_aiq_uapi_getCrSuppsn {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getCrSuppsn_t;

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
typedef struct rk_aiq_uapi_setFocusMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setFocusMode_t;

typedef struct rk_aiq_uapi_getFocusMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getFocusMode_t;


/*
*****************************
*
* Desc: set minimum focus distance
* Argument:
*   disrance:  unint is cm
*****************************
*/
typedef struct rk_aiq_uapi_setMinFocusDis {
   rk_aiq_sys_ctx_t* sys_ctx;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMinFocusDis_t;

typedef struct rk_aiq_uapi_getMinFocusDis {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMinFocusDis_t;


/*
*****************************
*
* Desc: set optical zoom range
* Argument:
*   range:  [1.0, 100.0]
*
*****************************
*/
typedef struct rk_aiq_uapi_setOpZoomRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setOpZoomRange_t ;

typedef struct rk_aiq_uapi_getOpZoomRange {
   rk_aiq_sys_ctx_t* sys_ctx;
    paRange_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getOpZoomRange_t;

/*
*****************************
*
* Desc: set optical zoom speed
* Argument:
*   level:  [1, 10]
*
*****************************
*/
typedef struct rk_aiq_uapi_setOpZoomSpeed {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setOpZoomSpeed_t;

typedef struct rk_aiq_uapi_getOpZoomSpeed {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getOpZoomSpeed_t;

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
typedef struct rk_aiq_uapi_setHDRMode {
   rk_aiq_sys_ctx_t* sys_ctx; 
   opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setHDRMode_t;
typedef struct rk_aiq_uapi_getHDRMode {
   rk_aiq_sys_ctx_t* sys_ctx; 
   opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getHDRMode_t;

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
typedef struct rk_aiq_uapi_setMHDRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on; 
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMHDRStrth_t;
typedef struct rk_aiq_uapi_getMHDRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMHDRStrth_t;

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
typedef struct rk_aiq_uapi_setNRMode {
   rk_aiq_sys_ctx_t* sys_ctx; 
   opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setNRMode_t;
typedef struct rk_aiq_uapi_getNRMode {
   rk_aiq_sys_ctx_t* sys_ctx; 
   opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getNRMode_t;

/*
*****************************
*
* Desc: set auto noise reduction strength
* Argument:
*   level: [0, 10]
*
*****************************
*/
typedef struct rk_aiq_uapi_setANRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setANRStrth_t;
typedef struct rk_aiq_uapi_getANRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getANRStrth_t;

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
typedef struct rk_aiq_uapi_setMSpaNRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMSpaNRStrth_t;
typedef struct rk_aiq_uapi_getMSpaNRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMSpaNRStrth_t;

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
typedef struct rk_aiq_uapi_setMTNRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMTNRStrth_t;
typedef struct rk_aiq_uapi_getMTNRStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on;
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMTNRStrth_t;

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
typedef struct rk_aiq_uapi_setDhzMode {
   rk_aiq_sys_ctx_t* sys_ctx;
    opMode_t attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_setDhzMode_t;
typedef struct rk_aiq_uapi_getDhzMode {
   rk_aiq_sys_ctx_t* sys_ctx; 
   opMode_t  attr;
   XCamReturn returnvalue;
} rk_aiq_uapi_getDhzMode_t;

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
typedef struct rk_aiq_uapi_setMDhzStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on;
  unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setMDhzStrth_t;
typedef struct rk_aiq_uapi_getMDhzStrth {
   rk_aiq_sys_ctx_t* sys_ctx; 
   bool on;
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getMDhzStrth_t;
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
typedef struct rk_aiq_uapi_setContrast {
   rk_aiq_sys_ctx_t* sys_ctx;
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setContrast_t;
typedef struct rk_aiq_uapi_getContrast {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getContrast_t;

/*
*****************************
*
* Desc: Adjust image brightness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
typedef struct rk_aiq_uapi_setBrightness {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setBrightness_t;
typedef struct rk_aiq_uapi_getBrightness {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getBrightness_t;
/*
*****************************
*
* Desc: Adjust image saturation level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
typedef struct rk_aiq_uapi_setSaturation {
   rk_aiq_sys_ctx_t* sys_ctx; 
   float level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setSaturation_t;
typedef struct rk_aiq_uapi_getSaturation {
   rk_aiq_sys_ctx_t* sys_ctx; 
   float level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getSaturation_t;
/*
*****************************
*
* Desc: Adjust image sharpness level
* Argument:
*    level: contrast level, [0, 100]
*****************************
*/
typedef struct rk_aiq_uapi_setSharpness {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setSharpness_t;
typedef struct rk_aiq_uapi_getSharpness {
   rk_aiq_sys_ctx_t* sys_ctx; 
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getSharpness_t;
/*
*****************************
*
* Desc: Adjust image hue level
* Argument:
*    level: hue level, [0, 100]
*****************************
*/
typedef struct rk_aiq_uapi_setHue {
   rk_aiq_sys_ctx_t* sys_ctx;
    unsigned int level;
   XCamReturn returnvalue;
} rk_aiq_uapi_setHue_t;
typedef struct rk_aiq_uapi_getHue {
   rk_aiq_sys_ctx_t* sys_ctx;
   unsigned int  level;
   XCamReturn returnvalue;
} rk_aiq_uapi_getHue_t;
RKAIQ_END_DECLARE

#endif

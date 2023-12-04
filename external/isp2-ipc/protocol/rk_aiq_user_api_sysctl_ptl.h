/*
 *  Copyright {c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License; Version 2.0 {the "License";};
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing; software
 * distributed under the License is distributed on an "AS IS" BASIS;
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND; either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef RK_AIQ_USER_API_SYSCTL_PTL_H
#define RK_AIQ_USER_API_SYSCTL_PTL_H
#pragma pack(1)
#include "rk_aiq.h"
#include "rk_aiq_algo_des.h"

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;


typedef struct rk_aiq_uapi_sysctl_init {
    char sns_ent_name[200];
    char iq_file_dir[200];
    rk_aiq_error_cb err_cb;
    rk_aiq_metas_cb metas_cb;
    rk_aiq_sys_ctx_t* return_ctx;
} rk_aiq_uapi_sysctl_init_t;

typedef struct rk_aiq_uapi_sysctl_deinit {
    rk_aiq_sys_ctx_t* ctx;
} rk_aiq_uapi_sysctl_deinit_t;

typedef struct rk_aiq_uapi_sysctl_prepare {
    rk_aiq_sys_ctx_t* ctx;
    uint32_t width;
    uint32_t height;
    rk_aiq_working_mode_t mode;
    XCamReturn xcamreturn;
} rk_aiq_uapi_sysctl_prepare_t;

typedef struct rk_aiq_uapi_sysctl_start {
    rk_aiq_sys_ctx_t* ctx;
   XCamReturn xcamreturn;
} rk_aiq_uapi_sysctl_start_t;

typedef struct rk_aiq_uapi_sysctl_stop {
    rk_aiq_sys_ctx_t* ctx;
     XCamReturn xcamreturn;
} rk_aiq_uapi_sysctl_stop_t;

typedef struct rk_aiq_uapi_sysctl_getStaticMetas {
    char sns_ent_name[200];
    rk_aiq_static_info_t static_info
} rk_aiq_uapi_sysctl_getStaticMetas_t;

typedef struct rk_aiq_uapi_sysctl_getMetaData {
    rk_aiq_sys_ctx_t* ctx;
    uint32_t frame_id;
    rk_aiq_metas_t metas;
} rk_aiq_uapi_sysctl_getMetaData_t;

typedef struct rk_aiq_uapi_sysctl_setModuleCtl {
    rk_aiq_sys_ctx_t* ctx;
    int32_t mod_en;
} rk_aiq_uapi_sysctl_setModuleCtl_t;

typedef struct rk_aiq_uapi_sysctl_getModuleCtl {
    rk_aiq_sys_ctx_t* ctx;
} rk_aiq_uapi_sysctl_getModuleCtl_t;

typedef struct rk_aiq_uapi_sysctl_regLib {
    rk_aiq_sys_ctx_t* ctx;
} rk_aiq_uapi_sysctl_regLib_t;

typedef struct rk_aiq_uapi_sysctl_unRegLib {
    rk_aiq_sys_ctx_t* ctx;
    int algo_type;
    int lib_id;
} rk_aiq_uapi_sysctl_unRegLib_t;

typedef struct rk_aiq_uapi_sysctl_enableAxlib {
    rk_aiq_sys_ctx_t* ctx;
    int algo_type;
    int lib_id;
    bool enable;
} rk_aiq_uapi_sysctl_enableAxlib_t;

typedef struct rk_aiq_uapi_sysctl_getAxlibStatus {
    rk_aiq_sys_ctx_t* ctx;
    int algo_type;
    int lib_id;
} rk_aiq_uapi_sysctl_getAxlibStatus_t;

typedef struct rk_aiq_uapi_sysctl_getEnabledAxlibCtx {
    rk_aiq_sys_ctx_t* ctx;
    int algo_type;
} rk_aiq_uapi_sysctl_getEnabledAxlibCtx_t;

RKAIQ_END_DECLARE

#endif

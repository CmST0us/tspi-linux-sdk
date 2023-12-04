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
#include "rk_aiq.h"
#include "rk_aiq_algo_des.h"
#include "../protocol/rk_aiq_user_api_sysctl_ptl.h"
#include "call_fun_ipc.h"
RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;

/*!
 * \brief initialze aiq control system context
 * Should call before any other APIs
 *
 * \param[in] sns_ent_name    active sensor media entity name. This represents the unique camera module\n
 *                            in system. And the whole active pipeline could be retrieved by this.
 * \param[in] iq_file_dir     define the search directory of the iq files.
 * \param[in] err_cb          not mandatory. it's used to return system errors to user.
 * \param[in] metas_cb        not mandatory. it's used to return the metas(sensor exposure settings,\n
 *                            isp settings, etc.) for each frame
 * \return return system context if success, or NULL if failure.
 */
rk_aiq_sys_ctx_t*
rk_aiq_uapi_sysctl_init(const char* sns_ent_name,
                        const char* iq_file_dir,
                        rk_aiq_error_cb err_cb,
                        rk_aiq_metas_cb metas_cb) {
    rk_aiq_uapi_sysctl_init_t para;
    memset(&para, 0, sizeof(rk_aiq_uapi_sysctl_init_t));
    if (sns_ent_name!=NULL && strlen(sns_ent_name) > sizeof(para.sns_ent_name)) {
       printf("%s err sns_ent_name too long\n", __func__);
       return NULL;
    }
 
    if (iq_file_dir!=NULL && strlen(iq_file_dir) > sizeof(para.iq_file_dir)) {
       printf("%s err iq_file_dir too long\n", __func__);
       return NULL;
    }
   printf("=======sns_ent_name=%s\n",sns_ent_name);
   if (sns_ent_name != NULL) {
        strcpy(para.sns_ent_name, sns_ent_name);
   }
   if (iq_file_dir != NULL) {
        strcpy(para.iq_file_dir, iq_file_dir);
   }
    printf("call call_fun_ipc_call enter, sns_ent_name=%s\n",para.sns_ent_name);
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_init_t), 1);
    printf("call call_fun_ipc_call,return_ctx=%d\n",para.return_ctx);
    return para.return_ctx;
}

/*!
 * \brief deinitialze aiq context
 * Should not be called in started state
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 */
void
rk_aiq_uapi_sysctl_deinit(rk_aiq_sys_ctx_t* ctx) {

    rk_aiq_uapi_sysctl_deinit_t para;
    para.ctx = ctx;
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_deinit_t), 0);
}

/*!
 * \brief prepare aiq control system before runninig
 * prepare AIQ running enviroment, should be called before \ref rk_aiq_uapi_sysctl_start.\n
 * And if re-prepared is required after \ref rk_aiq_uapi_sysctl_start is called,\n
 * should call \ref rk_aiq_uapi_sysctl_stop firstly.
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 * \param[in] width           sensor active output width, just used to check internally
 * \param[in] height          sensor active output height, just used to check internally
 * \param[in] mode            pipleline working mode
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_prepare(const rk_aiq_sys_ctx_t* ctx,
                           uint32_t  width, uint32_t  height,
                           rk_aiq_working_mode_t mode) {
    rk_aiq_uapi_sysctl_prepare_t para;
    para.ctx = ctx;
    para.width = width;
    para.height = height;
    memcpy(&para.mode,&mode, sizeof(rk_aiq_working_mode_t));
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_prepare_t), 1);

    return para.xcamreturn;
}

/*!
 * \brief start aiq control system
 * should be called after \ref rk_aiq_uapi_sysctl_prepare. After this call,
 * the aiq system repeats getting 3A statistics from ISP driver, running
 * aiq algorimths(AE, AWB, AF, etc.), setting new parameters to drivers.
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_start(const rk_aiq_sys_ctx_t* ctx) {

    rk_aiq_uapi_sysctl_start_t para;
    para.ctx = ctx;
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_start_t), 1);

    return para.xcamreturn;
}

/*!
 * \brief stop aiq control system
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_stop(const rk_aiq_sys_ctx_t* ctx) {
    rk_aiq_uapi_sysctl_stop_t para;

    para.ctx = ctx;
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_stop_t), 1);

    return para.xcamreturn;
}

rk_aiq_static_info_t*
rk_aiq_uapi_sysctl_getStaticMetas(const char* sns_ent_name) {
    rk_aiq_uapi_sysctl_getStaticMetas_t para;
    rk_aiq_static_info_t  *static_info = (rk_aiq_static_info_t*)malloc(sizeof(rk_aiq_static_info_t));
    if (strlen(sns_ent_name) > sizeof(para.sns_ent_name))
        printf("%s err sns_ent_name too long\n", __func__);

    strcpy(para.sns_ent_name, sns_ent_name);
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_getStaticMetas_t), 1);
    memcpy(static_info, &para.static_info, sizeof(rk_aiq_static_info_t));
    return static_info;
}

rk_aiq_metas_t*
rk_aiq_uapi_sysctl_getMetaData(const rk_aiq_sys_ctx_t* ctx, uint32_t frame_id){
    rk_aiq_uapi_sysctl_getMetaData_t para;
    rk_aiq_metas_t *metas = (rk_aiq_metas_t*)malloc(sizeof(rk_aiq_metas_t));
    para.ctx = ctx;
    para.frame_id = frame_id;
    call_fun_ipc_call((char *)__func__, &para, sizeof(rk_aiq_uapi_sysctl_getMetaData_t), 1);
    memcpy(metas, &para.metas, sizeof(rk_aiq_metas_t));
    return metas;
}

#if 0
int32_t
rk_aiq_uapi_sysctl_getState(const rk_aiq_sys_ctx_t* ctx);
#endif

XCamReturn
rk_aiq_uapi_sysctl_setModuleCtl(const rk_aiq_sys_ctx_t* ctx, int32_t mod_en) {
    return 0;
}

int32_t
rk_aiq_uapi_sysctl_getModuleCtl(const rk_aiq_sys_ctx_t* ctx){
    return 0;
}

/*!
 * \brief register customized algo lib
 *
 * \param[in] ctx context
 * \param[in,out] algo_lib_des allocate a new unique id value for algo_lib_des->id if success.\n
 *                             this id could be used in \ref rk_aiq_uapi_sysctl_unRegLib
 *                             or other lib APIs. The deference object by
 *                             \ref algo_lib_des should be valid until \ref rk_aiq_uapi_sysctl_unRegLib
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_regLib(const rk_aiq_sys_ctx_t* ctx,
                          RkAiqAlgoDesComm* algo_lib_des) {
     return 0;
}

/*!
 * \brief unregister customized algo lib
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \param[in] lib_id          returned by call \ref rk_aiq_uapi_sysctl_regLib
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_unRegLib(const rk_aiq_sys_ctx_t* ctx,
                            const int algo_type,
                            const int lib_id) {
     return 0;
}

/*!
 * \brief enable or disable algo lib
 * If the \ref lib_id is the same as the current running algo, this interface
 * could be called in any state except for the context uninitialized. Otherwise,
 * it could only be called in prepared or initialized state followed by
 * call \ref rk_aiq_uapi_sysctl_prepare , and in this case, the old algo which type
 * is \ref algo_type will be replaced by the new algo \ref lib_id.
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \param[in] lib_id          0 for system integrated algos;\n
 *                            returned by call \ref rk_aiq_uapi_sysctl_regLib for customer algos
 * \param[in] enable          enable or disable algos. enable means running the algo's processes\n
 *                            defined in \ref RkAiqAlgoDesComm; disable means\n
 *                            bypass the algo's prcosses.
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_enableAxlib(const rk_aiq_sys_ctx_t* ctx,
                               const int algo_type,
                               const int lib_id,
                               bool enable) {
    return 0;
}

/*!
 * \brief get algo lib enabled status
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \param[in] lib_id          0 for system integrated algos;\n
 *                            returned by call \ref rk_aiq_uapi_sysctl_regLib for customer algos
 * \return                    return true if enabled , false if disabled or unregistered
 */
bool
rk_aiq_uapi_sysctl_getAxlibStatus(const rk_aiq_sys_ctx_t* ctx,
                                  const int algo_type,
                                  const int lib_id) {
    return 0;
}

/*!
 * \brief get enabled algo lib context
 * The returned algo context will be used as the first param of algo private APIs.
 * For those customer's algo lib, this interface should be called after
 * \ref rk_aiq_uapi_sysctl_enableAxlib, or will return NULL.
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \return return current enabled algo context if success or NULL.
 */
const RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getEnabledAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type) {
    
    return NULL;
}

RKAIQ_END_DECLARE


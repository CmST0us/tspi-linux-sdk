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

#ifndef RK_AIQ_USER_API_SYSCTL_IPC_H
#define RK_AIQ_USER_API_SYSCTL_IPC_H

#include "rk_aiq_user_api_sysctl.h"

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
typedef enum rk_aiq_state_e {
    AIQ_STATE_INVALID           = 0,
    AIQ_STATE_INITIALIZED       = 1,
    AIQ_STATE_PREPARED          = 2,
    AIQ_STATE_RUNNING           = 3,
    AIQ_STATE_STOPPED           = 4,
    AIQ_STATE_MAX
} rk_aiq_state_t;

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
rk_aiq_uapi_sysctl_init_ipc(void *args);

/*!
 * \brief deinitialze aiq context
 * Should not be called in started state
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 */
void
rk_aiq_uapi_sysctl_deinit_ipc(void *args);

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
rk_aiq_uapi_sysctl_prepare_ipc(void *args);

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
rk_aiq_uapi_sysctl_start_ipc(void *args);

/*!
 * \brief stop aiq control system
 *
 * \param[in] ctx             the context returned by \ref rk_aiq_uapi_sysctl_init
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_stop_ipc(void *args);

XCamReturn
rk_aiq_uapi_sysctl_getStaticMetas_ipc(void *args);

XCamReturn
rk_aiq_uapi_sysctl_getMetaData_ipc(void *args);

#if 0
int32_t
rk_aiq_uapi_sysctl_getState(const rk_aiq_sys_ctx_t* ctx);
#endif

XCamReturn
rk_aiq_uapi_sysctl_setModuleCtl_ipc(void *args);

int32_t
rk_aiq_uapi_sysctl_getModuleCtl_ipc(void *args);

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
rk_aiq_uapi_sysctl_regLib_ipc(void *args);

/*!
 * \brief unregister customized algo lib
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \param[in] lib_id          returned by call \ref rk_aiq_uapi_sysctl_regLib
 * \return return 0 if success
 */
XCamReturn
rk_aiq_uapi_sysctl_unRegLib_ipc(void *args);

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
rk_aiq_uapi_sysctl_enableAxlib_ipc(void *args);

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
rk_aiq_uapi_sysctl_getAxlibStatus_ipc(void *args);

/*!
 * \brief get enabled algo lib context
 * The returned algo context will be used as the first param of algo private APIs.
 * For those customer's algo lib, this interface should be called after
 * \ref rk_aiq_uapi_sysctl_enableAxlib, or will return NULL.
 *
 * \param[in] ctx             context
 * \param[in] algo_type       algo type defined by RkAiqAlgoDesComm.type
 * \return return 0 if success
 */
const RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getEnabledAxlibCtx_ipc(void *args);

rk_aiq_state_t rk_aiq_get_state();

RKAIQ_END_DECLARE

#endif

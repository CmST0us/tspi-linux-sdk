/*
 *
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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
 */
#ifndef __VPU_DRV_H__
#define __VPU_DRV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define VPU_PATH                "/tmp/vpu_server"
// VPU_MAGIC 'v'
#define VPU_MAGIC               0x76

//#define _VPU_DEBUG

#define VPU_DEBUG(fmt, args...) /* not debugging: nothing */

typedef struct VPU_REGISTER {
    VPU_CLIENT_TYPE client_type;
    int pid;
} VPU_REGISTER_S;

typedef struct VPU_DEC_REG {
    RK_U32 reg[VPU_REG_NUM_DEC];
} VPU_DEC_REG_S;

typedef struct VPU_ENC_REG {
    RK_U32 reg[VPU_REG_NUM_ENC];
} VPU_ENC_REG_S;

RK_S32 VPUSendMsg(int sock, VPU_CMD_TYPE  cmd, void *msg, RK_U32 len, int flags);
RK_S32 VPURecvMsg(int sock, VPU_CMD_TYPE *cmd, void *msg, RK_U32 len, int flags);

#ifdef __cplusplus
}

#endif

#endif                       /* __VPU_DRV_H__ */


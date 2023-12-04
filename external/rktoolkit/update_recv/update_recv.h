/*
 *  Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *  Author: chad.ma <chad.ma@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __UPDATE_RECV_H__
#define __UPDATE_RECV_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "DefineHeader.h"

#define  UPDATE_IMG             "update.img"
#define  DEV_RECOVERY_NODE      "/dev/block/by-name/recovery"

extern int WriteFwData(char* imgPath, char* fwName);
extern bool CheckFwData(char* imgPath, char* fwName);
#endif

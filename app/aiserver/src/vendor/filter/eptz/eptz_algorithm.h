/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 * author: kevin.lin@rock-chips.com
 *   date: 2020-09-18
 * module: eptz
 */

#ifndef EPTZ_ALGORITHM_H_
#define EPTZ_ALGORITHM_H_

#include "eptz_type.h"

typedef struct _EptzInitInfo {
  INT32 eptz_npu_width;   // AI计算使用图像的宽
  INT32 eptz_npu_height;  // AI计算使用图像的高
  INT32 eptz_src_width;   //摄像头源数据的宽
  INT32 eptz_src_height;  //摄像头源数据的高
  INT32 eptz_dst_width;   //摄像头裁剪区域的宽，小于等于数据源宽
  INT32 eptz_dst_height;  //摄像头裁剪区域的高，小于等于数据源高
  INT32 camera_dst_width;   //预览显示的宽
  INT32 camera_dst_height;  //预览显示的高
  INT32 eptz_threshold_x; //人脸跟踪X灵敏度
  INT32 eptz_threshold_y; //人脸跟踪Y灵敏度
  INT32 eptz_iterate_x;   //人脸跟踪X方向速度
  INT32 eptz_iterate_y;   //人脸跟踪Y方向速度
  float eptz_facedetect_score_shold; // AI人脸质量分阈值
  INT32 eptz_fast_move_frame_judge;  //人物移动防抖阈值
  INT32 eptz_zoom_frame_judge;       //人物移动ZOOM防抖阈值
  INT32 eptz_zoom_speed;             //人脸跟踪ZOOM效果转换速度
  INT32 eptz_animation;              //人物移动动画效果
} EptzInitInfo;

/*
 * 说明：eptz参数初始化接口
 * parameter
 * _eptz_info: eptz初始化结构体
 */
EPTZ_RET eptzConfigInit(EptzInitInfo *_eptz_info);

/*
 * 说明：eptz裁剪区域计算接口
 * parameter
 * eptz_ai_data: AI相关人脸信息
 * output_result：算法返回的坐标结果
 * 返回值：EPTZ_OK 成功，EPTZ_ERR_BAD 失败
 */
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result);

/*
 * 说明：eptz裁剪区域计算接口
 * parameter
 * eptz_ai_data: AI相关人脸信息
 * output_result：算法返回的坐标结果
 * back_to_origin：未检测到人脸信息时，是否返回画面中心，默认为false
 * delay_time：配合back_to_origin使用，单位为秒，默认值5
 * 返回值：EPTZ_OK 成功，EPTZ_ERR_BAD 失败
 */
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result,
                           bool back_to_origin, int delay_time);

/*
 * 说明：返回值：根据人脸坐标计算当前人脸所在范围（calculateClipRect后调用）
 */
float *getPersonCurrentRect();

/*
 * 说明：返回值：true 移动状态，false 停止状态（calculateClipRect后调用）
 */
bool isMoving();

/*
 * 说明：设置eptz初始移动坐标（calculateClipRect前调用）
 * parameter
 * custom_postion: eptz初始移动位置
 */
void reSetPosition(INT32 *custom_postion);

/*
 * 说明：读取lic文件并校验
 */
void setEptzLicense(char *lic_path);

/*
 * 说明：动态切换eptz_zoom.confg文件
 */
void changeEptzConfig(char *config_path);

/*
 * 说明：动态切换eptz模式
 * parameter
 * mode模式: 1：灵动模式，跟随灵敏。 2：会议模式，画面稳定。
 */
void setEptzMode(INT32 mode);

/*
 * 说明：定制私有功能标志位
 * parameter
 */
void setPrivateFlg(INT32 flag);

#endif // EPTZ_ALGORITHM_H_

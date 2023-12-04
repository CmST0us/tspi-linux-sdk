



# Rockchip AUTO EPTZ方案介绍

文件标识：RK-XX-XX-0000

发布版本：V1.0.2

日期：2021-01-18

文件密级：□绝密   □秘密   □内部资料   ■公开

---

**免责声明**

本文档按“现状”提供，瑞芯微电子股份有限公司（“本公司”，下同）不对本文档的任何陈述、信息和内容的准确性、可靠性、完整性、适销性、特定目的性和非侵权性提供任何明示或暗示的声明或保证。本文档仅作为使用指导的参考。

由于产品版本升级或其他原因，本文档将可能在未经任何通知的情况下，不定期进行更新或修改。

**商标声明**

“Rockchip”、“瑞芯微”、“瑞芯”均为本公司的注册商标，归本公司所有。

本文档可能提及的其他所有注册商标或商标，由其各自拥有者所有。

**版权所有** **© 2021 **瑞芯微电子股份有限公司**

超越合理使用范畴，非经本公司书面许可，任何单位和个人不得擅自摘抄、复制本文档内容的部分或全部，并不得以任何形式传播。

瑞芯微电子股份有限公司

Rockchip Electronics Co., Ltd.

地址：     福建省福州市铜盘路软件园A区18号

网址：     [www.rock-chips.com](http://www.rock-chips.com)

客户服务电话： +86-4007-700-590

客户服务传真： +86-591-83951833

客户服务邮箱： [fae@rock-chips.com](mailto:fae@rock-chips.com)

---

**读者对象**

本文档主要适用于以下工程师：

- 技术支持工程师
- 软件开发工程师

**修订记录**

| **日期**   | **版本** | **作者** | **修改说明** |
| ---------- | -------- | -------- | ------------ |
| 2021/01/11 | 1.0.0    | 林其浩   | 初始版本     |
| 2021/01/18 | 1.0.1    | 林其浩   | 添加Q&A章节  |
| 2021/03/05 | 1.0.2    | 林其浩   | 内容完善     |



**目 录**

[TOC]



## 概述

Rockchip Linux平台支持EPTZ电子云台功能，指通过软件手段，结合智能识别技术实现预览界面的“数字平移- 倾斜- 缩放/变焦”功能。配合RK ROCKX人脸检测算法，快速实现预览画面人物聚焦功能，可应用于视屏会议等多种场景。

## EPTZ算法集成说明

### 简介

EPTZ模块支持库为libeptz.so，通过对EptzInitInfo结构体进行配置，实现相应的操作。相关代码位于SDK以下路径：

```
app/aiserver/src/vendor/samples/filter/eptz/
```

具体接口可参考eptz_algorithm.h文件，eptz版本说明详见app\aiserver\src\vendor\samples\filter\eptz\release_note.txt。

从v1.0.5版本开始支持两种模式：1.灵动模式；2.会议模式。

### EPTZ功能验证

------

RV1126/RV1109使用EPTZ功能，需将dts中的otp节点使能，evb默认配置中已将其使能。

```shell
&otp {
status = "okay";
};
```

在RV1126/RV1109中，提供三种方案进行AUTO EPTZ功能验证及使用。

* 环境变量：在启动脚本（例如：RkLunch.sh）中添加环境变量export ENABLE_EPTZ=1，默认开启EPTZ功能，
  在所有预览条件下都将启用人脸跟随效果。

* XU控制：通过UVC扩展协议，参考5.1中描述进行实现。当uvc_app接收到XU的CMD_SET_EPTZ(0x0a)指令
  时，将根据指令中所带的int参数1或0，进行EPTZ功能的开关，以确认下次预览时是否开启人脸跟随效果。

* dbus指令：最新版本已支持通过dbus指令通知aiserver进程跨进程动态启动AUTO EPTZ能力：

  ```shell
  #开启命令
  dbus-send --system --print-reply --type=method_call --dest=rockchip.aiserver.control
  /rockchip/aiserver/control/graph rockchip.aiserver.control.graph.EnableEPTZ int32:1
  #关闭命令
  dbus-send --system --print-reply --type=method_call --dest=rockchip.aiserver.control
  /rockchip/aiserver/control/graph rockchip.aiserver.control.graph.EnableEPTZ int32:0
  ```

RV1126/RV1109显示预期效果:

* 单人：在camera可视范围内，尽可能将人脸保持在画面中间。
* 多人：在camera可视范围内，尽可能的显示人多画面，且将其保持在画面中间。

具体测试用例，可参考《Rockchip AUTO EPTC验收用例》文档。

### 调试手段

* touch /tmp/eptz_face_debug：动态输出人脸坐标信息
* touch /tmp/eptz_zoom_debug：动态打开zoom日志，显示当前人脸范围和裁剪比例数据
* 设置环境变量export eptz_log_level=3：打开eptz控制算法，坐标计算debug信息
* touch /tmp/eptz_mode1: 动态切换到灵动模式，跟随灵敏。
* touch /tmp/eptz_mode2: 动态切换到会议模式，人物稳定后开始画面切换。

## 人脸检测算法集成说明

### ROCKX算法模型替换

------

SDK默认使用rockx_face_detect_v3模型，但rockx同时有提供rockx_face_detect_v2及rockx_face_detect_v2_h模型，三者数据对比如下：

| 场景                                                         | 识别有效距离 | AI数据帧率 | DDR带宽  | aiserver Uss 内存 |
| ------------------------------------------------------------ | ------------ | ---------- | -------- | ----------------- |
| 1080mjpeg 预览<br />scale1 720 nn<br />rockx_face_detect_v2  | 5米左右      | 10fps      | 3172MB/s | 106740K           |
| 1080mjpeg 预览<br />scale1 720 nn<br />rockx_face_detect_v2_h | 5米左右      | 15fps      | 3112MB/s | 105232K           |
| 1080mjpeg 预览<br />scale1 720 nn<br />rockx_face_detect_v3  | 2米左右      | 30fps      | 2881MB/s | 105596K           |

EPTZ对AI数据帧率要求不高，因此近距离场景建议使用**rockx_face_detect_v3**模型，较远距离场景使用**rockx_face_detect_v2_h**模型。

SDK默认使用**rockx_face_detect_v3**，模型替换为**rockx_face_detect_v2_h**需执行以下步骤：

* 将external/rockx/目录下的**rockx_face_detect_v2_horizontal.data**打包到usr/lib或oem/usr/lib目录下

  ```shell
  diff --git a/sdk/rockx-rv1109-Linux/RockXConfig.cmake b/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  index dd77dc7..151ed97 100644
  --- a/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  +++ b/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  @@ -39,7 +39,7 @@ if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109")
           set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/carplate_recognition.data")
       endif()
       if(${WITH_ROCKX_FACE_DETECTION})
  -        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_detection_v3.data")
  +        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/rockx_face_detect_v2_horizontal.data")
       endif()
       if(${WITH_ROCKX_FACE_RECOGNITION})
           set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_recognition.data")
  ```

* 修改app/aiserver/src/vendor/CMakeLists.txt

  ```shell
  diff --git a/src/vendor/CMakeLists.txt b/src/vendor/CMakeLists.txt
  index d3c8774..ffd177a 100755
  --- a/src/vendor/CMakeLists.txt
  +++ b/src/vendor/CMakeLists.txt
  @@ -25,7 +25,7 @@ if (${ENABLE_SAMPLE_NODE_EPTZ})
       )
   endif()
   
  -option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" OFF)
  +option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" ON)
  ```

修改后重新编译rockx模块、aiserver模块即可。

后续可以通过修改aicamera.json文件node_4、node_11中的opt_rockx_model，替换其他算法模型进行效果验证，推荐rockx_face_detect_v2_h和rockx_face_detect_v3模型。

若运行时提示 xxx model data not found, 需将对应模型文件从external/rockx/sdk/rockx-data-rv1109/下拷贝到/usr/lib或oem/usr/lib目录下。

备注：若sensor为2K以上分辨率，建议将RTNodeVFilterEptzDemo.cpp中的eptz_npu_width和eptz_npu_height修改为1280和720，同时将aicamera.json中的node_2节点opt_width、opt_height、opt_vir_width、opt_vir_height修改为1280、720、1280、720，可以提高人脸检测的识别率和准备率。若sensor做多只支持bypass 1920x1080输出，则相应节点需修改为640x360。

### 第三方检测算法集成

------

第三方算法集成，可参考external/rockit/doc/《Rockchip_Developer_Guide_Linux_Rockit_CN.pdf》文档进行开发。

具体代码demo可参考以下目录：

```shell
app/aiserver/src/vendor/samples/filter/rockx/
```

结合EPTZ功能使用时，需注意RTNodeVFilterEptzDemo.cpp中传入的AI数据结构要同步修改。

## Q&A

1.  人脸识别率不够，或识别距离较近该如何改善？

   目前EPTZ算法，人脸检测基于RK ROCKX模块，使用ROCKX中的NPU人脸检测算法实现。可参考3.1节使用rockx_face_detect_v2_h进行验证，2K分辨率以上sensor使用1280、720输入给算法。若还不能满足场景要求，建议使用第三方算法。

2. eptz初始化参数详细代表什么意思？

   * eptz_src_width、eptz_src_height：sensor bypass支持的最大输出分辨率，RkLunch.sh中通过环境变量获取。
   * eptz_dst_width、eptz_dst_height：eptz算法检测出的目标区域，即裁剪放大区域。默认设置为同eptz_src_width、eptz_src_height一致，即第一次打开预览全视角显示。
   * camera_dst_width、camera_dts_height：当前实际预览分辨率，由uvc_app传入，可以做相关变量参数配置使用。
   * eptz_npu_width、eptz_npu_height：输入算法的图像分辨率大小。
   * eptz_facedetect_score_shold：人脸检测算法数据中的阈值，通过该值可以将低质量的人脸过滤排除。
   * eptz_zoom_speed：zoom速度调整，可设置1,2,3，默认为1，其中3的速度最快。
   * eptz_fast_move_frame_judge：忽略人物x帧内快速移动的数据，人物移动防抖阈值。
   * eptz_zoom_frame_judge：忽略人物x帧内zoom比例变化数据，人物移动ZOOM防抖阈值。
   * eptz_threshold_x、eptz_threshold_y: x，y方向移动阈值，当x，y的范围变化超过设定值时进行移动调整。
   * mLastXY：初始化时，第一次显示的范围区域大小。

3. 人物移动时，发现画面人物跟随不上如何解决？

   * 可自行添加打印查看人脸检测算法帧率，AI帧率过低则可能存在现象，可以考虑算法优化。
   * 小幅度修改eptz_iterate_x和eptz_iterate_y的值可加快跟随速度，但修改值太大会造成画面移动不顺滑。

4. 是否支持动态调整eptz初始化参数，如何进行修改配置?

   支持动态修改阈值等参数，可在open或process方法中直接修改mEptzInfo对象参数，可参考RTNodeVFilterEptzDemo.cpp。

5. eptz_zoom.conf需要如何调整？其中参数对应关系指什么？

   area_ratio_data 是检测到的人脸范围，clip_ratio_data  是对应的裁剪范围。

   eptz_zoom-3840x2160.conf指初略适配3840x2160sensor的配置文件，其中的内容：

   ```shell
   area_ratio_data:0.035,0.07,0.3,0.55,0.75,1.0
   clip_ratio_data-480p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-720p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-1080p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-1440p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-2160p:0.10,0.30,0.45,0.70,0.85,1.0
   ```

   * 当人脸的范围在画面中所占比例小于等于0.035时，显示范围区域为sensor全视角的0.10，产生聚焦效果。
   * 当人物靠近sensor或人数增多时，画面占比逐渐变大，如超过0.3小于0.55，显示范围区域为sensor全视角的0.7，视角放大。
   * clip_ratio_data-480p代表预览分辨率为480P时，eptz模块读取的对应比例参数，clip_ratio_data-720p代表预览分辨率为720P时，eptz模块读取的对应比例参数，以此类推。

   SDK默认提供最大分辨率为1440P和2160P的配置文件eptz_zoom-2560x1440.conf和eptz_zoom-3840x2160.conf。对应的NPU输入为1280x720。

   用户定制对应eptz_zoom.conf文件时，通过touch /tmp/eptz_zoom_debug动态打开日志，查看当前人脸范围和裁剪比例数据 face_ratio[%.2f] eptz_clip_ratio[%.2f]。固定使用同一预览分辨率，分别在0.5m、1m、2m、3m|、5m（**距离仅作举例说明，可以减少或增加其中的部分数据**）等距离测量单人及多人时的face_ratio数值，配上期望的eptz_clip_ratio数值。如在1080P分辨率下，测试出一组满意的eptz_clip_ratio数据后，如:

   ```shell
   area_ratio_data:0.035,0.07,0.3,0.55,0.75,1.0
   clip_ratio_data-1080p:0.10,0.30,0.45,0.70,0.85,1.0
   ```

   可以将该比例copy到其他预览分辨率数据下，如下所示：

   ```shell
   clip_ratio_data-480p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-720p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-1080p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-1440p:0.10,0.30,0.45,0.70,0.85,1.0
   clip_ratio_data-2160p:0.10,0.30,0.45,0.70,0.85,1.0
   ```

   若存在360P或其他分辨率，则会自适应最接近的分辨率使用其参数。clip_ratio_data-480p、clip_ratio_data-720p、clip_ratio_data-1080p、clip_ratio_data-1440p、clip_ratio_data-2160p数据都存在eptz_zoomm.conf即可。

6. 是否支持eptz_zoom.conf文件动态修改？

   支持，可调用changeEptzConfig接口，使用其他eptz_zoon.conf文件来进行参数更新。

7. 其他

   其他EPTZ问题，建议redmine上提交，并复上对应的日志以及视频文件。贵司将安排人员进行分析解决。

   

   
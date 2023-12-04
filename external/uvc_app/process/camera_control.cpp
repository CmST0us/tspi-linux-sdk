/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "camera_control.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <list>

#include "uvc_log.h"
#if EPTZ_ENABLE
#include "eptz_control.h"
#endif

#if USE_RKMEDIA
#include <easymedia/buffer.h>
#include <easymedia/key_string.h>
#include <easymedia/media_config.h>
#include <easymedia/utils.h>

#include <easymedia/flow.h>

#endif

#if USE_ROCKIT
#include <rockit/rt_header.h>
#include <rockit/rt_metadata.h>
#include <rockit/RTUVCGraph.h>
#include <rockit/RTMediaBuffer.h>
#endif
#ifdef __cplusplus
extern "C"
{
#endif

extern void uvc_read_camera_buffer(void *cam_buf, struct MPP_ENC_INFO *info,
                                   void *extra_data, size_t extra_size);
#ifdef __cplusplus
}
#endif

#if USE_RK_AISERVER
#include "uvc_ipc_ext.h"
#include "uvc_data.pb.h"
#endif
struct Camera_Stream
{
    int width;
    int height;
    int fps;
    int format;
    int eptz;
    pthread_mutex_t record_mutex;
    pthread_cond_t record_cond;
#if USE_ROCKIT
    RTUVCGraph *uvc_graph;
    volatile int pthread_run;
#endif
#if USE_RKMEDIA
    std::shared_ptr<easymedia::Flow> input;
    std::shared_ptr<easymedia::Flow> uvc_flow;
    volatile int pthread_run;
    RK_U32 uvc_flow_output;
#endif
    pthread_t record_id;
    int deviceid;
};

#if USE_RKMEDIA
static bool do_uvc(easymedia::Flow *f,
                   easymedia::MediaBufferVector &input_vector);
class UVCJoinFlow : public easymedia::Flow
{
public:
    UVCJoinFlow(uint32_t id);
    virtual ~UVCJoinFlow()
    {
        StopAllThread();
    }

private:
    uint32_t id;
    friend bool do_uvc(easymedia::Flow *f,
                       easymedia::MediaBufferVector &input_vector);
};

UVCJoinFlow::UVCJoinFlow(uint32_t id)
    : id(20)
{
    easymedia::SlotMap sm;
    sm.thread_model = easymedia::Model::ASYNCCOMMON;
    sm.mode_when_full = easymedia::InputMode::DROPFRONT;
    sm.input_slots.push_back(0);
    sm.input_maxcachenum.push_back(2);
    sm.fetch_block.push_back(true);
    if (true)
    {
        sm.input_slots.push_back(1);
        sm.input_maxcachenum.push_back(1);
        sm.fetch_block.push_back(false);
    }
    sm.process = do_uvc;
    if (!InstallSlotMap(sm, "uvc_extract", -1))
    {
        LOG_ERROR( "Fail to InstallSlotMap, %s\n", "uvc_join");
        SetError(-EINVAL);
        return;
    }
}
#endif

#define RT_ALIGN(x, a)  (((x) + (a) - 1) & ~((a) - 1))

static struct Camera_Stream *stream_list = NULL;
static pthread_rwlock_t notelock = PTHREAD_RWLOCK_INITIALIZER;
static std::list<pthread_t> record_id_list;

#if USE_RKMEDIA
bool do_uvc(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector)
{
    UVCJoinFlow *flow = (UVCJoinFlow *)f;
    auto img_buf = input_vector[0];
    if (!img_buf || img_buf->GetType() != Type::Image)
        return false;

    auto img = std::static_pointer_cast<easymedia::ImageBuffer>(img_buf);
    struct MPP_ENC_INFO info;
    info.fd = img_buf->GetFD();
    info.size = img_buf->GetValidSize();
#if UVC_DYNAMIC_DEBUG_USE_TIME
    if (uvc_debug_info.first_frm || !access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
    {
        int32_t use_time_us, now_time_us;
        info.pts =img_buf->GetUSTimeStamp();
        uvc_debug_info.first_frm = false;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000;
        use_time_us = now_time_us - info.pts;
        LOG_INFO("isp->rkmedia->uvc latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
        if (uvc_debug_info.first_frm)
        {
            use_time_us = now_time_us - uvc_debug_info.stream_on_pts;
            LOG_INFO("streamon->isp->rkmedia->uvc latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
        }
    }
#endif


#if UVC_DYNAMIC_DEBUG_FPS
    if (!access(UVC_DYNAMIC_DEBUG_ISP_FPS_CHECK, 0))
    {
        uvc_debug_info.debug_isp_fps = true;
    }
    else if (!access(UVC_DYNAMIC_DEBUG_IPC_FPS_CHECK, 0))
    {
        uvc_debug_info.debug_ipc_fps = true;
        uvc_debug_info.debug_isp_fps = false;
    }
    else
    {
        uvc_debug_info.debug_isp_fps = false;
        uvc_debug_info.debug_ipc_fps = false;
    }
    if (uvc_debug_info.debug_isp_fps || uvc_debug_info.debug_ipc_fps)
    {
        ++uvc_debug_info.isp_frm;
        if (uvc_debug_info.isp_frm == 100)
        {
            struct timeval now_time;
            gettimeofday(&now_time, NULL);
            int64_t use_timems = (now_time.tv_sec * 1000 + now_time.tv_usec / 1000) -
                                 (uvc_debug_info.enter_time.tv_sec * 1000 +
                                  uvc_debug_info.enter_time.tv_usec / 1000);
            uvc_debug_info.enter_time.tv_sec = now_time.tv_sec;
            uvc_debug_info.enter_time.tv_usec = now_time.tv_usec;
            uvc_debug_info.fps = (1000.0 * uvc_debug_info.isp_frm) / use_timems;
            uvc_debug_info.isp_frm = 0;
            LOG_INFO("%s fps:%0.1f\n", uvc_debug_info.debug_isp_fps ? "isp" : "ipc", uvc_debug_info.fps);
        }
    }

    if (!uvc_debug_info.debug_isp_fps)
#endif
    {
        uvc_read_camera_buffer(img_buf->GetPtr(), &info,
                               NULL, 0);
    }
    return true;
}
#endif

#if (USE_RKMEDIA || USE_ROCKIT)
static void camera_control_wait(struct Camera_Stream *stream)
{
    pthread_mutex_lock(&stream->record_mutex);
    if (stream->pthread_run)
        pthread_cond_wait(&stream->record_cond, &stream->record_mutex);
    pthread_mutex_unlock(&stream->record_mutex);
}
#endif

#if USE_ROCKIT
static RT_RET emitted_buffer_to_uvc(RTMediaBuffer *buffer)
{
    struct MPP_ENC_INFO info;
    info.fd = buffer->getFd();
    info.size = buffer->getLength();
#if UVC_DYNAMIC_DEBUG_USE_TIME
    if (uvc_debug_info.first_frm || !access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
    {
        int32_t use_time_us, now_time_us;
        long long int pts;
        buffer->getMetaData()->findInt64(kKeyFramePts, &pts);
        info.pts = pts;
        uvc_debug_info.first_frm = false;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000;
        use_time_us = now_time_us - info.pts;
        LOG_INFO("isp->rockit->uvc latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
        if (uvc_debug_info.first_frm)
        {
            use_time_us = now_time_us - uvc_debug_info.stream_on_pts;
            LOG_INFO("streamon->isp->rockit->uvc latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
        }
        //LOG_INFO("isp:0x%x , uvc:0x%x \n", info.pts, now_time_us);
    }
#endif

#if UVC_DYNAMIC_DEBUG_FPS
    if (!access(UVC_DYNAMIC_DEBUG_ISP_FPS_CHECK, 0))
    {
        uvc_debug_info.debug_isp_fps = true;
    }
    else if (!access(UVC_DYNAMIC_DEBUG_IPC_FPS_CHECK, 0))
    {
        uvc_debug_info.debug_ipc_fps = true;
        uvc_debug_info.debug_isp_fps = false;
    }
    else
    {
        uvc_debug_info.debug_isp_fps = false;
        uvc_debug_info.debug_ipc_fps = false;
    }
	if (uvc_debug_info.debug_isp_fps || uvc_debug_info.debug_ipc_fps)
    {
        ++uvc_debug_info.isp_frm;
        if (uvc_debug_info.isp_frm == 100)
        {
            struct timeval now_time;
            gettimeofday(&now_time, NULL);
            int64_t use_timems = (now_time.tv_sec * 1000 + now_time.tv_usec / 1000) -
                                 (uvc_debug_info.enter_time.tv_sec * 1000 +
                                  uvc_debug_info.enter_time.tv_usec / 1000);
            uvc_debug_info.enter_time.tv_sec = now_time.tv_sec;
            uvc_debug_info.enter_time.tv_usec = now_time.tv_usec;
            uvc_debug_info.fps = (1000.0 * uvc_debug_info.isp_frm) / use_timems;
            uvc_debug_info.isp_frm = 0;
            LOG_INFO("%s fps:%0.1f\n", uvc_debug_info.debug_isp_fps ? "isp" : "ipc", uvc_debug_info.fps);
        }
    }

    if (!uvc_debug_info.debug_isp_fps)
#endif
    {
        uvc_read_camera_buffer(buffer->getData(), &info,
                               NULL, 0);
    }
    buffer->release();
    return RT_OK;
}
#endif

void video_record_signal(struct Camera_Stream *stream)
{
    pthread_mutex_lock(&stream->record_mutex);
#if USE_ROCKIT
    stream->uvc_graph->closeUVC();
    LOG_INFO("closeUVC ok\n");
#endif
#if USE_ROCKIT || USE_RKMEDIA
    stream->pthread_run = 0;
    pthread_cond_signal(&stream->record_cond);
#endif
    pthread_mutex_unlock(&stream->record_mutex);
}

static struct Camera_Stream *getfastvideo(void)
{
    return stream_list;
}

static void camera_stop(struct Camera_Stream *stream)
{
    LOG_INFO("%s \n", __func__);
}

static void *uvc_camera(void *arg)
{
    struct Camera_Stream *stream = (struct Camera_Stream *)arg;
    prctl(PR_SET_NAME, "uvc_camera", 0, 0, 0);
#if 0
#if USE_RK_AISERVER
    int needEPTZ = 0;
    int eptzWidth = 0;
    int eptzHeight = 0;
    char *enableEptz = getenv("ENABLE_EPTZ");
    LOG_INFO("enableEptz=%s", enableEptz);
    if (enableEptz)
    {
        LOG_INFO("%s :uvc eptz use evn setting \n", __FUNCTION__);
        needEPTZ = atoi(enableEptz);
    }
    int need_full_range = 1;
    char *full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range)
    {
        need_full_range = atoi(full_range);
        LOG_INFO("uvc full_range use env setting:%d \n", need_full_range);
    }

    if (needEPTZ)
        uvc_ipc_event(UVC_IPC_EVENT_ENABLE_ETPTZ, (void *)&needEPTZ);

    CAMERA_INFO camera_info;
    camera_info.width = stream->width;
    camera_info.height = stream->height;
    camera_info.vir_width = stream->width;
    camera_info.vir_height = stream->height;
    camera_info.buf_size = stream->width * stream->height * 2;
    camera_info.range = need_full_range;

    uvc_ipc_event(UVC_IPC_EVENT_CONFIG_CAMERA, (void *)&camera_info);
    uvc_ipc_event(UVC_IPC_EVENT_START, NULL); //after the camera config
    //int val = 12; // for test
    //uvc_ipc_event(UVC_IPC_EVENT_SET_ZOOM, (void *)&val);       // for test
#endif
#else
#if USE_ROCKIT
    int needEPTZ = 0;
    int eptzWidth = 0;
    int eptzHeight = 0;
    char* enableEptz = getenv("ENABLE_EPTZ");
    printf("enableEptz=%s", enableEptz);
    if (enableEptz){
        LOG_INFO("uvc eptz use evn setting \n");
        needEPTZ = atoi(enableEptz);
    } else if (stream->eptz){
        LOG_INFO("uvc eptz use xu setting:%d\n", stream->eptz);
        needEPTZ = stream->eptz;
    }
    int need_full_range = 1;
    char* full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range) {
        need_full_range = atoi(full_range);
        LOG_INFO("uvc full_range use env setting:%d \n",need_full_range);
    }

    if (needEPTZ) {
        stream->uvc_graph = new RTUVCGraph("eptz");
    } else {
        stream->uvc_graph = new RTUVCGraph("uvc");
    }

    stream->uvc_graph->observeUVCOutputStream(&emitted_buffer_to_uvc);
    RtMetaData cameraParams;
    cameraParams.setInt32("opt_width",        stream->width);
    cameraParams.setInt32("opt_height",       stream->height);
    cameraParams.setInt32("opt_vir_width",    stream->width);
    cameraParams.setInt32("opt_vir_height",   stream->height);
    cameraParams.setInt32("node_buff_size",   stream->width * stream->height * 2);
    if ((stream->format == V4L2_PIX_FMT_H264) && (need_full_range == 0)) {
       LOG_INFO("stream->format:V4L2_PIX_FMT_H264,use V4L2_QUANTIZATION_LIM_RANGE!!");
       cameraParams.setInt32("opt_quantization", V4L2_QUANTIZATION_LIM_RANGE);
    }
    stream->uvc_graph->updateCameraParams(&cameraParams);
    stream->uvc_graph->prepare();
    stream->uvc_graph->openUVC();
    stream->uvc_graph->enableEPTZ(needEPTZ);
    while (stream->pthread_run) {
#if CAMERA_CONTROL_PAN_TILT_ZOOM_DEBUG
        char check_test[20] = {0};
        int test_val;
        sleep(1);
        if (!access(CAMERA_CONTROL_PAN_LEFT_DEBUG_CHECK, 0)) {
            sprintf(check_test, "rm %s", CAMERA_CONTROL_PAN_LEFT_DEBUG_CHECK);
            system(check_test);
            camera_control_set_pan(-1);
        } else if (!access(CAMERA_CONTROL_PAN_RIGHT_DEBUG_CHECK, 0)) {
            sprintf(check_test, "rm %s", CAMERA_CONTROL_PAN_RIGHT_DEBUG_CHECK);
            system(check_test);
            camera_control_set_pan(1);
        } else if (!access(CAMERA_CONTROL_PAN_RANDOM_DEBUG_CHECK, 0)) {
            sprintf(check_test, "rm %s", CAMERA_CONTROL_PAN_RANDOM_DEBUG_CHECK);
            system(check_test);
            test_val = (random() % 21) - 10;
            camera_control_set_pan(test_val);
        } else if (!access(CAMERA_CONTROL_TILT_RANDOM_DEBUG_CHECK, 0)) {
            sprintf(check_test, "rm %s", CAMERA_CONTROL_TILT_RANDOM_DEBUG_CHECK);
            system(check_test);
            test_val = (random() % 21) - 10;
            camera_control_set_tilt(test_val);
        } else if (!access(CAMERA_CONTROL_ZOOM_RANDOM_DEBUG_CHECK, 0)) {
            sprintf(check_test, "rm %s", CAMERA_CONTROL_ZOOM_RANDOM_DEBUG_CHECK);
            system(check_test);
            test_val = ((random() % 5) + 1) * 10;
            camera_control_set_zoom(test_val);
        }
#else
        camera_control_wait(stream);
#endif
    }
    goto record_exit;
#endif
#if USE_RKMEDIA
    int needEPTZ = 0;
    int eptz_width = 0;
    int eptz_height = 0;
    int needRGA = 0;
    int rga_width = 0;
    int rga_height = 0;
    needEPTZ = stream->eptz;
    char* enable_eptz = getenv("ENABLE_EPTZ");
    if(enable_eptz && !needEPTZ){
      LOG_INFO("%s :uvc eptz use env setting \n",enable_eptz);
      needEPTZ = atoi(enable_eptz);
    }
    if(stream->height < 480 && !needEPTZ){
      LOG_INFO("usb RGA for isp resolusion \n");
      rga_width = stream->width;
      rga_height = stream->height;
      stream->height = 720;
      stream->width = 1280;
      needRGA = 1;
    }
    LOG_INFO("uvc width:%d,height:%d, needEPTZ %d, needRGA %d \n",stream->width,stream->height, needEPTZ, needRGA);
#if EPTZ_ENABLE
    if(needEPTZ && !needRGA){
      eptz_width = stream->width;
      eptz_height = stream->height;
      if(eptz_width > 1920 || eptz_height > 1080){
        needEPTZ = 0;
        LOG_ERROR("needEPTZ, not support this width(>1920) and height(>1080) \n");
      }else if(eptz_width == 1920){
        char* max_width = getenv("CAMERA_MAX_WIDTH");
        char* max_height = getenv("CAMERA_MAX_HEIGHT");
        if(max_width && max_height){
          int tmp_width = atoi(max_width);
          int tmp_height = atoi(max_height);
          LOG_INFO("needEPTZ, camera max resolutio [%d %d] \n", tmp_width, tmp_height);
          if(tmp_width > 1920 && tmp_height > 1080){
            stream->width = tmp_width;
            stream->height = tmp_height;
          }else{
            needEPTZ = 0;
            LOG_ERROR("needEPTZ, but camera max resolutio not support! error \n");
          }
        }else{
          stream->width = 2560;
          stream->height = 1440;
        }
      }else if(eptz_width <= 1280 && eptz_width >120 ){
        stream->width = eptz_width * 1.5;
        stream->height = eptz_height * 1.5;
      }else {
        needEPTZ = 0;
        LOG_ERROR("needEPTZ, match fail \n");
      }
      if(needEPTZ)
        LOG_INFO("needEPTZ uvc width:%d,height:%d \n",stream->width,stream->height);
    }
#endif
    std::shared_ptr<easymedia::Flow> video_rga_flow=NULL;
    std::shared_ptr<easymedia::Flow> video_save_flow=NULL;
   // std::string input_path = "/dev/video0";
    std::string input_path = "rkispp_scale0";//"/dev/video0"; //isp main path
    std::string input_format = IMAGE_NV12;
    int need_full_range = 1;
    char* full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range) {
        need_full_range = atoi(full_range);
        LOG_INFO("uvc full_range use env setting:%d \n",need_full_range);
    }

    //Reading yuv from camera
    std::string flow_name = "source_stream";
    std::string flow_param = "";

    PARAM_STRING_APPEND(flow_param, KEY_NAME, "v4l2_capture_stream");
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_SYNC);
    PARAM_STRING_APPEND(flow_param, KEK_INPUT_MODEL, KEY_DROPFRONT);
    PARAM_STRING_APPEND_TO(flow_param, KEY_INPUT_CACHE_NUM, 5);
    std::string stream_param = "";
    PARAM_STRING_APPEND_TO(stream_param, KEY_USE_LIBV4L2, 1);
    PARAM_STRING_APPEND(stream_param, KEY_DEVICE, input_path);
    // PARAM_STRING_APPEND(param, KEY_SUB_DEVICE, sub_input_path);
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_CAP_TYPE, KEY_V4L2_C_TYPE(VIDEO_CAPTURE));
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_MEM_TYPE, KEY_V4L2_M_TYPE(MEMORY_DMABUF));
    PARAM_STRING_APPEND_TO(stream_param, KEY_FRAMES, 3); // if not set, default is 2
    PARAM_STRING_APPEND(stream_param, KEY_OUTPUTDATATYPE, input_format);
    if ((stream->format == V4L2_PIX_FMT_H264) && (need_full_range == 0)) {
       LOG_INFO("stream->format:V4L2_PIX_FMT_H264,use V4L2_QUANTIZATION_LIM_RANGE!!");
       PARAM_STRING_APPEND_TO(stream_param, KEY_V4L2_QUANTIZATION, V4L2_QUANTIZATION_LIM_RANGE);
    }
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_WIDTH, stream->width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_HEIGHT, stream->height);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_WIDTH, stream->width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_HEIGHT, stream->height);

    flow_param = easymedia::JoinFlowParam(flow_param, 1, stream_param);
    LOG_INFO("\n#VideoCapture flow param:\n%s\n", flow_param.c_str());
    stream->input = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
        flow_name.c_str(), flow_param.c_str());
    if (!stream->input)
    {
        LOG_ERROR( "Create flow %s failed\n", flow_name.c_str());
        goto record_exit;
    }

    mpi_get_env_u32("uvc_flow_output", &stream->uvc_flow_output, 0);
    if (stream->uvc_flow_output) {
        // test dump
        std::string output_path = "/data/uvc_flow_output_nv12.yuv";
        flow_name = "file_write_flow";
        flow_param = "";
        PARAM_STRING_APPEND(flow_param, KEY_PATH, output_path.c_str());
        PARAM_STRING_APPEND(flow_param, KEY_OPEN_MODE, "w+"); // read and close-on-exec
        LOG_INFO("\n#FileWrite:\n%s\n", flow_param.c_str());
        video_save_flow = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
            flow_name.c_str(), flow_param.c_str());
        if (!video_save_flow)
        {
            LOG_ERROR( "Create flow video_save_flow failed\n");
            goto record_exit;
        }
        stream->input->AddDownFlow(video_save_flow, 0, 0);
    } else {
        stream->uvc_flow = std::make_shared<UVCJoinFlow>(stream->deviceid);
        if (!stream->uvc_flow)
        {
            LOG_ERROR( "Create flow UVCJoinFlow failed\n");
            goto record_exit;
        }
        if(needRGA) {
             flow_name = "filter";
             flow_param = "";
             PARAM_STRING_APPEND(flow_param, KEY_NAME, "rkrga");
             PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, input_format);
             //Set output buffer type.
             PARAM_STRING_APPEND(flow_param, KEY_OUTPUTDATATYPE, input_format);
             //Set output buffer size.
             PARAM_STRING_APPEND_TO(flow_param, KEY_BUFFER_WIDTH, rga_width);
             PARAM_STRING_APPEND_TO(flow_param, KEY_BUFFER_HEIGHT, rga_height);
             PARAM_STRING_APPEND_TO(flow_param, KEY_BUFFER_VIR_WIDTH, rga_width);
             PARAM_STRING_APPEND_TO(flow_param, KEY_BUFFER_VIR_HEIGHT, rga_height);
             std::string filter_param = "";
             ImageRect src_rect = {0, 0, stream->width, stream->height};
             ImageRect dst_rect = {0, 0, rga_width, rga_height};
             std::vector<ImageRect> rect_vect;
             rect_vect.push_back(src_rect);
             rect_vect.push_back(dst_rect);
             PARAM_STRING_APPEND(filter_param, KEY_BUFFER_RECT,
             easymedia::TwoImageRectToString(rect_vect).c_str());
             PARAM_STRING_APPEND_TO(filter_param, KEY_BUFFER_ROTATE, 0);
             flow_param = easymedia::JoinFlowParam(flow_param, 1, filter_param);
             LOG_INFO("\n#Rkrga Filter flow param:\n%s\n", flow_param.c_str());
             video_rga_flow = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
             flow_name.c_str(), flow_param.c_str());
             if (!video_rga_flow) {
                 LOG_ERROR( "Create flow %s failed\n", flow_name.c_str());
                 goto record_exit;
              }
              video_rga_flow->AddDownFlow(stream->uvc_flow, 0, 0);
              stream->input->AddDownFlow(video_rga_flow, 0, 0);
        } else {
              if(needEPTZ){
                  #if EPTZ_ENABLE
                  int ret = eptz_config(stream->width, stream->height, eptz_width, eptz_height);
                  if( ret == -1){
                    LOG_ERROR( "eptz_config failed\n");
                    goto record_exit;
                  }
                  dclip->AddDownFlow(stream->uvc_flow, 0, 0);
                  stream->input->AddDownFlow(dclip, 0, 0);
                  eptz_source->AddDownFlow(rknn, 0, 0);
                  #endif
              }else{
                  #if EPTZ_ENABLE
                  int ret = zoom_config(stream->width, stream->height);
                  if( ret == -1){
                    LOG_ERROR( "zoom_config failed\n");
                    goto record_exit;
                  }
                  zoom->AddDownFlow(stream->uvc_flow, 0, 0);
                  stream->input->AddDownFlow(zoom, 0, 0);
                  #else
                  stream->input->AddDownFlow(stream->uvc_flow, 0, 0);
                  #endif
              }
        }
    }

    LOG_INFO("start,uvc_flow_output=%d\n", stream->uvc_flow_output);
    //system("mediaserver -d -c /oem/usr/share/mediaserver/camera_nv12_rga_nn_link.conf &");

    while (stream->pthread_run)
    {
        camera_control_wait(stream);
    }
    goto record_exit;
#endif
#endif
record_exit:
    LOG_INFO("%s exit\n", __func__);
    pthread_rwlock_wrlock(&notelock);
    //system("killall -9 mediaserver");
    //usleep(500000);//rkisp requst the stream without init aiq close first!
#if USE_RKMEDIA
    if (stream->uvc_flow_output) {
        if (stream->input) {
            if (video_save_flow)
                stream->input->RemoveDownFlow(video_save_flow);
            stream->input.reset();
        }
        if (video_save_flow)
            video_save_flow.reset();
    }
    if (stream->input)
    {
        if (stream->uvc_flow) {
          if (needRGA) {
            stream->input->RemoveDownFlow(video_rga_flow);
            stream->input.reset();
            video_rga_flow->RemoveDownFlow(stream->uvc_flow);
            video_rga_flow.reset();
          } else if (needEPTZ){
            #if EPTZ_ENABLE
            if(eptz_source){
                if(rknn)
                  eptz_source->RemoveDownFlow(rknn);
              eptz_source.reset();
            }
            if(rknn)
              rknn.reset();
            if(dclip)
              stream->input->RemoveDownFlow(dclip);
            stream->input.reset();
            if(dclip){
              dclip->RemoveDownFlow(stream->uvc_flow);
              dclip.reset();
            }
            #endif
          } else {
            #if EPTZ_ENABLE
            stream->input->RemoveDownFlow(zoom);
            stream->input.reset();
            zoom->RemoveDownFlow(stream->uvc_flow);
            zoom.reset();
            #else
            stream->input->RemoveDownFlow(stream->uvc_flow);
            stream->input.reset();
            #endif
          }
        }
        stream->uvc_flow.reset();
    }
#endif
#if USE_ROCKIT
    if (stream->uvc_graph) {
        delete(stream->uvc_graph);
    }
#endif

    pthread_mutex_destroy(&stream->record_mutex);
    pthread_cond_destroy(&stream->record_cond);

    if (stream)
        free(stream);
    pthread_rwlock_unlock(&notelock);

    //camera_stop(stream);
    usleep(500000);//make sure rkispp deint

    stream = NULL;
    if(stream_list)
        stream_list = NULL;
    pthread_exit(NULL);
}

extern "C" int camera_control_start(int id, int width, int height, int fps, int format, int eptz)
{
    struct Camera_Stream *stream;
    int ret = 0;
    if (id < 0)
        return -1;
    LOG_INFO("%s!\n", __func__);
    pthread_rwlock_wrlock(&notelock);

    stream = (struct Camera_Stream *)calloc(1, sizeof(struct Camera_Stream));
    if (!stream)
    {
        LOG_ERROR("no memory!\n");
        goto addvideo_exit;
    }
    pthread_mutex_init(&stream->record_mutex, NULL);
    pthread_cond_init(&stream->record_cond, NULL);

#if USE_ROCKIT
    stream->pthread_run = 1;
#endif

#if USE_RKMEDIA
    stream->pthread_run = 1;
    stream->input = NULL;
    stream->uvc_flow = NULL;
#endif
    stream->fps = fps;
    stream->width = width;
    stream->height = height;
    stream->format = format;
    stream->deviceid = id;
    stream->eptz = eptz;

    LOG_DEBUG("stream%d is uvc video device\n", stream->deviceid);
    while (stream_list) {
        LOG_DEBUG("%s wait for release stream_list!\n", __func__);
        usleep(100000);//wait for next
    }
    stream_list = stream;
    if (pthread_create(&stream->record_id, NULL, uvc_camera, stream))
    {
        LOG_ERROR("%s pthread create err!\n", __func__);
        goto addvideo_exit;
    }
    record_id_list.push_back(stream->record_id);
    ret = 0;
    goto addvideo_ret;

addvideo_exit:

    if (stream)
    {
#if USE_RKMEDIA
        if (stream->input)
        {
            if (stream->uvc_flow)
                stream->input->RemoveDownFlow(stream->uvc_flow);
            stream->input.reset();
        }
        if (stream->uvc_flow)
            stream->uvc_flow.reset();
#endif
        pthread_mutex_destroy(&stream->record_mutex);
        pthread_cond_destroy(&stream->record_cond);
#if USE_RKMEDIA
        stream->input = NULL;
        stream->uvc_flow = NULL;
#endif
        free(stream);
        stream = NULL;
    }

    LOG_DEBUG("stream%d exit!\n", id);
    ret = -1;

addvideo_ret:
    LOG_INFO("%s!end\n", __func__);
    pthread_rwlock_unlock(&notelock);
    return ret;
}

extern "C" int camera_control_stop(int deviceid)
{
    struct Camera_Stream *stream;

    stream = getfastvideo();
    if(!stream)
       usleep(200000);//wait for stream on over
    pthread_rwlock_rdlock(&notelock);
    while (stream)
    {
        if (stream->deviceid == deviceid)
        {
            video_record_signal(stream);
            break;
        }
    }
    pthread_rwlock_unlock(&notelock);

    return 0;
}

extern "C" void camera_control_init()
{
    //todo
}

extern "C" void camera_control_set_eptz(int val){
    if (val > 0)
       val = 1;
     else
       val = 0;
#if USE_RK_AISERVER
    uvc_ipc_event(UVC_IPC_EVENT_ENABLE_ETPTZ, (void *)&val);
#endif

#if USE_ROCKIT
    if (stream_list) {
       if (stream_list->uvc_graph) {
           stream_list->uvc_graph->enableEPTZ(val);
       }
    }
#endif
}

extern "C" void camera_control_set_zoom(int val)
{
    LOG_INFO("set_zoom:%d\n",val);
#if USE_ROCKIT
    if (stream_list) {
       if (stream_list->uvc_graph) {
           stream_list->uvc_graph->setZoom((float)val/10);
       }
    }
#else
    #if EPTZ_ENABLE
    set_zoom((float)val/10);
    #endif
#endif
#if USE_RK_AISERVER
    uvc_ipc_event(UVC_IPC_EVENT_SET_ZOOM, (void *)&val);
#endif
}

extern "C" void camera_control_set_pan(int val)
{
    LOG_INFO("set_pan:%d\n",val);
#if USE_ROCKIT
    if (stream_list) {
        if (stream_list->uvc_graph) {
            stream_list->uvc_graph->setEptz(RT_EPTZ_PAN, val);
        }
    }
#endif
#if USE_RK_AISERVER
    uvc_ipc_event(UVC_IPC_EVENT_SET_EPTZ_PAN, (void *)&val);
#endif

}

extern "C" void camera_control_set_tilt(int val)
{
    LOG_INFO("set_tilt:%d\n",val);
#if USE_ROCKIT
    if (stream_list) {
        if (stream_list->uvc_graph) {
            stream_list->uvc_graph->setEptz(RT_EPTZ_TILT, val);
        }
    }
#endif
#if USE_RK_AISERVER
    uvc_ipc_event(UVC_IPC_EVENT_SET_EPTZ_TILT, (void *)&val);
#endif

}

extern "C" void camera_control_set_roll(int val)
{
    LOG_INFO("set_roll:%d\n",val);
    //todo
}

extern "C" void camera_control_deinit()
{
    LOG_INFO("%s!start\n", __func__);
    struct Camera_Stream *stream;
    std::list<pthread_t> save_list;

    pthread_rwlock_wrlock(&notelock);
    stream = getfastvideo();
    while (stream)
    {
        video_record_signal(stream);
        break;
    }
    save_list.clear();
    for (std::list<pthread_t>::iterator it = record_id_list.begin();
         it != record_id_list.end(); ++it)
        save_list.push_back(*it);
    record_id_list.clear();
    pthread_rwlock_unlock(&notelock);
    for (std::list<pthread_t>::iterator it = save_list.begin();
         it != save_list.end(); ++it)
    {
        LOG_INFO("pthread_join record id: %lu\n", *it);
        pthread_join(*it, NULL);
    }
    save_list.clear();
    LOG_INFO("%s!end\n", __func__);
}

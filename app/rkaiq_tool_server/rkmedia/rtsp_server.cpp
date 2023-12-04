#include "rtsp_server.h"

#include <iostream>
#include <memory>
#include <string>

#include "assert.h"
#include "signal.h"
#include "stdint.h"
#include "stdio.h"
#include "unistd.h"

static std::shared_ptr<easymedia::Flow> video_cap_flow = nullptr;
std::shared_ptr<easymedia::Flow> g_video_enc_flow = nullptr;
std::shared_ptr<easymedia::Flow> g_video_rtsp_flow = nullptr;
std::shared_ptr<easymedia::Flow> video_dump_flow = nullptr;

std::shared_ptr<easymedia::Flow> create_flow(const std::string& flow_name, const std::string& flow_param,
                                             const std::string& elem_param)
{
    auto&& param = easymedia::JoinFlowParam(flow_param, 1, elem_param);
    // printf("create_flow :\n");
    // printf("flow_name : %s\n", flow_name.c_str());
    // printf("param : \n%s\n", param.c_str());
    auto ret = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(flow_name.c_str(), param.c_str());
    printf(" ####### create_flow flow use_count %d\n", ret.use_count());
    if (!ret) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
    }
    return ret;
}

std::string get_video_cap_flow_param(std::string input_path, std::string pixel_format, int video_width,
                                     int video_height)
{
    std::string flow_param;
    // Reading yuv from camera
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_NAME, "v4l2_capture_stream");
    // PARAM_STRING_APPEND_TO(flow_param, KEY_FPS, video_fps);
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_SYNC);
    PARAM_STRING_APPEND(flow_param, KEK_INPUT_MODEL, KEY_DROPFRONT);
    PARAM_STRING_APPEND_TO(flow_param, KEY_INPUT_CACHE_NUM, 5);
    return flow_param;
}

std::string get_video_cap_stream_param(std::string input_path, std::string pixel_format, int video_width,
                                       int video_height)
{
    std::string stream_param;
    stream_param = "";
    PARAM_STRING_APPEND_TO(stream_param, KEY_USE_LIBV4L2, 1);
    PARAM_STRING_APPEND(stream_param, KEY_DEVICE, input_path);
    // PARAM_STRING_APPEND(param, KEY_SUB_DEVICE, sub_input_path);
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_CAP_TYPE, KEY_V4L2_C_TYPE(VIDEO_CAPTURE));
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_MEM_TYPE, KEY_V4L2_M_TYPE(MEMORY_MMAP));
    PARAM_STRING_APPEND_TO(stream_param, KEY_FRAMES, 4);
    PARAM_STRING_APPEND(stream_param, KEY_OUTPUTDATATYPE, pixel_format);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_WIDTH, video_width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_HEIGHT, video_height);
    return stream_param;
}

std::shared_ptr<easymedia::Flow> create_video_capture_flow(std::string input_path, std::string pixel_format,
                                                           int video_width, int video_height)
{
    std::string flow_name;
    std::string flow_param;
    std::string stream_param;
    std::shared_ptr<easymedia::Flow> video_read_flow;

    flow_name = "source_flow";
    flow_param = get_video_cap_flow_param(input_path, pixel_format, video_width, video_height);
    stream_param = get_video_cap_stream_param(input_path, pixel_format, video_width, video_height);
    video_read_flow = create_flow(flow_name, flow_param, stream_param);
    if (!video_read_flow) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }
    return video_read_flow;
}

std::string get_video_enc_flow_param(std::string pixel_format, std::string video_enc_type, int video_width,
                                     int video_height, int video_fps)
{
    std::string flow_param;
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_NAME, "rkmpp");
    PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, pixel_format);
    PARAM_STRING_APPEND(flow_param, KEY_OUTPUTDATATYPE, video_enc_type);
    return flow_param;
}

std::string get_video_enc_stream_param(std::string pixel_format, std::string video_enc_type, int video_width,
                                       int video_height, int video_fps)
{
    std::string stream_param;

    int bps = video_width * video_height * video_fps / 14;
    stream_param = "";
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_WIDTH, video_width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_HEIGHT, video_height);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_WIDTH, video_width);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_HEIGHT, video_height);
    PARAM_STRING_APPEND_TO(stream_param, KEY_COMPRESS_BITRATE, bps);
    PARAM_STRING_APPEND_TO(stream_param, KEY_COMPRESS_BITRATE_MAX, bps * 17 / 16);
    PARAM_STRING_APPEND_TO(stream_param, KEY_COMPRESS_BITRATE_MIN, bps / 16);
    PARAM_STRING_APPEND(stream_param, KEY_FPS, "30/0");
    PARAM_STRING_APPEND(stream_param, KEY_FPS_IN, "30/0");
    PARAM_STRING_APPEND_TO(stream_param, KEY_FULL_RANGE, 1);
    return stream_param;
}

std::shared_ptr<easymedia::Flow> create_video_enc_flow(std::string pixel_format, std::string video_enc_type,
                                                       int video_width, int video_height, int video_fps)
{
    std::shared_ptr<easymedia::Flow> video_encoder_flow;

    std::string flow_name;
    std::string flow_param;
    std::string stream_param;

    flow_name = "video_enc";
    flow_param = get_video_enc_flow_param(pixel_format, video_enc_type, video_width, video_height, video_fps);
    stream_param = get_video_enc_stream_param(pixel_format, video_enc_type, video_width, video_height, video_fps);
    video_encoder_flow = create_flow(flow_name, flow_param, stream_param);
    if (!video_encoder_flow) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }
    return video_encoder_flow;
}

std::string get_rtsp_server_flow_param(std::string channel_name, std::string media_type)
{
    std::string flow_param;
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, media_type);
    PARAM_STRING_APPEND(flow_param, KEY_CHANNEL_NAME, channel_name);
    PARAM_STRING_APPEND_TO(flow_param, KEY_PORT_NUM, 554);
    return flow_param;
}

std::string get_rtsp_server_stream_param(std::string channel_name, std::string media_type)
{
    std::string stream_param;
    stream_param = "";
    return stream_param;
}

std::shared_ptr<easymedia::Flow> create_rtsp_server_flow(std::string channel_name, std::string media_type)
{
    std::shared_ptr<easymedia::Flow> rtsp_flow;

    std::string flow_name;
    std::string flow_param;
    std::string stream_param;

    flow_name = "live555_rtsp_server";
    flow_param = get_rtsp_server_flow_param(channel_name, media_type);
    stream_param = get_rtsp_server_stream_param(channel_name, media_type);

    rtsp_flow = create_flow(flow_name, flow_param, stream_param);
    if (!rtsp_flow) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }
    return rtsp_flow;
}

std::string get_rtmp_server_flow_param(std::string stream_addr, std::string output_type)
{
    std::string flow_param;
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_NAME, "muxer_flow");
    PARAM_STRING_APPEND(flow_param, KEY_PATH, stream_addr);
    PARAM_STRING_APPEND(flow_param, KEY_OUTPUTDATATYPE, output_type);
    return flow_param;
}

std::string get_rtmp_server_stream_param(std::string channel_name, std::string media_type)
{
    std::string stream_param;
    stream_param = "";

    return stream_param;
}

std::shared_ptr<easymedia::Flow> create_rtmp_server_flow(std::string stream_addr, std::string output_type,
                                                         std::string video_param, std::string audio_param)
{
    std::shared_ptr<easymedia::Flow> rtmp_flow;
    std::string flow_name;
    std::string flow_param;
    std::string muxer_param;

    flow_name = "muxer_flow";
    flow_param = get_rtmp_server_flow_param(stream_addr, output_type);
    muxer_param = easymedia::JoinFlowParam(flow_param, 2, video_param, audio_param);

    rtmp_flow = create_flow(flow_name, flow_param, muxer_param);
    if (!rtmp_flow) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }

    return rtmp_flow;
}

std::string get_dump_flow_param(std::string input_type)
{
    std::string flow_param;
    flow_param = "";
    PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, input_type);
    return flow_param;
}

std::shared_ptr<easymedia::Flow> create_dump_server_flow(std::string input_type)
{
    std::shared_ptr<easymedia::Flow> dump_flow;
    std::string flow_name;
    std::string flow_param;

    flow_name = "link_flow";
    flow_param = get_dump_flow_param(input_type);
    dump_flow = create_flow(flow_name, flow_param, "");
    if (!dump_flow) {
        fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
        exit(EXIT_FAILURE);
    }

    return dump_flow;
}

void deinit_rtsp()
{
    if (video_cap_flow && video_dump_flow) {
        video_cap_flow->RemoveDownFlow(video_dump_flow);
    }
    if (video_cap_flow && g_video_enc_flow) {
        video_cap_flow->RemoveDownFlow(g_video_enc_flow);
    }
    if (g_video_rtsp_flow && g_video_enc_flow) {
        g_video_enc_flow->RemoveDownFlow(g_video_rtsp_flow);
    }
    if (g_video_rtsp_flow) {
        g_video_rtsp_flow.reset();
    }
    if (g_video_enc_flow) {
        g_video_enc_flow.reset();
    }
    if (video_cap_flow) {
        video_cap_flow.reset();
    }
    g_video_rtsp_flow = nullptr;
    g_video_enc_flow = nullptr;
    video_cap_flow = nullptr;
}

int init_rtsp(const char* video_dev, int width, int height, std::string enc_type)
{
    deinit_rtsp();

    int fps = 30;
    std::string yuv_format = IMAGE_NV12;
    printf("init_rtsp video_dev %s\n", video_dev);

    easymedia::REFLECTOR(Flow)::DumpFactories();
    easymedia::REFLECTOR(Stream)::DumpFactories();

    video_cap_flow = create_video_capture_flow(video_dev, yuv_format, width, height);
    g_video_enc_flow = create_video_enc_flow(yuv_format, enc_type, width, height, fps);
    g_video_rtsp_flow = create_rtsp_server_flow("main_stream", enc_type);
    video_dump_flow = create_dump_server_flow(yuv_format);

    g_video_enc_flow->AddDownFlow(g_video_rtsp_flow, 0, 0);
    video_cap_flow->AddDownFlow(g_video_enc_flow, 0, 0);
    video_cap_flow->AddDownFlow(video_dump_flow, 0, 0);

    if (!video_cap_flow || !video_dump_flow) {
        return -1;
    }
    return 0;
}

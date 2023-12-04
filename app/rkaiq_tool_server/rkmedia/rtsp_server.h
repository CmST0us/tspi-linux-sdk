#ifndef _RKAIQ_RTSP_SERVER_H__
#define _RKAIQ_RTSP_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "easymedia/buffer.h"
#include "easymedia/flow.h"
#include "easymedia/image.h"
#include "easymedia/key_string.h"
#include "easymedia/media_config.h"
#include "easymedia/media_type.h"
#include "easymedia/reflector.h"
#include "easymedia/stream.h"
#include "easymedia/utils.h"

std::shared_ptr<easymedia::Flow> create_video_capture_flow(std::string input_path, std::string pixel_format,
                                                           int video_width, int video_height);
std::shared_ptr<easymedia::Flow> create_video_enc_flow(std::string pixel_format, std::string video_enc_type,
                                                       int video_width, int video_height, int video_fps);
std::shared_ptr<easymedia::Flow> create_rtsp_server_flow(std::string channel_name, std::string media_type);

void deinit_rtsp();
int init_rtsp(const char* video_dev, int width, int height, std::string enc_type = VIDEO_H264);

#endif

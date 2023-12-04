#include <stdio.h>
#include <unistd.h>
#include "uvc_control.h"
#include "uvc_video.h"
#include "uvc_log.h"
#include <signal.h>
#ifdef CAMERA_CONTROL
#include "camera_control.h"
#endif

#include "mpi_enc.h"
#include "uevent.h"
#include "drm.h"

enum {
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG
};

#define LOG_TAG "uvc_app"
#define UVC_LOG_DYNAMIC_DEBUG "/tmp/uvc_log_debug"

#define ALIGN(size, align) ((size + align - 1) & (~(align - 1)))

#define UVC_VERSION "SDK V1.40"

void sigterm_handler(int sig) {
  LOG_INFO("signal %d\n", sig);
  app_quit = sig;
}

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    unsigned int handle;
    char *buffer;
    int handle_fd;
    size_t size;
    int i = 0;
    int width, height;
    int y, uv;
    int extra_cnt = 0;
    uint32_t flags = 0;
    int media_set = 0;

    enable_minilog = 0;
    uvc_app_log_level = LOG_INFO;
    app_quit = 0;

#if USE_RK_AISERVER
    media_set = 0x01;
#endif
#if USE_ROCKIT
    media_set = 0x02;
#endif
#if USE_RKMEDIA
    media_set = 0x04;
#endif
    LOG_INFO("VERSION:%s %s %s media_set:0x%x\n", UVC_VERSION, __DATE__, __TIME__, media_set);
#ifdef ENABLE_MINILOGGER
    enable_minilog = 1;
    __minilog_log_init(argv[0], NULL, false, true, argv[0],"1.0.0");
#endif
   if (!access(UVC_LOG_DYNAMIC_DEBUG, 0))
   {
       uvc_app_log_level = LOG_DEBUG;
   }
   char *log_level = getenv("uvc_app_log_level");
   if (log_level)
   {
       LOG_INFO("uvc_app_log_level=%d", atoi(log_level));
       uvc_app_log_level = atoi(log_level);
   }

#if (RK_MPP_ENC_TEST_NATIVE == 0)
#ifdef CAMERA_CONTROL
    if (argc != 3)
    {
        signal(SIGQUIT, sigterm_handler);
        signal(SIGTERM, sigterm_handler);
        LOG_DEBUG("uvc_app loop from v4l2.\n");
        camera_control_init();
        uvc_control_start_setcallback(camera_control_start);
        uvc_control_stop_setcallback(camera_control_deinit);
        uevent_monitor_run(UVC_CONTROL_CAMERA);
        //system("uvc_config.sh");
        uvc_control_run(UVC_CONTROL_CAMERA);
        while (1)
        {
            if(0 == uvc_control_loop())
                break;
            usleep(100000);
        }
        uvc_video_id_exit_all();
        camera_control_deinit();
        LOG_INFO("uvc_app exit.\n");
        return 0;
    }
#else
   if (argc != 3) {
     LOG_WARN("please select true control mode!!\n");
     return 0;
   }
#endif
#endif

    if (argc < 3) {
        LOG_WARN("Usage: uvc_app width height [test_file.nv12]\n");
        LOG_WARN("e.g. uvc_app 640 480 [test_file.nv12]\n");
        return -1;
    }
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    FILE *test_file = NULL;
    if (width == 0 || height == 0) {
        LOG_WARN("Usage: uvc_app width height [test_file.nv12]\n");
        LOG_WARN("e.g. uvc_app 640 480 [test_file.nv12]\n");
        return -1;
    }

    fd = drm_open();
    if (fd < 0)
        return -1;

    size = width * height * 3 / 2;
    ret = drm_alloc(fd, size, 16, &handle, 0);
    if (ret)
        return -1;
    LOG_DEBUG("size:%d", size);
    ret = drm_handle_to_fd(fd, handle, &handle_fd, 0);
    if (ret)
        return -1;

    buffer = (char *)drm_map_buffer(fd, handle, size);
    if (!buffer)
    {
        LOG_ERROR("drm map buffer fail.\n");
        return -1;
    }

    if (argc == 4) {
        test_file = fopen(argv[3], "r+b");
        if (!test_file) {
            LOG_ERROR("open %s fail.\n", argv[3]);
            return -1;
        }
    } else {
        y = width * height / 4;
        memset(buffer, 128, y);
        memset(buffer + y, 64, y);
        memset(buffer + y * 2, 128, y);
        memset(buffer + y * 3, 192, y);
        uv = width * height / 8;
        memset(buffer + y * 4, 0, uv);
        memset(buffer + y * 4 + uv, 64, uv);
        memset(buffer + y * 4 + uv * 2, 128, uv);
        memset(buffer + y * 4 + uv * 3, 192, uv);
    }

    flags = UVC_CONTROL_LOOP_ONCE;
#if RK_MPP_ENC_TEST_NATIVE
    uvc_encode_init(&uvc_enc, width, height, TEST_ENC_TPYE);
#else
    uvc_control_run(flags);
#endif
    MPP_ENC_INFO_DEF info;
    while (1)
    {
        if (test_file) {
            if(feof(test_file)) {
                rewind(test_file);
            }
            size = fread(buffer, 1, width * height * 3 / 2, test_file);
        }
        info.fd = handle_fd;
        info.size = size;
        extra_cnt++;
        uvc_read_camera_buffer(buffer, &info, &extra_cnt, sizeof(extra_cnt));
        usleep(30000);
    }
    if (test_file)
         fclose(test_file);

    uvc_control_join(flags);

    drm_unmap_buffer(buffer, size);
    close(handle_fd);
    drm_free(fd, handle);
    drm_close(fd);
    return 0;
}

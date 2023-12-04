#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <ctime>
#include <thread>

#include "camera_infohw.h"
#include "domain_tcp_client.h"
#include "rk-camera-module.h"
#if 0
    #include "rkaiq_manager.h"
#endif
#include "rkaiq_protocol.h"
#include "rkaiq_socket.h"
#include "tcp_server.h"
#ifdef __ANDROID__
    #include <rtspserver/RtspServer.h>
    #include <cutils/properties.h>
    #include "RtspServer.h"
#endif

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

DomainTCPClient g_tcpClient;
struct ucred* g_aiqCred = nullptr;
std::atomic_bool quit{false};
std::atomic<int> g_app_run_mode(APP_RUN_STATUS_INIT);
int g_width = 1920;
int g_height = 1080;
int g_device_id = 0;
uint32_t g_mmapNumber = 4;
uint32_t g_offlineFrameRate = 10;
int g_rtsp_en = 0;
int g_rtsp_en_from_cmdarg = 0;
int g_allow_killapp = 0;
int g_cam_count = 0;
uint32_t g_sensorHdrMode = 0;

std::string g_stream_dev_name;
std::string iqfile;
std::string g_sensor_name;

std::shared_ptr<TCPServer> tcpServer = nullptr;
#if 0
std::shared_ptr<RKAiqToolManager> rkaiq_manager;
#endif
std::shared_ptr<RKAiqMedia> rkaiq_media;

void signal_handle(int sig)
{
    quit.store(true, std::memory_order_release);
    if (tcpServer != nullptr)
        tcpServer->SaveExit();

    RKAiqProtocol::Exit();

    if (g_rtsp_en)
        deinit_rtsp();
}

static int get_env(const char* name, int* value, int default_value)
{
    char* ptr = getenv(name);
    if (NULL == ptr) {
        *value = default_value;
    } else {
        char* endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        errno = 0;
        *value = strtoul(ptr, &endptr, base);
        if (errno || (ptr == endptr)) {
            errno = 0;
            *value = default_value;
        }
    }
    return 0;
}

static const char short_options[] = "s:r:i:m:Dd:w:h:n:f:";
static const struct option long_options[] = {{"stream_dev", required_argument, NULL, 's'},
                                             {"enable_rtsp", required_argument, NULL, 'r'},
                                             {"iqfile", required_argument, NULL, 'i'},
                                             {"mode", required_argument, NULL, 'm'},
                                             {"width", no_argument, NULL, 'w'},
                                             {"height", no_argument, NULL, 'h'},
                                             {"device_id", required_argument, NULL, 'd'},
                                             {"mmap_buffer", required_argument, NULL, 'n'},
                                             {"frame_rate", required_argument, NULL, 'f'},
                                             {"help", no_argument, NULL, 'h'},
                                             {0, 0, 0, 0}};

static int parse_args(int argc, char** argv)
{
    int ret = 0;
    for (;;) {
        int idx;
        int c;
        c = getopt_long(argc, argv, short_options, long_options, &idx);
        if (-1 == c) {
            break;
        }
        switch (c) {
            case 0:
                break;
            case 's':
                g_stream_dev_name = optarg;
                break;
            case 'r':
                g_rtsp_en_from_cmdarg = atoi(optarg);
                if (g_rtsp_en_from_cmdarg != 0 && g_rtsp_en_from_cmdarg != 1) {
                    LOG_ERROR("enable_rtsp arg|only equals 0 or 1\n");
                    ret = 1;
                }
                break;
            case 'i':
                iqfile = optarg;
                break;
            case 'm':
                g_app_run_mode = atoi(optarg);
                break;
            case 'w':
                g_width = atoi(optarg);
                break;
            case 'h':
                g_height = atoi(optarg);
                break;
            case 'd':
                g_device_id = atoi(optarg);
                break;
            case 'n':
                g_mmapNumber = (uint32_t)atoi(optarg);
                if (g_mmapNumber < 4 || g_mmapNumber > 200) {
                    g_mmapNumber = 4;
                    LOG_INFO("mmap Number out of range[4,200], use default 4\n");
                }
                break;
            case 'f':
                g_offlineFrameRate = (uint32_t)atoi(optarg);
                if (g_offlineFrameRate < 1) {
                    g_offlineFrameRate = 1;
                } else if (g_offlineFrameRate > 100) {
                    g_offlineFrameRate = 100;
                }
                LOG_INFO("set framerate:%u\n", g_offlineFrameRate);
                break;

            default:
                break;
        }
    }
    if (iqfile.empty()) {
#ifdef __ANDROID__
        iqfile = "/vendor/etc/camera/rkisp2";
#else
        iqfile = "/oem/etc/iqfiles";
#endif
    }

    return ret;
}

static void RawCaptureinit()
{
    struct capture_info cap_info;
    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
    if (mi.cif.linked_sensor) {
        cap_info.link = link_to_vicap;
        strcpy(cap_info.sd_path.device_name, mi.cif.sensor_subdev_path.c_str());
        strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
        cap_info.dev_name = cap_info.cif_path.cif_video_path;
    } else if (mi.dvp.linked_sensor) {
        cap_info.link = link_to_dvp;
        strcpy(cap_info.sd_path.device_name, mi.dvp.sensor_subdev_path.c_str());
        strcpy(cap_info.cif_path.cif_video_path, mi.dvp.dvp_id0.c_str());
        cap_info.dev_name = cap_info.cif_path.cif_video_path;
    } else {
        cap_info.link = link_to_isp;
        strcpy(cap_info.sd_path.device_name, mi.isp.sensor_subdev_path.c_str());
        strcpy(cap_info.vd_path.isp_main_path, mi.isp.main_path.c_str());
        cap_info.dev_name = cap_info.vd_path.isp_main_path;
    }
    strcpy(cap_info.vd_path.media_dev_path, mi.isp.media_dev_path.c_str());
    strcpy(cap_info.vd_path.isp_sd_path, mi.isp.isp_dev_path.c_str());
    if (mi.lens.module_lens_dev_name.length()) {
        strcpy(cap_info.lens_path.lens_device_name, mi.lens.module_lens_dev_name.c_str());
    } else {
        cap_info.lens_path.lens_device_name[0] = '\0';
    }
    cap_info.dev_fd = -1;
    cap_info.subdev_fd = -1;
    cap_info.lensdev_fd = -1;
    LOG_DEBUG("cap_info.link: %d \n", cap_info.link);
    LOG_DEBUG("cap_info.dev_name: %s \n", cap_info.dev_name);
    LOG_DEBUG("cap_info.isp_media_path: %s \n", cap_info.vd_path.media_dev_path);
    LOG_DEBUG("cap_info.vd_path.isp_sd_path: %s \n", cap_info.vd_path.isp_sd_path);
    LOG_DEBUG("cap_info.sd_path.device_name: %s \n", cap_info.sd_path.device_name);
    LOG_DEBUG("cap_info.lens_path.lens_dev_name: %s \n", cap_info.lens_path.lens_device_name);

    cap_info.io = IO_METHOD_MMAP;
    cap_info.height = g_height;
    cap_info.width = g_width;
    // cap_info.format = v4l2_fourcc('B', 'G', '1', '2');
    LOG_DEBUG("get ResW: %d  ResH: %d\n", cap_info.width, cap_info.height);

    //
    int fd = device_open(cap_info.sd_path.device_name);
    LOG_DEBUG("sensor subdev path: %s\n", cap_info.sd_path.device_name);
    LOG_DEBUG("cap_info.subdev_fd: %d\n", fd);
    if (fd < 0) {
        LOG_ERROR("Open %s failed.\n", cap_info.sd_path.device_name);
    } else {
        rkmodule_hdr_cfg hdrCfg;
        int ret = ioctl(fd, RKMODULE_GET_HDR_CFG, &hdrCfg);
        if (ret > 0) {
            g_sensorHdrMode = NO_HDR;
            LOG_ERROR("Get sensor hdr mode failed, use default, No HDR\n");
        } else {
            g_sensorHdrMode = hdrCfg.hdr_mode;
            LOG_INFO("Get sensor hdr mode:%u\n", g_sensorHdrMode);
        }
    }
    close(fd);
    if (mi.cif.linked_sensor) {
        if (g_sensorHdrMode == NO_HDR) {
            LOG_INFO("Get sensor mode: NO_HDR\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id0.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        } else if (g_sensorHdrMode == HDR_X2) {
            LOG_INFO("Get sensor mode: HDR_2\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id1.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        } else if (g_sensorHdrMode == HDR_X3) {
            LOG_INFO("Get sensor mode: HDR_3\n");
            strcpy(cap_info.cif_path.cif_video_path, mi.cif.mipi_id2.c_str());
            cap_info.dev_name = cap_info.cif_path.cif_video_path;
        }
    }
    //
    fd = open(cap_info.dev_name, O_RDWR, 0);
    LOG_INFO("fd: %d\n", fd);
    if (fd < 0) {
        LOG_ERROR("Open dev %s failed.\n", cap_info.dev_name);
    } else {
        if (g_sensorHdrMode == NO_HDR) {
            int value = CSI_LVDS_MEM_WORD_LOW_ALIGN;
            int ret = ioctl(fd, RKCIF_CMD_SET_CSI_MEMORY_MODE, &value); // set to no compact
            if (ret > 0) {
                LOG_ERROR("set cif node %s compact mode failed.\n", cap_info.dev_name);
            } else {
                LOG_INFO("cif node %s set to no compact mode.\n", cap_info.dev_name);
            }
        } else {
            LOG_INFO("cif node HDR mode, compact format as default.\n");
        }
    }
    close(fd);
}

int main(int argc, char** argv)
{
    int ret = -1;
    LOG_ERROR("#### AIQ tool server v2.0.6-20220215_145026 ####\n");

#ifdef _WIN32
    signal(SIGINT, signal_handle);
    signal(SIGQUIT, signal_handle);
    signal(SIGTERM, signal_handle);
#else
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    struct sigaction new_action, old_action;
    new_action.sa_handler = signal_handle;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);
    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGTERM, &new_action, NULL);
#endif

#ifdef __ANDROID__
    char property_value[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.aiqtool.log", property_value, "5");
    log_level = strtoull(property_value, nullptr, 16);
    property_get("persist.vendor.aiqtool.killapp", property_value, "1");
    g_allow_killapp = strtoull(property_value, nullptr, 16);
    // property_get("persist.vendor.rkisp_no_read_back", property_value, "-1");
    // readback = strtoull(property_value, nullptr, 16);
#else
    get_env("rkaiq_tool_server_log_level", &log_level,
            LOG_LEVEL_INFO); // LOG_LEVEL_ERROR   LOG_LEVEL_WARN  LOG_LEVEL_INFO  LOG_LEVEL_DEBUG
    get_env("rkaiq_tool_server_kill_app", &g_allow_killapp, 0);
#endif

    if (parse_args(argc, argv) != 0) {
        LOG_ERROR("Tool server args parse error.\n");
        return 1;
    }

    LOG_DEBUG("iqfile cmd_parser.get  %s\n", iqfile.c_str());
    LOG_DEBUG("g_mode cmd_parser.get  %d\n", g_app_run_mode.load());
    LOG_DEBUG("g_width cmd_parser.get  %d\n", g_width);
    LOG_DEBUG("g_height cmd_parser.get  %d\n", g_height);
    LOG_DEBUG("g_device_id cmd_parser.get  %d\n", g_device_id);

    rkaiq_media = std::make_shared<RKAiqMedia>();
    rkaiq_media->GetMediaInfo();
    rkaiq_media->DumpMediaInfo();

    LOG_DEBUG("================== %d =====================\n", g_app_run_mode.load());
    system("mkdir -p /data/OfflineRAW && sync");
    RawCaptureinit();

    // return 0;
    if (g_stream_dev_name.length() > 0) {
        if (0 > access(g_stream_dev_name.c_str(), R_OK | W_OK)) {
            LOG_DEBUG("Could not access streaming device\n");
            if (g_rtsp_en_from_cmdarg == 1) {
                g_rtsp_en = 0;
            }
        } else {
            LOG_DEBUG("Access streaming device\n");
            if (g_rtsp_en_from_cmdarg == 1) {
                g_rtsp_en = 1;
            }
        }
    }

    if (g_rtsp_en && g_stream_dev_name.length() > 0) {
        ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_STREAMING);
        if (ret != 0) {
            LOG_ERROR("Failed set mode to tunning mode, does app started?\n");
        }
    } else {
        ret = RKAiqProtocol::DoChangeAppMode(APP_RUN_STATUS_TUNRING);
        if (ret != 0) {
            LOG_ERROR("Failed set mode to tunning mode, does app started?\n");
        }
    }

    if (g_tcpClient.Setup("/tmp/UNIX.domain") == false) {
        LOG_INFO("#### ToolServer connect AIQ failed ####\n");
        // g_tcpClient.Close();
        // return -1;
    } else {
        LOG_INFO("#### ToolServer connect AIQ success ####\n");
    }

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    tcpServer = std::make_shared<TCPServer>();
    tcpServer->RegisterRecvCallBack(RKAiqProtocol::HandlerTCPMessage);
    tcpServer->Process(SERVER_PORT);
    while (!quit.load() && !tcpServer->Exited()) {
        pause();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    tcpServer->SaveExit();

    if (g_aiqCred != nullptr) {
        delete g_aiqCred;
        g_aiqCred = nullptr;
    }

    if (g_rtsp_en) {
        system("pkill rkaiq_3A_server*");
        deinit_rtsp();
    }

#if 0
  rkaiq_manager.reset();
  rkaiq_manager = nullptr;
#endif
    return 0;
}

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>  /* low-level i/o */
#include <getopt.h> /* getopt_long() */
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include "common/mediactl/mediactl.h"
#include "rk_aiq_user_api_sysctl.h"

#include "../utils/log.h"
#include "config.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define DBG(...)                              \
  do {                                        \
    if (!silent) printf("DBG: " __VA_ARGS__); \
  } while (0)
#define ERR(...)                          \
  do {                                    \
    fprintf(stderr, "ERR: " __VA_ARGS__); \
  } while (0)

/* Private v4l2 event */
#define CIFISP_V4L2_EVENT_STREAM_START (V4L2_EVENT_PRIVATE_START + 1)
#define CIFISP_V4L2_EVENT_STREAM_STOP (V4L2_EVENT_PRIVATE_START + 2)

#define RKAIQ_FILE_PATH_LEN 64
#define RKAIQ_FLASH_NUM_MAX 2

/* 1 vicap + 2 mipi + 1 bridge + 1 redundance */
#define MAX_MEDIA_NODES 5

#if SYS_START
#define IQ_PATH "/etc/iqfiles"
#elif CONFIG_OEM
#define IQ_PATH "/oem/etc/iqfiles"
#else
#define IQ_PATH "/etc/iqfiles"
#endif

#if CONFIG_DBUS
#if CONFIG_CALLFUNC
#include "call_fun_ipc.h"
#include "fun_map.h"
#endif
#include <gdbus.h>
#endif

#if CONFIG_DBSERVER
#include "db_monitor.h"
#include "isp_func.h"
#include "isp_n2d_ctl.h"
#include "manage.h"
#endif

enum { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG };

int enable_minilog = 0;
#define LOG_TAG "ispserver"
int ispserver_log_level = LOG_INFO;

#define CLEAR(x) memset(&(x), 0, sizeof(x))
rk_aiq_sys_ctx_t *db_aiq_ctx = NULL;
rk_aiq_working_mode_t mode[MAX_MEDIA_NODES] = {
    RK_AIQ_WORKING_MODE_ISP_HDR2, RK_AIQ_WORKING_MODE_ISP_HDR2,
    RK_AIQ_WORKING_MODE_ISP_HDR2, RK_AIQ_WORKING_MODE_ISP_HDR2,
    RK_AIQ_WORKING_MODE_ISP_HDR2};

static int silent = 0;
static int width = 2688;
static int height = 1520;
static int has_mul_cam = 0;
static int fixfps = -1;
static bool need_sync_db = true;

#if SYS_START
static bool is_quick_start = true;
#else
static bool is_quick_start = false;
#endif

struct rkaiq_media_info {
  char sd_isp_path[RKAIQ_FILE_PATH_LEN];
  char vd_params_path[RKAIQ_FILE_PATH_LEN];
  char vd_stats_path[RKAIQ_FILE_PATH_LEN];
  char mainpath[RKAIQ_FILE_PATH_LEN];
  char sensor_entity_name[32];

  char mdev_path[32];
  int available;
  rk_aiq_sys_ctx_t *aiq_ctx;

  pthread_t pid;
  int camIndex;
};

static struct rkaiq_media_info media_infos[MAX_MEDIA_NODES];

static void errno_exit(const char *s) {
  ERR("%s error %d, %s\n", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg) {
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

static int save_prepare_status(int cam_id, int status) {
  char pipe_name[128];
  char data[32];
  sprintf(pipe_name, "/tmp/.ispserver_cam%d", cam_id);
  int fd = open(pipe_name, O_RDWR | O_CREAT | O_TRUNC | O_SYNC);
  if (fd > 0) {
    snprintf(data, sizeof(data), "%d", status);
    write(fd, data, strlen(data));
    close(fd);
    sync();
  } else {
    printf("save_prepare_status open err\n");
  }
}

static int rkaiq_get_devname(struct media_device *device, const char *name,
                             char *dev_name) {
  const char *devname;
  struct media_entity *entity = NULL;

  entity = media_get_entity_by_name(device, name, strlen(name));
  if (!entity) return -1;

  devname = media_entity_get_devname(entity);

  if (!devname) {
    fprintf(stderr, "can't find %s device path!", name);
    return -1;
  }

  strncpy(dev_name, devname, RKAIQ_FILE_PATH_LEN);

  DBG("get %s devname: %s\n", name, dev_name);

  return 0;
}

int rkaiq_get_media_info(struct rkaiq_media_info *media_info) {
  struct media_device *device = NULL;
  const char *sensor_name;
  int ret;

  device = media_device_new(media_info->mdev_path);
  if (!device) return -ENOMEM;
  /* Enumerate entities, pads and links. */
  ret = media_device_enumerate(device);
  if (ret) return ret;
  if (!ret) {
    /* Try rkisp */
    ret =
        rkaiq_get_devname(device, "rkisp-isp-subdev", media_info->sd_isp_path);
    ret |= rkaiq_get_devname(device, "rkisp-input-params",
                             media_info->vd_params_path);
    ret |= rkaiq_get_devname(device, "rkisp-statistics",
                             media_info->vd_stats_path);
    ret |= rkaiq_get_devname(device, "rkisp_mainpath", media_info->mainpath);
  }
  if (ret) {
    fprintf(stderr, "Cound not find rkisp dev names, skipped %s\n",
            media_info->mdev_path);
    media_device_unref(device);
    return ret;
  }
  printf("=======mainpath=%s\n", media_info->mainpath);
  sensor_name = rk_aiq_uapi_sysctl_getBindedSnsEntNmByVd(media_info->mainpath);
  if (sensor_name == NULL || strlen(sensor_name) == 0) {
    fprintf(stderr, "ERR: No sensor attached to %s\n", media_info->mdev_path);
    media_device_unref(device);
    return -EINVAL;
  }
  printf("get sensor name=%s\n", sensor_name);
  strcpy(media_info->sensor_entity_name, sensor_name);

  media_device_unref(device);

  return ret;
}

static void db_config_sync(char *hdr_mode) {
#if CONFIG_DBSERVER
  if (need_sync_db && !is_quick_start) {
    /* IMAGE_ADJUSTMENT */
    int brightness = 50;
    int contrast = 50;
    int saturation = 50;
    int sharpness = 50;
    int hue = 50;
    hash_image_adjustment_get(&brightness, &contrast, &saturation, &sharpness,
                              &hue);
    LOG_INFO(
        "brightness:%d, contrast:%d, saturation:%d, sharpness:%d, hue:%d\n\n",
        brightness, contrast, saturation, sharpness, hue);
    brightness_set(brightness);
    contrast_set(contrast);
    saturation_set(saturation);
    sharpness_set(sharpness);
    hue_set(hue);
    /* EXPOSURE */
    exposure_para_set_by_hash();
    /* BLC */
    if (0 != atoi(hdr_mode)) {
      int hdr_level = 0;
      hash_image_blc_get(NULL, &hdr_level, NULL, NULL, NULL);
      blc_hdr_level_enum_set(hdr_level);
    } else {
      blc_normal_mode_para_set_by_hash();
    }
    /* IMAGE_ENHANCEMENT */
    nr_mode_t hash_nr_mode;
    dc_mode_t hash_dc_mode;
    work_mode_2_t dehaze_mode;
    int spatial_level = 0;
    int temporal_level = 0;
    int fec_level = 0;
    int dehaze_level = 0;
    int ldch_level = 0;
    int rotation_angle = 0;
    hash_image_enhancement_get(&hash_nr_mode, &hash_dc_mode, &dehaze_mode,
                               &spatial_level, &temporal_level, &dehaze_level,
                               &ldch_level, &fec_level, &rotation_angle);
    LOG_INFO(
        "nr_mode:%d, distortion_correction_mode:%d, dehaze_mode:%d, "
        "spatial_level:%d, temporal_level:%d, dehaze_level:%d, "
        "ldch_level:%d, fec_level:%d, rotation_angle: %d\n\n",
        hash_nr_mode, hash_dc_mode, dehaze_mode, spatial_level, temporal_level,
        dehaze_level, ldch_level, fec_level, rotation_angle);

    nr_para_set(hash_nr_mode, spatial_level, temporal_level);

    /* IMAGE_ADJUSTMENT */
    expPwrLineFreq_t frequency_mode;
    flip_mode_t mirror_mode;
    hash_image_video_adjustment_get(&frequency_mode, &mirror_mode);
    LOG_INFO("frequency_mode is %d, mirror_mode is %d\n\n", frequency_mode,
             mirror_mode);
    frequency_mode_set(frequency_mode);

    dehaze_para_set(dehaze_mode, dehaze_level);
    dc_para_set(hash_dc_mode, ldch_level, fec_level);

    /* WHITE_BALANCE */
    white_balance_set_by_hash_table();

    mirror_mode_set(mirror_mode);
    /* only for fbc*/
    bypass_stream_rotation_set(rotation_angle);
    /* NIGHT_TO_DAY*/
    night_to_day_para_cap_set_db();
    night2day_loop_run();
  }
#endif
}

static void init_engine(struct rkaiq_media_info *media_info) {
  int index;

#if CONFIG_DBSERVER
  if (need_sync_db && !is_quick_start) {
    rk_aiq_working_mode_t hdr_mode_db = RK_AIQ_WORKING_MODE_NORMAL;
    while (hash_image_hdr_mode_get4init(&hdr_mode_db)) {
      LOG_INFO("Get data is empty, please start dbserver\n");
      sleep(1);
    }
    LOG_INFO("hdr_mode_db: %d \n", hdr_mode_db);
    switch (hdr_mode_db) {
      case RK_AIQ_WORKING_MODE_NORMAL: {
        setenv("HDR_MODE", "0", 1);
        LOG_DEBUG("set hdr normal\n");
        break;
      }
      case RK_AIQ_WORKING_MODE_ISP_HDR2: {
        setenv("HDR_MODE", "1", 1);
        break;
      }
      case RK_AIQ_WORKING_MODE_ISP_HDR3: {
        setenv("HDR_MODE", "2", 1);
        break;
      }
    }
  }
#endif

  char *hdr_mode = getenv("HDR_MODE");
  if (hdr_mode) {
    LOG_INFO("hdr mode: %s\n", hdr_mode);
    if (0 == atoi(hdr_mode))
      mode[media_info->camIndex] = RK_AIQ_WORKING_MODE_NORMAL;
    else if (1 == atoi(hdr_mode))
      mode[media_info->camIndex] = RK_AIQ_WORKING_MODE_ISP_HDR2;
    else if (2 == atoi(hdr_mode))
      mode[media_info->camIndex] = RK_AIQ_WORKING_MODE_ISP_HDR3;
  } else {
    mode[media_info->camIndex] = RK_AIQ_WORKING_MODE_NORMAL;
  }
  /* if (media_info[media_info->camIndex].fix_nohdr_mode)
    mode[media_info->camIndex] = RK_AIQ_WORKING_MODE_NORMAL;
  for (index = 0; index < RKAIQ_CAMS_NUM_MAX; index++)
    if (media_info[cam_id].cams[index].link_enabled) break; */

  media_info->aiq_ctx = rk_aiq_uapi_sysctl_init(media_info->sensor_entity_name,
                                                IQ_PATH, NULL, NULL);
  if (has_mul_cam) rk_aiq_uapi_sysctl_setMulCamConc(media_info->aiq_ctx, 1);

#if CONFIG_DBSERVER
  /* sync fec from db*/
  if (need_sync_db && !is_quick_start) {
    int fec_en;
    hash_image_fec_enable_get4init(&fec_en, NULL);
    int fec_ret = rk_aiq_uapi_setFecEn(media_info->aiq_ctx, fec_en);
    LOG_INFO("set fec_en: %d, ret is %d\n", fec_en, fec_ret);
  }
  /* sync hdr mode for quick_start */
  if (!need_sync_db || is_quick_start) {
    hdr_global_value_set(mode[media_info->camIndex]);
  }
#endif

  if (rk_aiq_uapi_sysctl_prepare(media_info->aiq_ctx, width, height,
                                 RK_AIQ_WORKING_MODE_NORMAL)) {
    ERR("rkaiq engine prepare failed !\n");
    exit(-1);
  }
  db_aiq_ctx = media_info->aiq_ctx;
  save_prepare_status(media_info->camIndex, 1);

#if CONFIG_DBSERVER
  set_stream_on();
#endif

  if (fixfps > 0) {
#if CONFIG_DBSERVER
    isp_fix_fps_set(fixfps);
#else
    frameRateInfo_t fps_info;
    memset(&fps_info, 0, sizeof(fps_info));
    fps_info.fps = fixfps;
    fps_info.mode = OP_MANUAL;
    rk_aiq_uapi_setFrameRate(media_info->aiq_ctx, fps_info);
#endif
  }
  db_config_sync(hdr_mode);
}

static void start_engine(struct rkaiq_media_info *media_info) {
  DBG("device manager start\n");
  rk_aiq_uapi_sysctl_start(media_info->aiq_ctx);
  if (media_info->aiq_ctx == NULL) {
    ERR("rkisp_init engine failed\n");
    exit(-1);
  } else {
#if CONFIG_DBSERVER
#if SYS_START
    send_stream_on_signal();
#endif
#endif
    DBG("rkisp_init engine succeed\n");
  }
}

static void stop_engine(struct rkaiq_media_info *media_info) {
  rk_aiq_uapi_sysctl_stop(media_info->aiq_ctx, false);
}

static void deinit_engine(struct rkaiq_media_info *media_info) {
#if CONFIG_DBSERVER
  if (need_sync_db) {
    set_stream_off();
    night2day_loop_stop();
  }
#endif
  save_prepare_status(media_info->camIndex, 0);
  rk_aiq_uapi_sysctl_deinit(media_info->aiq_ctx);
}

// blocked func
static int wait_stream_event(int fd, unsigned int event_type, int time_out_ms) {
  int ret;
  struct v4l2_event event;

  CLEAR(event);

  do {
    /*
     * xioctl instead of poll.
     * Since poll() cannot wait for input before stream on,
     * it will return an error directly. So, use ioctl to
     * dequeue event and block until sucess.
     */
    ret = xioctl(fd, VIDIOC_DQEVENT, &event);
    if (ret == 0 && event.type == event_type) return 0;
  } while (true);

  return -1;
}

static int subscrible_stream_event(struct rkaiq_media_info *media_info, int fd,
                                   bool subs) {
  struct v4l2_event_subscription sub;
  int ret = 0;

  CLEAR(sub);
  sub.type = CIFISP_V4L2_EVENT_STREAM_START;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    ERR("can't subscribe %s start event!\n", media_info->vd_params_path);
    exit(EXIT_FAILURE);
  }

  CLEAR(sub);
  sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    ERR("can't subscribe %s stop event!\n", media_info->vd_params_path);
  }

  DBG("subscribe events from %s success !\n", media_info->vd_params_path);

  return 0;
}

void *wait_thread_func() {
  LOG_INFO("wait_thread_func...q: %d, db: %d\n", is_quick_start, need_sync_db);
#if CONFIG_DBSERVER
  database_hash_init();
  while (!wait_dbus_init_func()) {
    if (dbus_warn_log_status_get()) {
      dbus_warn_log_close();
    }
    LOG_INFO("wait for dbus init\n");
    usleep(100000);
  }
  if (need_sync_db) {
    dbus_warn_log_open();
  }
  database_init();
  manage_init();
  rk_aiq_working_mode_t hdr_mode_db = RK_AIQ_WORKING_MODE_NORMAL;
  hash_image_hdr_mode_get4init(&hdr_mode_db);
  hdr_mode_set4db(hdr_mode_db);
  switch (hdr_mode_db) {
    case RK_AIQ_WORKING_MODE_NORMAL:
      is_quick_start = false;
      db_config_sync("0");
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR2:
      is_quick_start = false;
      db_config_sync("1");
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR3:
      is_quick_start = false;
      db_config_sync("2");
      break;
    default:
      LOG_ERROR("undefine hdr mode: %d, fail to sync db config\n", hdr_mode_db);
      break;
  }
#endif
}

static void main_exit(void) {
  LOG_INFO("server %s\n", __func__);
  for (int id = 0; id < MAX_MEDIA_NODES; id++) {
    if (media_infos[id].available) {
      LOG_INFO("stop engine camera index %d...\n", id);
      stop_engine(&media_infos[id]);
      LOG_INFO("deinit engine camera index %d...\n", id);
      deinit_engine(&media_infos[id]);
      media_infos[id].available = 0;
    }
  }
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  LOG_INFO("deinit call_fun_ipc_server...\n");
  call_fun_ipc_server_deinit();
#endif
#endif
}

void signal_crash_handler(int sig) {
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  call_fun_ipc_server_deinit();
#endif
#endif
  exit(-1);
}

void signal_exit_handler(int sig) {
#if CONFIG_DBUS
#if CONFIG_CALLFUNC
  call_fun_ipc_server_deinit();
#endif
#endif
  exit(0);
}

void parse_args(int argc, char **argv) {
  int c;
  int digit_optind = 0;

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
        {"silent", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {"nsd", no_argument, NULL, 'n'},
        {"fps", required_argument, NULL, 'f'},
        {0, 0, 0, 0}};

    c = getopt_long(argc, argv, "shnf", long_options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 'w':
        width = atoi(optarg);
        break;
      case 'h':
        height = atoi(optarg);
        break;
      case 's':
        silent = 1;
        break;
      case 'f':
        fixfps = atoi(optarg);
        LOG_INFO("## fixfps: %d\n", fixfps);
        break;
      case 'n':
        need_sync_db = false;
        LOG_INFO("## need_sync_db: %d\n", need_sync_db);
        break;
      case '?':
        ERR("Usage: %s to start 3A engine\n"
            "         --silent,  optional, subpress debug log\n",
            argv[0]);
        exit(-1);

      default:
        ERR("?? getopt returned character code %c ??\n", c);
    }
  }
}

void *engine_thread(void *arg) {
  int ret = 0;
  int isp_fd;
  unsigned int stream_event = -1;
  struct rkaiq_media_info *media_info;

  media_info = (struct rkaiq_media_info *)arg;

  isp_fd = open(media_info->vd_params_path, O_RDWR);
  if (isp_fd < 0) {
    ERR("open %s failed %s\n", media_info->vd_params_path, strerror(errno));
    return NULL;
  }

  subscrible_stream_event(media_info, isp_fd, true);
  init_engine(media_info);

  for (;;) {
    DBG("%s: wait stream start event...\n", media_info->mdev_path);
    wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
    DBG("%s: wait stream start event success ...\n", media_info->mdev_path);

    start_engine(media_info);

    DBG("%s: wait stream stop event...\n", media_info->mdev_path);
    wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
    DBG("%s: wait stream stop event success ...\n", media_info->mdev_path);

    stop_engine(media_info);

#if CONFIG_DBSERVER
    if (!check_stream_status()) break;
#endif
  }

  deinit_engine(media_info);
  subscrible_stream_event(media_info, isp_fd, false);
  close(isp_fd);

  return NULL;
}

int main(int argc, char **argv) {
#ifdef ENABLE_MINILOGGER
  enable_minilog = 1;
  __minilog_log_init(argv[0], NULL, false, false, "ispserver", "1.0.0");
#endif

  int ret, i;
  int threads = 0;

  /* Line buffered so that printf can flash every line if redirected to
   * no-interactive device.
   */
  setlinebuf(stdout);

  parse_args(argc, argv);

  for (i = 0; i < MAX_MEDIA_NODES; i++) {
    sprintf(media_infos[i].mdev_path, "/dev/media%d", i);
    if (rkaiq_get_media_info(&media_infos[i])) {
      ERR("Bad media topology for: %s\n", media_infos[i].mdev_path);
      media_infos[i].available = 0;
      continue;
    }
    media_infos[i].available = 1;
    media_infos[i].camIndex = i;
    threads++;
  }

  if (threads > 1) has_mul_cam = 1;

#if CONFIG_DBSERVER
  if (!need_sync_db) dbus_warn_log_close();

  if (wait_dbus_init_func()) {
    LOG_INFO("dbus init complete, alter quick start false\n");
    is_quick_start = false;
  }

  if (!is_quick_start && (need_sync_db || wait_dbus_init_func())) {
    LOG_INFO("init for db\n");
    database_hash_init();
    database_init();
    manage_init();
  }

  LOG_INFO("before enter wait thread q: %d, db: %d\n", is_quick_start,
            need_sync_db);
  if (is_quick_start || !need_sync_db) {
    pthread_t wait_db_p;
    if (pthread_create(&wait_db_p, NULL, wait_thread_func, NULL) != 0) {
      LOG_INFO("enter wait thread for db error\n");
    }
  }
#endif

  for (i = 0; i < MAX_MEDIA_NODES; i++) {
    if (!media_infos[i].available) continue;
    ret = pthread_create(&media_infos[i].pid, NULL, engine_thread,
                         &media_infos[i]);
    if (ret) {
      media_infos[i].pid = 0;
      ERR("Failed to create camera engine thread for: %s\n",
          media_infos[i].mdev_path);
      errno_exit("Create thread failed");
    }
  }

#if CONFIG_DBUS
  pthread_detach(pthread_self());
  GMainLoop *main_loop;
  atexit(main_exit);
  signal(SIGTERM, signal_exit_handler);
  signal(SIGINT, signal_exit_handler);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGBUS, signal_crash_handler);
  signal(SIGSEGV, signal_crash_handler);
  signal(SIGFPE, signal_crash_handler);
  signal(SIGABRT, signal_crash_handler);

#if CONFIG_CALLFUNC
  call_fun_ipc_server_init(map, sizeof(map) / sizeof(struct FunMap), DBUS_NAME,
                           DBUS_IF, DBUS_PATH, 0);
  LOG_INFO("call_fun_ipc_demo_server init\n");
#endif

  main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(main_loop);
  if (main_loop) g_main_loop_unref(main_loop);
#endif

  for (i = 0; i < MAX_MEDIA_NODES; i++) {
    if (!media_infos[i].available || media_infos[i].pid == 0) continue;
    pthread_join(media_infos[i].pid, NULL);
  }

  return 0;
}

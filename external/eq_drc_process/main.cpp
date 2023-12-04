#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>

#include "eq_log.h"
#include "Rk_wake_lock.h"
#include "Rk_socket_app.h"

#define EQ_DRC_PROCESS_VERSION      "1.29 20210720"

#define SOC_IS_RK3308           (0x1)
#define SOC_IS_RK3326           (0x2)

#define ROCKCHIP_SOC            SOC_IS_RK3326

#define SAMPLE_RATE 48000
#define CHANNEL 2
#define REC_DEVICE_NAME "fake_record"
#define WRITE_DEVICE_NAME "fake_play"
#define JACK_DEVICE_NAME "fake_jack"
#define JACK2_DEVICE_NAME "fake_jack2"
#define READ_FRAME_DEFAULT      1920
#define PERIOD_SIZE_DEFAULT     (READ_FRAME_DEFAULT)
#define PERIOD_COUNTS_DEFAULT   (8)
#define BUFFER_SIZE_DEFAULT     (PERIOD_SIZE_DEFAULT * PERIOD_COUNTS_DEFAULT) /* Keeping a large buffer_size ASAP */
#define MUTE_TIME_DEFAULT       (3) /* seconds */
//#define ALSA_READ_FORMAT SND_PCM_FORMAT_S32_LE
#define ALSA_READ_FORMAT SND_PCM_FORMAT_S16_LE
#define ALSA_WRITE_FORMAT SND_PCM_FORMAT_S16_LE

/*
 * Select different alsa pathways based on device type.
 *  LINE_OUT: LR-Mix(fake_play)->EqDrcProcess(ladspa)->Speaker(real_playback)
 *  HEAD_SET: fake_jack -> Headset(real_playback)
 *  BLUETOOTH: device as bluetooth source.
 */
#define DEVICE_FLAG_LINE_OUT        0x01
#define DEVICE_FLAG_ANALOG_HP       0x02
#define DEVICE_FLAG_DIGITAL_HP      0x03
#define DEVICE_FLAG_BLUETOOTH       0x04
#define DEVICE_FLAG_BLUETOOTH_BSA   0x05

enum BT_CONNECT_STATE{
    BT_DISCONNECT = 0,
    BT_CONNECT_BLUEZ,
    BT_CONNECT_BSA
};

#define POWER_STATE_PATH        "/sys/power/state"
#define USER_PLAY_STATUS        "/dev/snd/pcmC7D0p"
#define USER_CAPT_STATUS        "/dev/snd/pcmC0D0c"

/**
 * 0: By default and universal (Recommend)
 * 1: More fast but only used for RK817 or RK809 Codec
 */
#define KEEPING_HW_CARD         0
#if KEEPING_HW_CARD
#define HW_CARD_PATH_DEFAULT    "SPK"
#endif

struct user_play_inotify {
    int fd;
    int watch_desc;
    bool stop;
};

struct user_capt_inotify {
    int fd;
    int watch_desc;
    bool stop;
};

enum {
    USER_PLAY_CLOSED = 0,
    USER_PLAY_CLOSING,
    USER_PLAY_OPENED,
};

enum {
    USER_CAPT_CLOSED = 0,
    USER_CAPT_CLOSING,
    USER_CAPT_OPENED,
};

enum {
    POWER_STATE_RESUME = 0,
    POWER_STATE_SUSPENDING,
    POWER_STATE_SUSPEND,
};

static int g_read_frame = READ_FRAME_DEFAULT;
static int g_period_size = PERIOD_SIZE_DEFAULT;
static int g_period_counts = PERIOD_COUNTS_DEFAULT;
static int g_buffer_size = BUFFER_SIZE_DEFAULT;

#if KEEPING_HW_CARD
static char g_path_name[32];
volatile static bool g_fast_codec = false;
#endif

static struct user_play_inotify g_upi;
static struct user_capt_inotify g_uci;
static char g_bt_mac_addr[17];
static enum BT_CONNECT_STATE g_bt_is_connect = BT_DISCONNECT;
static bool g_system_sleep = false;
static char sock_path[] = "/data/bsa/config/bsa_socket";

#if KEEPING_HW_CARD
static snd_pcm_t *write_handle_bak = NULL;
#endif

volatile static int power_state = POWER_STATE_RESUME;
volatile static int user_play_state = USER_PLAY_CLOSED;
volatile static int user_capt_state = USER_CAPT_CLOSED;

struct timeval tv_begin, tv_end;
//gettimeofday(&tv_begin, NULL);

extern int set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t buffer_size,
                         snd_pcm_uframes_t period_size, char **msg);

/* epoll for inotify */
#define EPOLL_SIZE                      512
#define ARRAY_LENGTH                    128
#define NAME_LENGTH                     128

#define EPOLL_MAX_EVENTS                32

struct file_name_fd_desc {
    int fd;
    char name[32];
    char base_name[NAME_LENGTH];
};

static struct epoll_event g_PendingEventItems[EPOLL_MAX_EVENTS];

static struct file_name_fd_desc g_file_name_fd_desc[ARRAY_LENGTH];
static int array_index = 0;

static const char *base_dir = "/sys/power";

/* hanlding fade-in or fade-out */
#define FADE_IN             0
#define FADE_OUT            1

#ifndef av_clipd
#   define av_clipd         av_clipd_c
#endif

enum CurveType { NONE = -1, TRI, QSIN, ESIN, HSIN, LOG, IPAR, QUA, CUB, SQU, CBR, PAR, EXP, IQSIN, IHSIN, DESE, DESI, LOSI, SINC, ISINC, NB_CURVES };

/**
 * Clip a double value into the amin-amax range.
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static inline const double av_clipd_c(double a, double amin, double amax)
{
    // eq_info("[EQ] %s - %d enter\n", __func__, __LINE__);
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

static double fade_gain(int curve, int64_t index, int64_t range)
{
#define CUBE(a) ((a)*(a)*(a))
    double gain;

    gain = av_clipd(1.0 * index / range, 0, 1.0);

    // eq_info("[EQ] %s - %d index=%ld gain=%lf\n", __func__, __LINE__, index, gain);

    switch (curve) {
    case QSIN:
        gain = sin(gain * M_PI / 2.0);
        break;
    case IQSIN:
        /* 0.6... = 2 / M_PI */
        gain = 0.6366197723675814 * asin(gain);
        break;
    case ESIN:
        gain = 1.0 - cos(M_PI / 4.0 * (CUBE(2.0*gain - 1) + 1));
        break;
    case HSIN:
        gain = (1.0 - cos(gain * M_PI)) / 2.0;
        break;
    case IHSIN:
        /* 0.3... = 1 / M_PI */
        gain = 0.3183098861837907 * acos(1 - 2 * gain);
        break;
    case EXP:
        /* -11.5... = 5*ln(0.1) */
        gain = exp(-11.512925464970227 * (1 - gain));
        break;
    case LOG:
        gain = av_clipd(1 + 0.2 * log10(gain), 0, 1.0);
        break;
    case PAR:
        gain = 1 - sqrt(1 - gain);
        break;
    case IPAR:
        gain = (1 - (1 - gain) * (1 - gain));
        break;
    case QUA:
        gain *= gain;
        break;
    case CUB:
        gain = CUBE(gain);
        break;
    case SQU:
        gain = sqrt(gain);
        break;
    case CBR:
        gain = cbrt(gain);
        break;
    case DESE:
        gain = gain <= 0.5 ? cbrt(2 * gain) / 2: 1 - cbrt(2 * (1 - gain)) / 2;
        break;
    case DESI:
        gain = gain <= 0.5 ? CUBE(2 * gain) / 2: 1 - CUBE(2 * (1 - gain)) / 2;
        break;
    case LOSI: {
                   const double a = 1. / (1. - 0.787) - 1;
                   double A = 1. / (1.0 + exp(0 -((gain-0.5) * a * 2.0)));
                   double B = 1. / (1.0 + exp(a));
                   double C = 1. / (1.0 + exp(0-a));
                   gain = (A - B) / (C - B);
               }
        break;
    case SINC:
        gain = gain >= 1.0 ? 1.0 : sin(M_PI * (1.0 - gain)) / (M_PI * (1.0 - gain));
        break;
    case ISINC:
        gain = gain <= 0.0 ? 0.0 : 1.0 - sin(M_PI * gain) / (M_PI * gain);
        break;
    case NONE:
        gain = 1.0;
        break;
    }

    return gain;
}

#define FADE(name, type)                                                    \
static void fade_samples_## name (uint8_t **dst, uint8_t * const *src,      \
                                  int nb_samples, int channels, int dir,    \
                                  int64_t start, int64_t range, int curve)  \
{                                                                           \
    type *d = (type *)dst[0];                                               \
    const type *s = (type *)src[0];                                         \
    int i, c, k = 0;                                                        \
                                                                            \
    for (i = 0; i < nb_samples; i++) {                                      \
        double gain = fade_gain(curve, start + i * dir, range);             \
        for (c = 0; c < channels; c++, k++)                                 \
            d[k] = s[k] * gain;                                             \
    }                                                                       \
}

FADE(dbl, double)
FADE(flt, float)
FADE(s16, int16_t)
FADE(s32, int32_t)

static int add_to_epoll(int epoll_fd, int fd)
{
    int result;
    struct epoll_event eventItem;

    memset(&eventItem, 0, sizeof(eventItem));
    eventItem.events    = EPOLLIN;
    eventItem.data.fd   = fd;
    result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &eventItem);

    return result;
}

static void remove_from_epoll(int epoll_fd, int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

static int get_name_from_fd(int fd, char **name)
{
    int i;

    for(i = 0; i < ARRAY_LENGTH; i++)
    {
        if(fd == g_file_name_fd_desc[i].fd)
        {
            *name = g_file_name_fd_desc[i].name;
            return 0;
        }
    }

    return -1;
}

static int inotify_ctl_info(int inotify_fd, int epoll_fd)
{
    char event_buf[EPOLL_SIZE];
    int event_pos = 0;
    int event_size;
    struct inotify_event *event;
    int result;
    int tmp_fd;
    int i;

    memset(event_buf, 0, EPOLL_SIZE);
    result = read(inotify_fd, event_buf, sizeof(event_buf));
    if(result < (int)sizeof(*event)) {
        printf("could not get event!\n");
        return -1;
    }

    while (result >= (int)sizeof(*event))
    {
        event = (struct inotify_event *)(event_buf + event_pos);
        if (event->len)
        {
            if (event->mask & IN_CREATE)
            {
                sprintf(g_file_name_fd_desc[array_index].name, "%s", event->name);
                sprintf(g_file_name_fd_desc[array_index].base_name, "%s/%s", base_dir, event->name);

                tmp_fd = open(g_file_name_fd_desc[array_index].base_name, O_RDWR);
                if(-1 == tmp_fd)
                {
                    printf("inotify_ctl_info open error!\n");
                    return -1;
                }
                add_to_epoll(epoll_fd, tmp_fd);

                g_file_name_fd_desc[array_index].fd = tmp_fd;
                if(ARRAY_LENGTH == array_index)
                {
                    array_index = 0;
                }
                array_index += 1;

                printf("add file to epoll: %s\n", event->name);
            }
            else if (event->mask & IN_DELETE)
            {
                for(i = 0; i < ARRAY_LENGTH; i++)
                {
                    if(!strcmp(g_file_name_fd_desc[i].name, event->name))
                    {
                        remove_from_epoll(epoll_fd, g_file_name_fd_desc[i].fd);

                        g_file_name_fd_desc[i].fd = 0;
                        memset(g_file_name_fd_desc[i].name, 0, sizeof(g_file_name_fd_desc[i].name));
                        memset(g_file_name_fd_desc[i].base_name, 0, sizeof(g_file_name_fd_desc[i].base_name));

                        printf("remove file from epoll: %s\n", event->name);
                        break;
                    }
                }
            }
            else if (event->mask & IN_MODIFY)
            {
                printf("modify file to epoll: %s and will suspend, power_state: %d\n",
                    event->name, power_state);

                if (power_state == POWER_STATE_RESUME)
                    power_state = POWER_STATE_SUSPENDING;
                else
                    power_state = POWER_STATE_RESUME;
            }
        }

        event_size = sizeof(*event) + event->len;
        result -= event_size;
        event_pos += event_size;
    }

    return 0;
}

static void *power_status_listen(void *arg)
{
    int inotify_fd;
    int epoll_fd;
    int result;
    int i;

    char readbuf[EPOLL_SIZE];
    int readlen;

    char *tmp_name;

    return NULL;

    eq_info("[EQ] %s enter\n", __func__);

    epoll_fd = epoll_create(1);
    if(-1 == epoll_fd)
    {
        printf("epoll_create error!\n");
        goto err;
    }

    inotify_fd = inotify_init();

    result = inotify_add_watch(inotify_fd, base_dir, IN_MODIFY);
    if(-1 == result)
    {
        printf("inotify_add_watch error!\n");
        goto err;
    }

    add_to_epoll(epoll_fd, inotify_fd);

    eq_info("[EQ] %s, %d add_to_epoll\n", __func__, __LINE__);

    while (1)
    {
        result = epoll_wait(epoll_fd, g_PendingEventItems, EPOLL_MAX_EVENTS, -1);
        if (-1 == result)
        {
            printf("epoll wait error!\n");
            goto err;
        }
        else
        {
            for (i = 0; i < result; i++)
            {
                if (g_PendingEventItems[i].data.fd == inotify_fd)
                {
                    if (-1 == inotify_ctl_info(inotify_fd, epoll_fd))
                    {
                        printf("inotify_ctl_info error!\n");
                        goto err;
                    }
                }
                else
                {
                    if (!get_name_from_fd(g_PendingEventItems[i].data.fd, &tmp_name))
                    {
                        readlen = read(g_PendingEventItems[i].data.fd, readbuf, EPOLL_SIZE);
                        readbuf[readlen] = '\0';
                        printf("read data from %s : %s\n", tmp_name, readbuf);
                    }
                }
            }
        }
    }

err:
    eq_info("[EQ] %s exit\n", __func__);
    return NULL;
}

void alsa_fake_device_record_open(snd_pcm_t** capture_handle,int channels,uint32_t rate)
{
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t periodSize = g_period_size;
    snd_pcm_uframes_t bufferSize = g_buffer_size;
    int dir = 0;
    int err;

    err = snd_pcm_open(capture_handle, REC_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Unable to open capture PCM device\n");
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] snd_pcm_open\n");
    //err = snd_pcm_hw_params_alloca(&hw_params);

    err = snd_pcm_hw_params_malloc(&hw_params);
    if(err)
    {
        eq_err("[EQ_RECORD_OPEN] cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] snd_pcm_hw_params_malloc\n");

    err = snd_pcm_hw_params_any(*capture_handle, hw_params);
    if(err)
    {
        eq_err("[EQ_RECORD_OPEN] cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] snd_pcm_hw_params_any!\n");

    err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    // err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Error setting interleaved mode\n");
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] snd_pcm_hw_params_set_access!\n");

    err = snd_pcm_hw_params_set_format(*capture_handle, hw_params, ALSA_READ_FORMAT);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Error setting format: %s\n", snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] snd_pcm_hw_params_set_format\n");

    err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, channels);
    if (err)
    {
        eq_debug("[EQ_RECORD_OPEN] channels = %d\n",channels);
        eq_err("[EQ_RECORD_OPEN] Error setting channels: %s\n", snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] channels = %d\n",channels);

    err = snd_pcm_hw_params_set_buffer_size_near(*capture_handle, hw_params, &bufferSize);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Error setting buffer size (%d): %s\n", bufferSize, snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] bufferSize = %d\n",bufferSize);

    err = snd_pcm_hw_params_set_period_size_near(*capture_handle, hw_params, &periodSize, 0);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Error setting period time (%d): %s\n", periodSize, snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] periodSize = %d\n",periodSize);

    err = snd_pcm_hw_params_set_rate_near(*capture_handle, hw_params, &rate, 0/*&dir*/);
    if (err)
    {
        eq_err("[EQ_RECORD_OPEN] Error setting sampling rate (%d): %s\n", rate, snd_strerror(err));
        exit(1);
    }
    eq_debug("[EQ_RECORD_OPEN] Rate = %d\n", rate);

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(*capture_handle, hw_params);
    if (err < 0)
    {
        eq_err("[EQ_RECORD_OPEN] Unable to set HW parameters: %s\n", snd_strerror(err));
        exit(1);
    }

    eq_debug("[EQ_RECORD_OPEN] Open record device done \n");
    //if(set_sw_params(*capture_handle,bufferSize,periodSize,NULL) < 0)
    //    exit(1);

    if(hw_params)
        snd_pcm_hw_params_free(hw_params);
}

int alsa_fake_device_write_open(snd_pcm_t** write_handle, int channels,
                                 uint32_t write_sampleRate, int device_flag,
                                 int *socket_fd)
{
    snd_pcm_hw_params_t *write_params;
    snd_pcm_uframes_t write_periodSize = g_period_size;
    snd_pcm_uframes_t write_bufferSize = g_buffer_size;
    int write_err;
    int write_dir;
    char bluealsa_device[256] = {0};

    if (device_flag == DEVICE_FLAG_ANALOG_HP) {
        eq_debug("[EQ_WRITE_OPEN] Open PCM: %s\n", JACK_DEVICE_NAME);
        write_err = snd_pcm_open(write_handle, JACK_DEVICE_NAME,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    } else if (device_flag == DEVICE_FLAG_DIGITAL_HP) {
        eq_debug("[EQ_WRITE_OPEN] Open PCM: %s\n", JACK2_DEVICE_NAME);
        write_err = snd_pcm_open(write_handle, JACK2_DEVICE_NAME,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    } else if (device_flag == DEVICE_FLAG_BLUETOOTH) {
        sprintf(bluealsa_device, "%s%s", "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=",
                g_bt_mac_addr);
        eq_debug("[EQ_WRITE_OPEN] Open PCM: %s\n", bluealsa_device);
        write_err = snd_pcm_open(write_handle, bluealsa_device,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    } else if (device_flag == DEVICE_FLAG_BLUETOOTH_BSA) {
        *socket_fd = RK_socket_client_setup(sock_path);
        if (*socket_fd < 0) {
            eq_err("[EQ_WRITE_OPEN] Fail to connect server socket\n");
            return -1;
        } else {
            eq_debug("[EQ_WRITE_OPEN] Socket client connected\n");
            return 0;
        }
    } else {
        eq_debug("[EQ_WRITE_OPEN] Open PCM: %s\n", WRITE_DEVICE_NAME);
        write_err = snd_pcm_open(write_handle, WRITE_DEVICE_NAME,
                                 SND_PCM_STREAM_PLAYBACK, 0);
    }

    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Unable to open playback PCM device: %d\n", write_err);
        return write_err;
    }
    eq_debug("[EQ_WRITE_OPEN] interleaved mode\n");

    // snd_pcm_hw_params_alloca(&write_params);
    snd_pcm_hw_params_malloc(&write_params);
    eq_debug("[EQ_WRITE_OPEN] snd_pcm_hw_params_alloca\n");

    snd_pcm_hw_params_any(*write_handle, write_params);

    write_err = snd_pcm_hw_params_set_access(*write_handle, write_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    //write_err = snd_pcm_hw_params_set_access(*write_handle,  write_params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Error setting interleaved mode\n");
        goto failed;
    }
    eq_debug( "[EQ_WRITE_OPEN] interleaved mode\n");

    write_err = snd_pcm_hw_params_set_format(*write_handle, write_params, ALSA_WRITE_FORMAT);
    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Error setting format: %s\n", snd_strerror(write_err));
        goto failed;
    }
    eq_debug("[EQ_WRITE_OPEN] format successed\n");

    write_err = snd_pcm_hw_params_set_channels(*write_handle, write_params, channels);
    if (write_err) {
        eq_err( "[EQ_WRITE_OPEN] Error setting channels: %s\n", snd_strerror(write_err));
        goto failed;
    }
    eq_debug("[EQ_WRITE_OPEN] channels = %d\n", channels);

    write_err = snd_pcm_hw_params_set_rate_near(*write_handle, write_params, &write_sampleRate, 0/*&write_dir*/);
    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Error setting sampling rate (%d): %s\n", write_sampleRate, snd_strerror(write_err));
        goto failed;
    }
    eq_debug("[EQ_WRITE_OPEN] setting sampling rate (%d)\n", write_sampleRate);

    write_err = snd_pcm_hw_params_set_buffer_size_near(*write_handle, write_params, &write_bufferSize);
    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Error setting buffer size (%ld): %s\n", write_bufferSize, snd_strerror(write_err));
        goto failed;
    }
    eq_debug("[EQ_WRITE_OPEN] write_bufferSize = %d\n", write_bufferSize);

    write_err = snd_pcm_hw_params_set_period_size_near(*write_handle, write_params, &write_periodSize, 0);
    if (write_err) {
        eq_err("[EQ_WRITE_OPEN] Error setting period time (%ld): %s\n", write_periodSize, snd_strerror(write_err));
        goto failed;
    }
    eq_debug("[EQ_WRITE_OPEN] write_periodSize = %d\n", write_periodSize);

#if 0
    snd_pcm_uframes_t write_final_buffer;
    write_err = snd_pcm_hw_params_get_buffer_size(write_params, &write_final_buffer);
    eq_debug(" final buffer size %ld \n" , write_final_buffer);

    snd_pcm_uframes_t write_final_period;
    write_err = snd_pcm_hw_params_get_period_size(write_params, &write_final_period, &write_dir);
    eq_debug(" final period size %ld \n" , write_final_period);
#endif

    {
        int monotonic, can_pause;

        monotonic = snd_pcm_hw_params_is_monotonic(write_params);
        can_pause = snd_pcm_hw_params_can_pause(write_params);

        eq_info("[EQ_INFO] monotonic: %d, can_pause: %d\n", monotonic, can_pause);
    }

    /* Write the parameters to the driver */
    write_err = snd_pcm_hw_params(*write_handle, write_params);
    if (write_err < 0) {
        eq_err("[EQ_WRITE_OPEN] Unable to set HW parameters: %s\n", snd_strerror(write_err));
        goto failed;
    }

    eq_debug("[EQ_WRITE_OPEN] open write device is successful\n");
    if(set_sw_params(*write_handle, write_bufferSize, write_periodSize, NULL) < 0)
        goto failed;

    if(write_params)
        snd_pcm_hw_params_free(write_params);

#if KEEPING_HW_CARD
    if (device_flag == DEVICE_FLAG_LINE_OUT)
        g_fast_codec = true;
#endif
    return 0;

failed:
    if(write_params)
        snd_pcm_hw_params_free(write_params);

    snd_pcm_close(*write_handle);
    *write_handle = NULL;

    return -1;
}

int set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t buffer_size,
                  snd_pcm_uframes_t period_size, char **msg) {

    snd_pcm_sw_params_t *params;
    snd_pcm_uframes_t threshold;
    char buf[256];
    int err;

    //snd_pcm_sw_params_alloca(&params);
    snd_pcm_sw_params_malloc(&params);
    if ((err = snd_pcm_sw_params_current(pcm, params)) != 0) {
        eq_err("[EQ_SET_SW_PARAMS] Get current params: %s\n", snd_strerror(err));
        goto failed;
    }

    /* start the transfer when the buffer is full (or almost full) */
    threshold = (buffer_size / period_size) * period_size;
    if ((err = snd_pcm_sw_params_set_start_threshold(pcm, params, threshold)) != 0) {
        eq_err("[EQ_SET_SW_PARAMS] Set start threshold: %s: %lu\n", snd_strerror(err), threshold);
        goto failed;
    }

    /* allow the transfer when at least period_size samples can be processed */
    if ((err = snd_pcm_sw_params_set_avail_min(pcm, params, period_size)) != 0) {
        eq_err("[EQ_SET_SW_PARAMS] Set avail min: %s: %lu\n", snd_strerror(err), period_size);
        goto failed;
    }

    if ((err = snd_pcm_sw_params(pcm, params)) != 0) {
        eq_err("[EQ_SET_SW_PARAMS] %s\n", snd_strerror(err));
        goto failed;
    }

    if(params)
        snd_pcm_sw_params_free(params);

    return 0;

failed:
    if(params)
        snd_pcm_sw_params_free(params);

    return -1;
}

static int is_mute_frame(short *in,unsigned int size)
{
    int i;
    int mute_count = 0;

    if (!size) {
        eq_err("frame size is zero!!!\n");
        return 0;
    }
    for (i = 0; i < size;i ++) {
        if(in[i] != 0)
        return 0;
    }

    return 1;
}

/* Determine whether to enter the energy saving mode according to
 * the value of the environment variable "EQ_LOW_POWERMODE"
 */
bool low_power_mode_check()
{
    char *value = NULL;

    /* env: "EQ_LOW_POWERMODE=TRUE" or "EQ_LOW_POWERMODE=true" ? */
    value = getenv("EQ_LOW_POWERMODE");
    if (value && (!strcmp("TRUE", value) || !strcmp("true", value)))
        return true;

    return false;
}

/* Check device changing. */
int get_device_flag()
{
    int fd = 0, ret = 0;
    char buff[512] = {0};
    int device_flag = DEVICE_FLAG_LINE_OUT;
#if (ROCKCHIP_SOC == SOC_IS_RK3308)
    const char *path = "/sys/devices/platform/ff560000.acodec/rk3308-acodec-dev/dac_output";
#else /* else is RK3326 */
    const char *path = "/sys/class/switch/h2w/state";
#endif
    FILE *pp = NULL; /* pipeline */
    char *bt_mac_addr = NULL;

    if (g_bt_is_connect == BT_CONNECT_BLUEZ)
        return DEVICE_FLAG_BLUETOOTH;
    else if(g_bt_is_connect == BT_CONNECT_BSA)
        return DEVICE_FLAG_BLUETOOTH_BSA;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        eq_err("[EQ_DEVICE_FLAG] Open %s failed!\n", path);
        return device_flag;
    }

    ret = read(fd, buff, sizeof(buff));
    if (ret <= 0) {
        eq_err("[EQ_DEVICE_FLAG] Read %s failed!\n", path);
        close(fd);
        return device_flag;
    }

#if (ROCKCHIP_SOC == SOC_IS_RK3308)
    if (strstr(buff, "hp out"))
        device_flag = DEVICE_FLAG_ANALOG_HP;
#else /* else is RK3326 */
    if (strstr(buff, "1"))
        device_flag = DEVICE_FLAG_ANALOG_HP;
    else if (strstr(buff, "2"))
        device_flag = DEVICE_FLAG_DIGITAL_HP;
#endif

    close(fd);

    return device_flag;
}

/* Get device name frome device_flag */
const char *get_device_name(int device_flag)
{
    const char *device_name = NULL;

    switch (device_flag) {
        case DEVICE_FLAG_BLUETOOTH:
        case DEVICE_FLAG_BLUETOOTH_BSA:
            device_name = "BLUETOOTH";
            break;
        case DEVICE_FLAG_ANALOG_HP:
            device_name = JACK_DEVICE_NAME;
            break;
        case DEVICE_FLAG_DIGITAL_HP:
            device_name = JACK2_DEVICE_NAME;
            break;
        case DEVICE_FLAG_LINE_OUT:
            device_name = WRITE_DEVICE_NAME;
            break;
        default:
            break;
    }

    return device_name;
}

static void user_play_inotify_handler(struct inotify_event *event)
{
    // eq_info("[EQ] %s enter\n", __func__);
    // eq_info("[EQ] event->mask: 0x%08x\n", event->mask);
    // eq_info("[EQ] event->name: %s\n", event->name);

    switch (event->mask)
    {
        case IN_OPEN:
            user_play_state = USER_PLAY_OPENED;
            eq_info("[EQ] %s USER_PLAY_OPENED\n", __func__);
            break;
        case IN_CLOSE_WRITE:
            user_play_state = USER_PLAY_CLOSING;
            eq_info("[EQ] %s USER_PLAY_CLOSING\n", __func__);
            break;
        default:
            break;
    }
}

static void user_capt_inotify_handler(struct inotify_event *event)
{
    // eq_info("[EQ] %s enter\n", __func__);
    // eq_info("[EQ] event->mask: 0x%08x\n", event->mask);
    // eq_info("[EQ] event->name: %s\n", event->name);

    switch (event->mask)
    {
        case IN_OPEN:
            user_capt_state = USER_CAPT_OPENED;
            eq_info("[EQ] %s USER_CAPT_OPENED\n", __func__);
            break;
        case IN_CLOSE_WRITE:
            user_capt_state = USER_CAPT_CLOSING;
            eq_info("[EQ] %s USER_CAPT_CLOSING\n", __func__);
            break;
        default:
            break;
    }
}

static void *user_play_status_listen(void *arg)
{
    struct user_play_inotify *upi = &g_upi;
    struct inotify_event *event = NULL;
    FILE *fp;
    char *buf;

    eq_info("[EQ] %s enter\n", __func__);

    buf = (char *)calloc(1024, 1);
    if (!buf) {
        eq_err("[EQ] %s alloc buf failed!\n", __func__);
        return NULL;
    }

    upi->stop = 0;
    upi->fd = inotify_init();

    upi->watch_desc = inotify_add_watch(upi->fd, USER_PLAY_STATUS, IN_OPEN | IN_CLOSE_WRITE);
    while (!upi->stop)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(upi->fd, &fds);

        if (select(upi->fd + 1, &fds, NULL, NULL, NULL) > 0)
        {
            int len, index = 0;
            while (((len = read(upi->fd, buf, 1024)) < 0) && (errno == EINTR));
            while (index < len)
            {
                event = (struct inotify_event *)(buf + index);
                user_play_inotify_handler(event);
                index += sizeof(struct inotify_event) + event->len;
            }
        }
    }

    if (upi->fd >= 0) {
        inotify_rm_watch(upi->fd, upi->watch_desc);
        close(upi->fd);
        upi->fd = -1;
    }

    if (buf)
        free(buf);

    eq_info("[EQ] %s exit\n", __func__);

    return NULL;

err_out:
    if (buf)
        free(buf);

    return NULL;
}

static void *user_capt_status_listen(void *arg)
{
    struct user_capt_inotify *uci = &g_uci;
    struct inotify_event *event = NULL;
    FILE *fp;
    char *buf;

    eq_info("[EQ] %s enter\n", __func__);

    buf = (char *)calloc(1024, 1);
    if (!buf) {
        eq_err("[EQ] %s alloc buf failed!\n", __func__);
        return NULL;
    }

    uci->stop = 0;
    uci->fd = inotify_init();

    uci->watch_desc = inotify_add_watch(uci->fd, USER_CAPT_STATUS, IN_OPEN | IN_CLOSE_WRITE);
    while (!uci->stop)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(uci->fd, &fds);

        if (select(uci->fd + 1, &fds, NULL, NULL, NULL) > 0)
        {
            int len, index = 0;
            while (((len = read(uci->fd, buf, 1024)) < 0) && (errno == EINTR));
            while (index < len)
            {
                event = (struct inotify_event *)(buf + index);
                user_capt_inotify_handler(event);
                index += sizeof(struct inotify_event) + event->len;
            }
        }
    }

    if (uci->fd >= 0) {
        inotify_rm_watch(uci->fd, uci->watch_desc);
        close(uci->fd);
        uci->fd = -1;
    }

    if (buf)
        free(buf);

    eq_info("[EQ] %s exit\n", __func__);

    return NULL;

err_out:
    if (buf)
        free(buf);

    return NULL;
}

void *a2dp_status_listen(void *arg)
{
    int ret = 0;
    char buff[100] = {0};
    struct sockaddr_un clientAddr;
    struct sockaddr_un serverAddr;
    int sockfd;
    socklen_t addr_len;
    char *start = NULL;
    snd_pcm_t* audio_bt_handle;
    char bluealsa_device[256] = {0};
    int retry_cnt = 5;

    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        eq_err("[EQ_A2DP_LISTEN] Create socket failed!\n");
        return NULL;
    }

    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "/tmp/a2dp_master_status");

    system("rm -rf /tmp/a2dp_master_status");
    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        eq_err("[EQ_A2DP_LISTEN] Bind Local addr failed!\n");
        return NULL;
    }

    while(1) {
        addr_len = sizeof(clientAddr);
        memset(buff, 0, sizeof(buff));
        ret = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&clientAddr, &addr_len);
        if (ret <= 0) {
            eq_err("[EQ_A2DP_LISTEN]: %s\n", strerror(errno));
            break;
        }
        eq_debug("[EQ_A2DP_LISTEN] Received a message(%s)\n", buff);

        if (strstr(buff, "status:connect:bsa-source")) {
            if (g_bt_is_connect == BT_DISCONNECT) {
                eq_debug("[EQ_A2DP_LISTEN] bsa bluetooth source is connect\n");
                g_bt_is_connect = BT_CONNECT_BSA;
            }
        } else if (strstr(buff, "status:connect")) {
            start = strstr(buff, "address:");
            if (start == NULL) {
                eq_debug("[EQ_A2DP_LISTEN] Received a malformed connect message(%s)\n", buff);
                continue;
            }
            start += strlen("address:");
            if (g_bt_is_connect == BT_DISCONNECT) {
                //sleep(2);
                memcpy(g_bt_mac_addr, start, sizeof(g_bt_mac_addr));
                sprintf(bluealsa_device, "%s%s", "bluealsa:HCI=hci0,PROFILE=a2dp,DEV=",
                        g_bt_mac_addr);
                retry_cnt = 5;
                while (retry_cnt--) {
                    eq_debug("[EQ_A2DP_LISTEN] try open bluealsa device(%d)\n", retry_cnt + 1);
                    ret = snd_pcm_open(&audio_bt_handle, bluealsa_device,
                                       SND_PCM_STREAM_PLAYBACK, 0);
                    if (ret == 0) {
                        snd_pcm_close(audio_bt_handle);
                        g_bt_is_connect = BT_CONNECT_BLUEZ;
                        break;
                    }
                    usleep(600000); //600ms * 5 = 3s.
                }
            }
        } else if (strstr(buff, "status:disconnect")) {
            g_bt_is_connect = BT_DISCONNECT;
        } else if (strstr(buff, "status:suspend")) {
            g_system_sleep = true;
        } else if (strstr(buff, "status:resume")) {
            g_system_sleep = false;
        } else {
            eq_debug("[EQ_A2DP_LISTEN] Received a malformed message(%s)\n", buff);
        }
    }

    close(sockfd);
    return NULL;
}

static void sigpipe_handler(int sig)
{
    eq_info("[EQ] catch the signal number: %d\n", sig);
}

static int signal_handler()
{
    struct sigaction sa;

    /* Install signal handler for SIGPIPE */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigpipe_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGPIPE, &sa, NULL) < 0) {
        eq_err("sigaction() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

#if 1
/* I/O error handler */
static int eq_drc_xrun(snd_pcm_t *handle, snd_pcm_stream_t stream)
{
    snd_pcm_status_t *status;
    snd_output_t *log;
    int fatal_errors = 0, monotonic = 1, verbose = 1;
    int res;

    eq_err("[EQ] %s %d enter\n", __func__, __LINE__);

    snd_output_stdio_attach(&log, stderr, 0);

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        eq_err("[EQ] status error: %s\n", snd_strerror(res));
        // prg_exit(EXIT_FAILURE);
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        if (fatal_errors) {
            eq_err("[EQ] fatal %s: %s\n",
                    stream == SND_PCM_STREAM_PLAYBACK ? "underrun" : "overrun",
                    snd_strerror(res));
            // prg_exit(EXIT_FAILURE);
        }
        if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
            struct timespec now, diff, tstamp;
            clock_gettime(CLOCK_MONOTONIC, &now);
            snd_pcm_status_get_trigger_htstamp(status, &tstamp);
            timermsub(&now, &tstamp, &diff);
            fprintf(stderr, "%s!!! (at least %.3f ms long)\n",
                stream == SND_PCM_STREAM_PLAYBACK ? "underrun" : "overrun",
                diff.tv_sec * 1000 + diff.tv_nsec / 1000000.0);
#else
            fprintf(stderr, "%s !!!\n", "underrun");
#endif
        } else {
            struct timeval now, diff, tstamp;
            gettimeofday(&now, 0);
            snd_pcm_status_get_trigger_tstamp(status, &tstamp);
            timersub(&now, &tstamp, &diff);
            fprintf(stderr, "%s!!! (at least %.3f ms long)\n",
                stream == SND_PCM_STREAM_PLAYBACK ? "underrun" : "overrun",
                diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        }
        if (verbose) {
            fprintf(stderr, "Status:\n");
            snd_pcm_status_dump(status, log);
        }
        if ((res = snd_pcm_prepare(handle))<0) {
            eq_err("[EQ] xrun: prepare error: %s\n", snd_strerror(res));
            // prg_exit(EXIT_FAILURE);
        }
        goto out_xrun;     /* ok, data should be accepted again */
    } if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        if (verbose) {
            fprintf(stderr, "Status(DRAINING):\n");
            snd_pcm_status_dump(status, log);
        }
        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, "capture stream format change? attempting recover...\n");
            if ((res = snd_pcm_prepare(handle))<0) {
                eq_err("[EQ] xrun(DRAINING): prepare error: %s\n", snd_strerror(res));
                // prg_exit(EXIT_FAILURE);
            }
            goto out_xrun;
        }
    }
    if (verbose) {
        fprintf(stderr, "Status(R/W):\n");
        snd_pcm_status_dump(status, log);
    }
    eq_err("[EQ] read/write error, state = %s\n", snd_pcm_state_name(snd_pcm_status_get_state(status)));
    // prg_exit(EXIT_FAILURE);

out_xrun:
    snd_output_close(log);

    return 0;
}
#endif

static void usage(char *command)
{
    snd_pcm_format_t k;
    printf(
"Usage: %s [OPTION]...\n"
"\n"
"-h, --help              help\n"
"-v  --version           print current version\n"
"-s, --seconds           close sound card after playback is stopped seconds (default: 3s)\n"
"-p, --period-size       specify the size of the frame period (default: 1920)\n"
"-n, --period-counts     specify the count of the frame periods (default: 8)\n"
#if KEEPING_HW_CARD
"-P, --path-name         specify the name of playback path for RK817/RK809 codec (default: SPK)\n"
#endif
    ,
    command);
}

static long parse_long(const char *str, int *err)
{
    long val;
    char *endptr;

    errno = 0;
    val = strtol(str, &endptr, 0);

    if (errno != 0 || *endptr != '\0')
        *err = -1;
    else
        *err = 0;

    return val;
}

static void get_version(char *command)
{
    printf("%s: version " EQ_DRC_PROCESS_VERSION " by Rockchip\n", command);
}

int main(int argc, char *argv[])
{
    int err, c;
    snd_pcm_t *capture_handle, *write_handle;
    char *buffer;
    unsigned int sampleRate, channels;
    int mute_frame_thd, mute_frame, skip_frame = 0;
    /* LINE_OUT is the default output device */
    int device_flag, new_flag, last_flag;
    pthread_t a2dp_status_listen_thread;
    pthread_t user_play_status_listen_thread;
    // pthread_t user_capt_status_listen_thread;
    // pthread_t power_status_listen_thread;
    // struct rk_wake_lock* wake_lock;
    bool low_power_mode = low_power_mode_check();
    volatile bool need_close_card = false;
    char *silence_data;
    int socket_fd = -1;
    clock_t startProcTime, endProcTime;
    int mute_time = MUTE_TIME_DEFAULT;
    int option_index;

    static char *command = argv[0];
    static const char short_options[] = "hvs:p:n:P:";
    static const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"version", 0, 0, 'v'},
        {"seconds", 1, 0, 's'},
        {"period-size", 1, 0, 'p'},
        {"period-counts", 1, 0, 'n'},
#if KEEPING_HW_CARD
        {"path-name", 1, 0, 'P'},
#endif
        {0, 0, 0, 0}
    };

#if KEEPING_HW_CARD
    memset(g_path_name, 0, sizeof(g_path_name));
    strcpy(g_path_name, HW_CARD_PATH_DEFAULT);
#endif

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
    switch (c) {
    case 'h':
        usage(command);
        return 0;
    case 'v':
        get_version(command);
        return 0;
    case 's':
        mute_time = parse_long(optarg, &err);
        if (err < 0) {
            eq_err("[EQ] invalid mute_time argument '%s'\n", optarg);
            return -1;
        }
        break;
    case 'p':
        g_period_size = parse_long(optarg, &err);
        if (err < 0) {
            eq_err("[EQ] invalid g_period_size argument '%s'\n", optarg);
            return -1;
        }
        g_read_frame = g_period_size;
        break;
    case 'n':
        g_period_counts = parse_long(optarg, &err);
        if (err < 0) {
            eq_err("[EQ] invalid g_period_counts argument '%s'\n", optarg);
            return -1;
        }
        break;
#if KEEPING_HW_CARD
    case 'P':
        memset(g_path_name, 0, sizeof(g_path_name));
        strcpy(g_path_name, optarg);
        break;
#endif
    default:
            eq_err("[EQ] Try `%s --help' for more information.\n", command);
            return 1;
        }
    }

    if (g_period_size == 0 || g_period_counts == 0) {
        eq_err("[EQ] g_period_size:%d or g_period_size:%d is zero!\n",
               g_period_size, g_period_counts);
        return -1;
    }

    g_buffer_size = g_period_size * g_period_counts;

    buffer = (char *)malloc(g_read_frame * g_period_counts * CHANNEL * sizeof(int16_t));
    if (!buffer) {
        eq_err("[EQ] Alloc buffer failed\n");
        return -1;
    }

    silence_data = (char *)malloc(g_read_frame * sizeof(int16_t) * CHANNEL); /* 2ch 16bit */
    if (!silence_data) {
        eq_err("[EQ] Alloc silence_data failed\n");
        return -1;
    }

    // wake_lock = RK_wake_lock_new("eq_drc_process");

    if(signal_handler() < 0) {
        eq_err("[EQ] Install signal_handler for SIGPIPE failed\n");
        return -1;
    }

    /* Create a thread to listen for Bluetooth connection status. */
    // pthread_create(&power_status_listen_thread, NULL, power_status_listen, NULL);
    pthread_create(&user_play_status_listen_thread, NULL, user_play_status_listen, NULL);
    // pthread_create(&user_capt_status_listen_thread, NULL, user_capt_status_listen, NULL);
    pthread_create(&a2dp_status_listen_thread, NULL, a2dp_status_listen, NULL);

repeat:
    capture_handle = NULL;
    write_handle = NULL;
    err = 0;
    memset(buffer, 0, sizeof(buffer));
    memset((char *)silence_data, 0, sizeof(silence_data));
    sampleRate = SAMPLE_RATE;
    channels = CHANNEL;
    mute_frame_thd = (int)(SAMPLE_RATE * mute_time / g_read_frame);
    mute_frame = 0;
    /* LINE_OUT is the default output device */
    device_flag = DEVICE_FLAG_LINE_OUT;
    new_flag = DEVICE_FLAG_LINE_OUT;
    last_flag = DEVICE_FLAG_LINE_OUT;

    eq_debug("\n==========EQ/DRC process release version %s==============\n", EQ_DRC_PROCESS_VERSION);
    eq_debug("==========KEEPING_HW_CARD: %d===============\n", KEEPING_HW_CARD);
    eq_debug("===== g_read_frame:%d g_period_size:%d g_period_counts:%d g_buffer_size:%d =====\n",
                g_read_frame, g_period_size, g_period_counts, g_buffer_size);
#if KEEPING_HW_CARD
    eq_debug("===== g_path_name: %s =====\n", g_path_name);
#endif

    alsa_fake_device_record_open(&capture_handle, channels, sampleRate);

#if KEEPING_HW_CARD
    if (write_handle_bak != NULL) {
        snd_pcm_close(write_handle_bak);
        eq_info("[EQ] resume process and release last write_handle_bak: 0x%x\n", write_handle_bak);
        write_handle_bak = NULL;
    } else {
        eq_info("[EQ] run process first\n");
    }
#endif

    device_flag = get_device_flag();
    err = alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
    if (err < 0) {
        eq_err("LINE: %d, first open playback device failed, and repeat\n", __LINE__);
        // return -1;
        goto repeat;
    } else {
        eq_info("LINE: %d, open write_handle: 0x%x\n", __LINE__, write_handle);
    }

#if KEEPING_HW_CARD
    /* Avoid start with plugged phones and crashed during close sound card. */
    if (device_flag == DEVICE_FLAG_LINE_OUT)
        write_handle_bak = write_handle;

    eq_info("[EQ] line: %d init write_handle: 0x%x | 0x%x, device_flag: %d | %d\n",
        __LINE__, write_handle, write_handle_bak, device_flag, last_flag);
#endif

    // RK_acquire_wake_lock(wake_lock);

    while (1) {
        // startProcTime = clock();
        err = snd_pcm_readi(capture_handle, buffer, g_read_frame);
        // endProcTime = clock();
        // printf("snd_pcm_readi cost_time: %ld us\n", endProcTime - startProcTime);
        if (err != g_read_frame) {
            if (err == -ESTRPIPE) {
                eq_err("====[EQ] LINE: %d system suspend and resumed\n", __LINE__);
            } else {
                eq_err("====[EQ] LINE: %d read frame error = %d, not %d\n", __LINE__, err, g_read_frame);
            }
        }

        if (err < 0) {
            if (err == -EPIPE)
                eq_err("[EQ] Overrun occurred: %d\n", err);

            err = snd_pcm_recover(capture_handle, err, 0);
            // Still an error, need to exit.
            if (err < 0) {
                eq_err("[EQ] Error occured while recording: %s, goto repeat\n", snd_strerror(err));
                // usleep(200 * 1000);
                if (capture_handle)
                    snd_pcm_close(capture_handle);

                goto repeat;
            }
        }

        if (g_system_sleep)
            mute_frame = mute_frame_thd;
        else if(low_power_mode && is_mute_frame((short *)buffer, channels * g_read_frame))
            mute_frame ++;
        else
            mute_frame = 0;

        if (device_flag == DEVICE_FLAG_BLUETOOTH_BSA) {
            if ((g_bt_is_connect == BT_DISCONNECT) && (socket_fd >= 0)) {
                eq_debug("[EQ] bsa bt source disconnect, teardown client socket\n");
                RK_socket_client_teardown(socket_fd);
                socket_fd = -1;
            }
        }

        // eq_info("[EQ] user_play_state=%d\n", user_play_state);

        if(mute_frame >= mute_frame_thd) {
             // eq_info("[EQ] g_system_sleep=%d, power_state=%d\n", g_system_sleep, power_state);
            //usleep(30*1000);
            /* Reassign to avoid overflow */
            // memset(buffer, 0, sizeof(buffer));

            mute_frame = mute_frame_thd;
            if (write_handle) {
#if 1 // fade-out
                int64_t start = 0;
                int fade_type = FADE_OUT;
                int nb_samples = g_read_frame * g_period_counts;
                int buf_bytes = g_read_frame * channels * sizeof(short) * g_period_counts;
                short *fade_buf, *src_buf;
                int curve_type = IQSIN;

                fade_buf = (short *)calloc(buf_bytes, 1);
                if (!fade_buf) {
                    eq_err("[EQ] alloc fade_buf failed\n");
                    return -1;
                }

                src_buf = (short *)calloc(buf_bytes, 1);
                if (!src_buf) {
                    eq_err("[EQ] alloc src_buf failed\n");
                    return -1;
                }

                memcpy((void *)(src_buf), (void *)(buffer), buf_bytes);

                eq_info("[EQ] USER_PLAY_CLOSED and fade out\n");

                fade_samples_s16((uint8_t **)(&fade_buf), (uint8_t **)(&src_buf),
                            nb_samples, channels,
                            fade_type ? -1 : 1, start,
                            nb_samples, curve_type);
                memcpy((void *)(buffer), (void *)(fade_buf), buf_bytes);

                if (src_buf)
                    free(src_buf);
                if (fade_buf)
                    free(fade_buf);
#endif

#if KEEPING_HW_CARD
                if (device_flag == DEVICE_FLAG_LINE_OUT) {
                    system("amixer sset 'Playback Path' OFF");
                    eq_info("[EQ] disable Playback path and PA\n");
                    write_handle = NULL;
                } else {
                    snd_pcm_close(write_handle);
                    eq_info("[EQ]: %d Close sound card\n", __LINE__);
                    write_handle = NULL;
                }
#else
                snd_pcm_close(write_handle);
                write_handle = NULL;
#endif

                // RK_release_wake_lock(wake_lock);

                if (power_state == POWER_STATE_SUSPENDING) {
                    eq_err("[EQ] suspend and close write handle for you right now!\n");
                    power_state = POWER_STATE_SUSPEND;
                } else {
                    eq_err("[EQ] %d second no playback, close write handle for you now!\n ", mute_time);
                }

                user_play_state = USER_PLAY_CLOSED;
            }

#if KEEPING_HW_CARD
            // if (write_handle == NULL) {
                // snd_pcm_forward(write_handle_bak, g_read_frame);
                // eq_info("[EQ] forward %d frames\n", g_read_frame);
            // }
#endif

            continue;
        }

        last_flag = device_flag;
        new_flag = get_device_flag();
        if (new_flag != device_flag) {
            eq_debug("\n[EQ] Device route changed, from\"%s\" to \"%s\"\n\n",
                   get_device_name(device_flag), get_device_name(new_flag));
            device_flag = new_flag;
#if KEEPING_HW_CARD
            if (device_flag == DEVICE_FLAG_LINE_OUT) {
                if (g_fast_codec == true &&
                    last_flag == DEVICE_FLAG_LINE_OUT &&
                    last_flag == device_flag) {
                    eq_info("[EQ]: %d Do nothing, write_handle: 0x%x | 0x%x\n",
                        __LINE__, device_flag, write_handle, write_handle_bak);
                } else {
                    eq_info("[EQ]: %d Close card LINE_OUT from %d, write_handle: 0x%x | 0x%x, g_fast_codec:%d\n",
                        __LINE__, last_flag, write_handle, write_handle_bak, g_fast_codec);

                    if (write_handle) {
                        snd_pcm_close(write_handle);
                        eq_info("[EQ]: %d Close sound card\n", __LINE__);
                        write_handle = NULL;
                        write_handle_bak = NULL;
                        g_fast_codec = false;
                    }
                }
            } else {
                eq_info("[EQ]: %d Close card, %d, write_handle: 0x%x | 0x%x, g_fast_codec:%d\n",
                    __LINE__, device_flag, write_handle, write_handle_bak, g_fast_codec);
                if (write_handle == write_handle_bak) {
                    if (write_handle) {
                        snd_pcm_close(write_handle);
                        eq_info("[EQ]: %d Close sound card\n", __LINE__);
                        write_handle = NULL;
                        write_handle_bak = NULL;
                        g_fast_codec = false;
                    }
                } else {
                    if (write_handle_bak) {
                        snd_pcm_close(write_handle_bak);
                        eq_info("[EQ]: %d Close write_handle_bak card\n", __LINE__);
                        write_handle_bak = NULL;
                    }
                    if (write_handle) {
                        snd_pcm_close(write_handle);
                        eq_info("[EQ]: %d Close write_handle card\n", __LINE__);
                        write_handle = NULL;
                    }
                    g_fast_codec = false;
                }
            }
#else
            if (write_handle) {
                snd_pcm_close(write_handle);
                write_handle = NULL;
            }
#endif
        }

        // eq_info("[EQ] device_flag: %d, %s, write_handle: 0x%x\n",
        //            device_flag, get_device_name(device_flag), write_handle);

        while (write_handle == NULL && socket_fd < 0) {
            // RK_acquire_wake_lock(wake_lock);
            eq_info("[EQ] device_flag: %d, %s, write_handle: 0x%x, socket_fd: %d\n",
                    device_flag, get_device_name(device_flag), write_handle, socket_fd);
#if KEEPING_HW_CARD
            if (device_flag == DEVICE_FLAG_LINE_OUT) {
                if (g_fast_codec == true &&
                    last_flag == DEVICE_FLAG_LINE_OUT &&
                    write_handle_bak > 0) {
                    char cmd_str[64] = { 0 };

                    write_handle = write_handle_bak;

                    sprintf(cmd_str, "amixer sset 'Playback Path' %s", g_path_name);
                    system(cmd_str);
                    eq_info("[EQ] enable Playback path and PA, write_handle: 0x%x\n", write_handle);
                    // snd_pcm_forward(write_handle, g_read_frame);
                    // continue;
                } else  {
                    eq_info("EQ]: %d if switch device_flag: %d | %d and open start, write_handle: 0x%x | 0x%x g_fast_codec: %d\n",
                        __LINE__, device_flag, last_flag, write_handle, write_handle_bak, g_fast_codec);

                    err = alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
                    if (err < 0 || (write_handle == NULL && socket_fd < 0)) {
                        eq_err("[EQ] line:%d Route change failed! Using default audio path.\n", __LINE__);
                        // last_flag = DEVICE_FLAG_LINE_OUT;
                        g_bt_is_connect = BT_DISCONNECT;
                        continue;
                    } else {
                        eq_info("LINE: %d, open write_handle: 0x%x\n", __LINE__, write_handle);
                    }
                    write_handle_bak = write_handle;
                }
            } else {
                static int overflow = 0;

                eq_info("EQ]: %d else switch device_flag: %d | %d and open start, write_handle: 0x%x | 0x%x g_fast_codec: %d\n",
                        __LINE__, device_flag, last_flag, write_handle, write_handle_bak, g_fast_codec);

                // if (write_handle_bak) {
                //     snd_pcm_close(write_handle_bak);
                //     write_handle_bak = NULL;
                //     g_fast_codec = false;
                //     eq_info("EQ]: %d close g_fast_codec\n", __LINE__);
                // }

                err = alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
                if (err < 0 || (write_handle == NULL && socket_fd < 0)) {
                    eq_err("[EQ] line:%d Route change failed!\n", __LINE__);
                    // device_flag = DEVICE_FLAG_LINE_OUT;

                    if (device_flag == DEVICE_FLAG_DIGITAL_HP) {
                        /* Maybe need to more prepare some time for digital headphone */
                        usleep(200 * 1000);
                        if (overflow++ >= 12) {
                            /* about 3s */
                            eq_err("[EQ] line:%d Using default audio path: %d, overflow: %d\n",
                                __LINE__, DEVICE_FLAG_LINE_OUT, overflow);
                            last_flag = DEVICE_FLAG_DIGITAL_HP;
                            device_flag = DEVICE_FLAG_LINE_OUT;
                            overflow = 0;
                        }

                        // need_close_card = true;
                    } else {
                        eq_err("[EQ] line:%d device_flag will: %d to %d\n",
                                __LINE__, last_flag, DEVICE_FLAG_LINE_OUT);
                        if (device_flag == DEVICE_FLAG_BLUETOOTH ||
                            device_flag == DEVICE_FLAG_BLUETOOTH_BSA) {
                            g_bt_is_connect = BT_DISCONNECT;
                        }

                        last_flag = device_flag;
                        device_flag = DEVICE_FLAG_LINE_OUT;
                    }
                    // else if (device_flag == DEVICE_FLAG_ANALOG_HP) {
                    //     last_flag = DEVICE_FLAG_ANALOG_HP;
                    //     device_flag = DEVICE_FLAG_LINE_OUT;
                    //     need_close_card = true;
                    // }

                    // g_bt_is_connect = BT_DISCONNECT;
                    continue;
                } else {
                    eq_err("[EQ] line:%d Clean overflow:%d, write_handle: 0x%x\n", __LINE__, overflow, write_handle);
                    overflow = 0;
                }

                if (write_handle_bak) {
                    snd_pcm_close(write_handle_bak);
                    write_handle_bak = NULL;
                    g_fast_codec = false;
                    eq_info("EQ]: %d close g_fast_codec\n", __LINE__);
                }
            }
#else
            err = alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
            if (err < 0 || (write_handle == NULL && socket_fd < 0)) {
                eq_err("[EQ] Route change failed! Using default audio path.\n");
                device_flag = DEVICE_FLAG_LINE_OUT;
                g_bt_is_connect = BT_DISCONNECT;
            }
#endif

            skip_frame = 0;

            // memset(buffer, 0xff, sizeof(buffer));

            // if (capture_handle)
            //         snd_pcm_close(capture_handle);
            // alsa_fake_device_record_open(&capture_handle, channels, sampleRate);

            if (0 && low_power_mode) {
                int i, num = 4;
                eq_debug("[EQ] feed mute data %d frame\n", num);
                for (i = 0; i < num; i++) {
                    if(write_handle != NULL) {
                        err = snd_pcm_writei(write_handle, silence_data, g_read_frame);
                        if(err != g_read_frame)
                            eq_err("====[EQ] %d, write frame error = %d, not %d\n", __LINE__, err, g_read_frame);
                    } else if (socket_fd >= 0) {
                        err = RK_socket_send(socket_fd, silence_data, g_read_frame * 4); //2ch 16bit
                        if(err != (g_read_frame * 4))
                            eq_err("====[EQ] %d, write frame error = %d, not %d\n", __LINE__, err, g_read_frame * 4);
                    }
                }
            }
        }

        if(write_handle != NULL) {
#if 0
            if (skip_frame > 0) {
                int err;
                err = snd_pcm_writei(write_handle, silence_data, g_read_frame);
                if(err != g_read_frame)
                    eq_err("====[EQ] %d, write frame error = %d, not %d\n", __LINE__, err, g_read_frame);

                eq_err("skip_frame = %d\n", skip_frame);
                skip_frame--;
                continue;
            }
#endif

            //usleep(30*1000);
            err = snd_pcm_writei(write_handle, buffer, g_read_frame);
            if(err != g_read_frame) {
                eq_err("====[EQ] %d, write frame error = %d, not %d\n", __LINE__, err, g_read_frame);

                // if (err > 0) {
                //     snd_pcm_sframes_t frames = g_read_frame - err;
                //     startProcTime = clock();
                //     frames = snd_pcm_forward(write_handle, frames);
                //     endProcTime = clock();
                //     printf("snd_pcm_forward cost_time: %ld us\n", endProcTime - startProcTime);
                //     eq_err("[EQ] snd_pcm_forward frames: %d\n", frames);
                // }
            }

            if (err < 0) {
                if (err == -EPIPE) {
                    eq_err("[EQ] Underrun occurred from write: %d\n", err);
#if 1
                    err = snd_pcm_recover(write_handle, err, 0);
                    if (err < 0) {
                        eq_err( "[EQ] Error occured while writing: %s\n", snd_strerror(err));
                        // usleep(200 * 1000);
    #if KEEPING_HW_CARD
                        /* Do nothing */
    #else
                        if (write_handle) {
                            snd_pcm_close(write_handle);
                            write_handle = NULL;
                        }
    #endif
                        if (device_flag == DEVICE_FLAG_BLUETOOTH)
                            g_bt_is_connect = BT_DISCONNECT;
                    }
#else
                    eq_drc_xrun(write_handle, SND_PCM_STREAM_PLAYBACK);
#endif
                }
#if KEEPING_HW_CARD
                else if (err == -EBADFD) {
                    int err;

                    eq_err("====[EQ] %d, EBADFD and re-open sound, device_flag: %d write_handle: 0x%x | 0x%x\n",
                        __LINE__, device_flag, write_handle, write_handle_bak);

                    if (write_handle) {
                        snd_pcm_close(write_handle);
                        write_handle = NULL;
                        write_handle_bak = NULL;
                        g_fast_codec = false;
                    }

                    err = alsa_fake_device_write_open(&write_handle, channels, sampleRate, device_flag, &socket_fd);
                    if (err < 0) {
                        // eq_err("LINE: %d, open playback device failed, exit eq\n", __LINE__);
                        eq_err("LINE: %d, open playback device failed, continue\n", __LINE__);
                        // write_handle_bak = write_handle;
                        // return -1;
                        continue;
                    } else {
                        eq_info("LINE: %d, open write_handle: 0x%x\n", __LINE__, write_handle);
                    }

                    write_handle_bak = write_handle;
                }
#endif
            }
        }else if (socket_fd >= 0) {
            if (g_bt_is_connect == BT_CONNECT_BSA) {
                err = RK_socket_send(socket_fd, (char *)buffer, g_read_frame * 4);
                if (err != g_read_frame * 4 && -EAGAIN != err)
                    eq_err("====[EQ] %d, write frame error = %d, not %d\n", __LINE__, err, g_read_frame * 4);

                if (err < 0 && -EAGAIN != err) {
                    if (socket_fd >= 0) {
                        eq_err("[EQ] socket send err: %d, teardown client socket\n", err);
                        RK_socket_client_teardown(socket_fd);
                        socket_fd = -1;
                    }

                    g_bt_is_connect = BT_DISCONNECT;
                }
            } else {
                if(socket_fd >= 0){
                    eq_debug("[EQ] bsa bt source disconnect, teardown client socket\n");
                    RK_socket_client_teardown(socket_fd);
                    socket_fd = -1;
                }
            }
        }
    }

error:
    eq_debug("=== [EQ] Exit eq ===\n");

    if (silence_data) {
        free(silence_data);
        silence_data = NULL;
    }

    if (buffer) {
        free(buffer);
        buffer = NULL;
    }

    g_upi.stop = 1;

    if (capture_handle)
        snd_pcm_close(capture_handle);

    if (write_handle)
        snd_pcm_close(write_handle);

    if (socket_fd >= 0)
        RK_socket_client_teardown(socket_fd);

    pthread_cancel(a2dp_status_listen_thread);
    pthread_join(a2dp_status_listen_thread, NULL);

    return 0;
}

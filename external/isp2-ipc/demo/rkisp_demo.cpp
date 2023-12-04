/*
 * V4L2 video capture example
 * AUTHOT : Jacob Chen
 * DATA : 2018-02-25
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <signal.h>
#include <linux/videodev2.h>
#include "call_fun_ipc.h"
#include "drmDsp.h"
#include "rk_aiq_user_api_sysctl.h"
#include "rk_aiq_user_api_a3dlut.h"
// #include "rk_aiq_user_api_ablc.h"
#include "rk_aiq_user_api_imgproc.h"
#include "config.h"
#include <termios.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1

#define BUFFER_COUNT 4
#define CAPTURE_RAW_PATH "/tmp"
#define CAPTURE_CNT_FILENAME "capture_cnt"

struct buffer {
        void *start;
        size_t length;
        int export_fd;
	int sequence;
};

static char out_file[255];
static char dev_name[255];
static int width = 640;
static int height = 480;
static int format = V4L2_PIX_FMT_NV12;
static int fd = -1;
static enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
struct buffer *buffers;
static unsigned int n_buffers;
static int frame_count = -1;
FILE *fp = NULL;
static rk_aiq_sys_ctx_t* aiq_ctx = NULL;
static int silent = 0;
static int vop = 0;
static int rkaiq = 0;
static int writeFile = 0;
static int writeFileSync = 0;
static int pponeframe = 0;
static int hdrmode = 0;
static char sensor[64];

static int fd_pp_input = -1;
static int fd_isp_mp = -1;
struct buffer *buffers_mp;
static int outputCnt = 3;
static int skipCnt = 30;

static char yuv_dir_path[64];
static bool _is_yuv_dir_exist = false;
static int g_capture_yuv_num = 0x0;
static bool _is_capture_yuv;


#define DBG(...) do { if(!silent) printf(__VA_ARGS__); } while(0)
#define ERR(...) do { fprintf(stderr, __VA_ARGS__); } while (0)



static struct termios oldt;

//restore terminal settings
void restore_terminal_settings(void)
{
    // Apply saved settings
    tcsetattr(0, TCSANOW, &oldt);
}

//make terminal read 1 char at a time
void disable_terminal_return(void)
{
    struct termios newt;

    //save terminal settings
    tcgetattr(0, &oldt);
    //init new settings
    newt = oldt;
    //change settings
    newt.c_lflag &= ~(ICANON | ECHO);
    //apply settings
    tcsetattr(0, TCSANOW, &newt);

    //make sure settings will be restored when program ends
    atexit(restore_terminal_settings);
}

void test_imgproc(const rk_aiq_sys_ctx_t* ctx) {

   if (ctx == NULL) {
      return;
   }

   int key =getchar();
   printf("press key=[%c]\n",key);

    opMode_t mode;
    paRange_t range;
    expPwrLineFreq_t freq;
    rk_aiq_wb_scene_t scene;
    rk_aiq_wb_gain_t gain;
    unsigned int ct;
    antiFlickerMode_t flicker;
    switch (key)
    {
    case '0':
       rk_aiq_uapi_setExpMode(ctx, OP_MANUAL);
       printf("set exp manual\n");
       break;
    case '.':
       rk_aiq_uapi_setExpMode(ctx, OP_AUTO);
       printf("set exp auto\n");
       break;
    case '1':
       rk_aiq_uapi_getExpMode(ctx, &mode);
       printf("exp mode=%d\n",mode);
       break;
    case '2':
        range.min = 5.0f;
        range.max = 8.0f;
        rk_aiq_uapi_setExpGainRange(ctx, &range);
        printf("set gain range\n");
        break;
    case '3':
        rk_aiq_uapi_getExpGainRange(ctx, &range);
        printf("get gain range[%f,%f]\n",range.min,range.max);
        break;
    case '4':
        range.min = 10.0f;
        range.max = 30.0f;
        rk_aiq_uapi_setExpTimeRange(ctx, &range);
        printf("set time range\n");
      break;
    case '5':
        rk_aiq_uapi_getExpTimeRange(ctx, &range);
        printf("get time range[%f,%f]\n",range.min,range.max);
        break;
    case '6':
        rk_aiq_uapi_setExpPwrLineFreqMode(ctx, EXP_PWR_LINE_FREQ_50HZ);
        printf("setExpPwrLineFreqMode 50hz\n");
        break;
    case ',':
        rk_aiq_uapi_setExpPwrLineFreqMode(ctx, EXP_PWR_LINE_FREQ_60HZ);
        printf("setExpPwrLineFreqMode 60hz\n");
        break;
    case '7':
        rk_aiq_uapi_getExpPwrLineFreqMode(ctx, &freq);
        printf("getExpPwrLineFreqMode=%d\n",freq);
        break;
    case '8':
        rk_aiq_uapi_setWBMode(ctx, OP_MANUAL);
        printf("setWBMode manual\n");
        break;
    case '/':
        rk_aiq_uapi_setWBMode(ctx, OP_AUTO);
        printf("setWBMode auto\n");
        break;
    case '9':
        rk_aiq_uapi_getWBMode(ctx, &mode);
        printf("getWBMode=%d\n",mode);
        break;
    case 'a':
        rk_aiq_uapi_lockAWB(ctx);
        printf("lockAWB\n");
        break;
    case 'b':
        rk_aiq_uapi_unlockAWB(ctx);
        printf("unlockAWB\n");
        break;
    case 'c':
        rk_aiq_uapi_setMWBScene(ctx,RK_AIQ_WBCT_TWILIGHT);
        printf("setMWBScene\n");
        break;
    case 'd':
        rk_aiq_uapi_getMWBScene(ctx,&scene);
        printf("getMWBScene=%d\n",scene);
        break;
    case 'e':
        gain.rgain = 0.5f;
        gain.grgain = 0.5f;
        gain.gbgain = 0.5f;
        gain.bgain = 0.5f;
        rk_aiq_uapi_setMWBGain(ctx,&gain);
        printf("setMWBGain\n");
        break;
    case 'f':
        rk_aiq_uapi_getMWBGain(ctx,&gain);
        printf("getMWBGain=[%f %f %f %f]\n",gain.rgain,gain.grgain,gain.gbgain,gain.bgain);
        break;
    case 'g':
        ct = 5500;
        rk_aiq_uapi_setMWBCT(ctx, &ct);
        printf("setMWBCT\n");
        break;
    case 'h':
        rk_aiq_uapi_getMWBCT(ctx,&ct);
        printf("getMWBCT=[%d]\n",ct);
        break;
    case 'i':
        rk_aiq_uapi_setAntiFlickerMode(ctx,ANTIFLICKER_NORMAL_MODE);
        printf("setAntiFlickerMode normal\n");
        break;
    case 'j':
        rk_aiq_uapi_setAntiFlickerMode(ctx,ANTIFLICKER_AUTO_MODE);
        printf("setAntiFlickerMode auto\n");
        break;
    case 'k':
        rk_aiq_uapi_getAntiFlickerMode(ctx, &flicker);
        printf("getAntiFlickerMode=%d\n",flicker);
        break;
    case 'l':
        rk_aiq_uapi_setSaturation(ctx, 50);
        printf("setSaturation\n");
        break;
    case 'm':
        unsigned int level1;
        rk_aiq_uapi_getSaturation(ctx, &level1);
        printf("getSaturation=%f\n",level1);
        break;
    case 'n':
        rk_aiq_uapi_setCrSuppsn(ctx, 50.0);
        printf("setCrSuppsn\n");
        break;
    case 'o':
        unsigned int level2;
        rk_aiq_uapi_getCrSuppsn(ctx, &level2);
        printf("getCrSuppsn=%d\n",level2);
        break;
    case 'p':
        rk_aiq_uapi_setHDRMode(ctx, OP_AUTO);
        printf("setHDRMode\n");
        break;
    case 'q':
        rk_aiq_uapi_setHDRMode(ctx, OP_MANUAL);
        printf("setHDRMode\n");
        break;
    case 'r':
        rk_aiq_uapi_getHDRMode(ctx, &mode);
        printf("getHDRMode=%d\n",mode);
        break;
    case 's':
        rk_aiq_uapi_setNRMode(ctx, OP_MANUAL);
        printf("setNRMode\n");
        break;
    case 't':
        rk_aiq_uapi_getNRMode(ctx, &mode);
        printf("getNRMode=%d\n",mode);
        break;
     case 'u':
        rk_aiq_uapi_setDhzMode(ctx, OP_MANUAL);
        printf("setDhzMode\n");
        break;
    case 'v':
        rk_aiq_uapi_getDhzMode(ctx, &mode);
        printf("getDhzMode=%d\n",mode);
        break;
    default:
        break;
    }
}

static void errno_exit(const char *s)
{
        ERR("%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
        int r;
        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
}

bool get_value_from_file(const char* path, int* value)
{
    char buffer[8] = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp) {
	if (read(fp, buffer, sizeof(buffer)) <= 0)
	    printf("%s read %s failed!\n", __func__, path);
	else
	    *value = atoi(buffer);
	close(fp);
	return true;
    }

    return false;
}

static int write_yuv_to_file(const void *p,
			     int size, int sequence)
{
	char file_name[64] = {0};

	snprintf(file_name, sizeof(file_name),
			"%s/frame%d.yuv",
			yuv_dir_path,
			sequence);
	fp = fopen(file_name, "wb+");
	if (fp == NULL) {
		ERR("fopen yuv file %s failed!\n", file_name);
		return -1;
	}

	fwrite(p, size, 1, fp);
	fflush(fp);

	if (fp) {
		fclose(fp);
		fp = NULL;
	}

        for (int i = 0; i < g_capture_yuv_num; i++)
            printf("<");

	printf("\n");
	// printf("write frame%d yuv\n", sequence);

	return 0;
}

static int creat_yuv_dir(const char* path)
{
	time_t now;
	struct tm* timenow;

	if (!path)
		return -1;

	time(&now);
	timenow = localtime(&now);
	snprintf(yuv_dir_path, sizeof(yuv_dir_path),
			"%s/yuv_%04d-%02d-%02d_%02d-%02d-%02d",
			path,
			timenow->tm_year + 1900,
			timenow->tm_mon + 1,
			timenow->tm_mday,
			timenow->tm_hour,
			timenow->tm_min,
			timenow->tm_sec);

	// printf("mkdir %s for capturing yuv!\n", yuv_dir_path);

	if(mkdir(yuv_dir_path, 0755) < 0) {
		printf("mkdir %s error!!!\n", yuv_dir_path);
		return -1;
	}

	_is_yuv_dir_exist = true;

	return 0;
}

static void process_image(const void *p, int sequence,int size)
{
	if (fp && sequence >= skipCnt && outputCnt-- > 0) {
		printf(">\n");
		fwrite(p, size, 1, fp);
		fflush(fp);
	} else if (writeFileSync) {
		int ret = 0;

		if (!_is_capture_yuv) {
		    char file_name[32] = {0};

		    snprintf(file_name, sizeof(file_name), "%s/%s",
			     CAPTURE_RAW_PATH, CAPTURE_CNT_FILENAME);
		    get_value_from_file(file_name, &g_capture_yuv_num);

		    if (g_capture_yuv_num > 0)
			_is_capture_yuv = true;

		    if (sequence != 0)
			return;
		}

		if (_is_capture_yuv) {
		    if (!_is_yuv_dir_exist) {
		        creat_yuv_dir(CAPTURE_RAW_PATH);
		    }

		    if (_is_yuv_dir_exist) {
			write_yuv_to_file(p, size, sequence);
		    }

		    if (--g_capture_yuv_num == 0) {
			_is_capture_yuv = false;
			_is_yuv_dir_exist = false;
		    }
		}
	}
}

static int read_frame()
{
        struct v4l2_buffer buf;
        int i, bytesused;

        CLEAR(buf);

        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_MMAP;

        struct v4l2_plane planes[FMT_NUM_PLANES];
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
                errno_exit("VIDIOC_DQBUF");

        i = buf.index;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type)
            bytesused = buf.m.planes[0].bytesused;
        else
            bytesused = buf.bytesused;

	if (vop) {
	    drmDspFrame(width, height, buffers[i].start, DRM_FORMAT_NV12);
	}

	process_image(buffers[i].start,  buf.sequence, bytesused);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");

        return 1;
}

static int read_frame_pp_oneframe()
{
        struct v4l2_buffer buf;
        struct v4l2_buffer buf_pp;
        int i,ii, bytesused;
        static int first_time = 1;

        CLEAR(buf);
        // dq one buf from isp mp
        printf("------ dq 1 from isp mp --------------\n");
        buf.type = buf_type;
        buf.memory = V4L2_MEMORY_MMAP;

        struct v4l2_plane planes[FMT_NUM_PLANES];
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd_isp_mp, VIDIOC_DQBUF, &buf))
                errno_exit("VIDIOC_DQBUF");

        i = buf.index;

        if (first_time ) {
            printf("------ dq 2 from isp mp --------------\n");
            if (-1 == xioctl(fd_isp_mp, VIDIOC_DQBUF, &buf))
                    errno_exit("VIDIOC_DQBUF");

            ii = buf.index;
        }

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type)
            bytesused = buf.m.planes[0].bytesused;
        else
            bytesused = buf.bytesused;

        // queue to ispp input
        printf("------ queue 1 index %d to ispp input --------------\n", i);
        CLEAR(buf_pp);
        buf_pp.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        buf_pp.memory = V4L2_MEMORY_DMABUF;
        buf_pp.index = i;

        struct v4l2_plane planes_pp[FMT_NUM_PLANES];
        memset(planes_pp, 0, sizeof(planes_pp));
        buf_pp.m.planes = planes_pp;
        buf_pp.length = FMT_NUM_PLANES;
        buf_pp.m.planes[0].m.fd = buffers_mp[i].export_fd;

        if (-1 == xioctl(fd_pp_input, VIDIOC_QBUF, &buf_pp))
                errno_exit("VIDIOC_QBUF");

        if (first_time ) {
            printf("------ queue 2 index %d to ispp input --------------\n", ii);
            buf_pp.index = ii;
            buf_pp.m.planes[0].m.fd = buffers_mp[ii].export_fd;
            if (-1 == xioctl(fd_pp_input, VIDIOC_QBUF, &buf_pp))
                    errno_exit("VIDIOC_QBUF");
        }
        // read frame from ispp sharp/scale
        printf("------ readframe from output --------------\n");
        read_frame();
        // dq from pp input
        printf("------ dq 1 from ispp input--------------\n");
        if (-1 == xioctl(fd_pp_input, VIDIOC_DQBUF, &buf_pp))
                errno_exit("VIDIOC_DQBUF");
        // queue back to mp
        printf("------ queue 1 index %d back to isp mp--------------\n", buf_pp.index);
        buf.index = buf_pp.index;
        if (-1 == xioctl(fd_isp_mp, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");

        first_time = 0;
        return 1;
}

static void mainloop(void)
{
    bool loop_inf = frame_count == -1 ? true : false;

    while (loop_inf || (frame_count-- > 0)) {
        if (pponeframe)
            read_frame_pp_oneframe();
        else
            read_frame();
    }
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

        type = buf_type;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
}

static void stop_capturing_pp_oneframe(void)
{
        enum v4l2_buf_type type;

        type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        if (-1 == xioctl(fd_pp_input, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF ppinput");
        type = buf_type;
        if (-1 == xioctl(fd_isp_mp, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF ispmp");
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    struct v4l2_plane planes[FMT_NUM_PLANES];

                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
        }
        type = buf_type;
        printf("-------- stream on output -------------\n");
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
}

static void start_capturing_pp_oneframe(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        printf("-------- stream on pp input -------------\n");
        if (-1 == xioctl(fd_pp_input, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON pp input");

        type = buf_type;
        for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    struct v4l2_plane planes[FMT_NUM_PLANES];

                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }
                if (-1 == xioctl(fd_isp_mp, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
        }
        printf("-------- stream on isp mp -------------\n");
        if (-1 == xioctl(fd_isp_mp, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON ispmp");
}


static void uninit_device(void)
{
        unsigned int i;

        for (i = 0; i < n_buffers; ++i) {
            if (-1 == munmap(buffers[i].start, buffers[i].length))
                    errno_exit("munmap");

            close(buffers[i].export_fd);
        }

        free(buffers);
}

static void uninit_device_pp_oneframe(void)
{
        unsigned int i;

        for (i = 0; i < n_buffers; ++i) {
            if (-1 == munmap(buffers_mp[i].start, buffers_mp[i].length))
                    errno_exit("munmap");

            close(buffers_mp[i].export_fd);
        }

        free(buffers_mp);
}

static void init_mmap(int pp_onframe)
{
        struct v4l2_requestbuffers req;
        int fd_tmp = -1;

        CLEAR(req);

        if (pp_onframe)
            fd_tmp = fd_isp_mp ;
        else
            fd_tmp = fd;

        req.count = BUFFER_COUNT;
        req.type = buf_type;
        req.memory = V4L2_MEMORY_MMAP;

        struct buffer *tmp_buffers = NULL;

        if (-1 == xioctl(fd_tmp, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        ERR("%s does not support "
                                 "memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                ERR("Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }

        tmp_buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));

        if (!tmp_buffers) {
                ERR("Out of memory\n");
                exit(EXIT_FAILURE);
        }

        if (pp_onframe)
            buffers_mp = tmp_buffers;
        else
            buffers = tmp_buffers;

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;
                struct v4l2_plane planes[FMT_NUM_PLANES];
                CLEAR(buf);
                CLEAR(planes);

                buf.type = buf_type;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    buf.m.planes = planes;
                    buf.length = FMT_NUM_PLANES;
                }

                if (-1 == xioctl(fd_tmp, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
                    tmp_buffers[n_buffers].length = buf.m.planes[0].length;
                    tmp_buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.m.planes[0].length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd_tmp, buf.m.planes[0].m.mem_offset);
                } else {
                    tmp_buffers[n_buffers].length = buf.length;
                    tmp_buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd_tmp, buf.m.offset);
                }

                if (MAP_FAILED == tmp_buffers[n_buffers].start)
                        errno_exit("mmap");

                // export buf dma fd
                struct v4l2_exportbuffer expbuf;
                xcam_mem_clear (expbuf);
                expbuf.type = buf_type;
                expbuf.index = n_buffers;
                expbuf.flags = O_CLOEXEC;
                if (xioctl(fd_tmp, VIDIOC_EXPBUF, &expbuf) < 0) {
                    errno_exit("get dma buf failed\n");
                } else {
                    printf("get dma buf(%d)-fd: %d\n", n_buffers, expbuf.fd);
                }
                tmp_buffers[n_buffers].export_fd = expbuf.fd;
        }
}

static void init_input_dmabuf_oneframe(void) {
        struct v4l2_requestbuffers req;

        CLEAR(req);

        printf("-------- request pp input buffer -------------\n");
        req.count = BUFFER_COUNT;
        req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        req.memory = V4L2_MEMORY_DMABUF;

        if (-1 == xioctl(fd_pp_input, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        ERR("does not support "
                                 "DMABUF\n");
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                ERR("Insufficient buffer memory on %s\n",
                         dev_name);
                exit(EXIT_FAILURE);
        }
    printf("-------- request isp mp buffer -------------\n");
    init_mmap(true);
}

static void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_format fmt;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        ERR("%s is no V4L2 device\n",
                                 dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
                !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
            ERR("%s is not a video capture device, capabilities: %x\n",
                         dev_name, cap.capabilities);
                exit(EXIT_FAILURE);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                ERR("%s does not support streaming i/o\n",
                    dev_name);
                exit(EXIT_FAILURE);
        }

        if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
            buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
            buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

        CLEAR(fmt);
        fmt.type = buf_type;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                errno_exit("VIDIOC_S_FMT");

	init_mmap(false);
}

static void init_device_pp_oneframe(void)
{
    // TODO, set format and link, now do with setup_link.sh
    init_input_dmabuf_oneframe();
}

static void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

static void open_device(void)
{
        printf("-------- open output dev_name -------------\n");
        fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == fd) {
                ERR("Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

static void close_device_pp_oneframe(void)
{
        if (-1 == close(fd_pp_input))
                errno_exit("close");

        fd_pp_input = -1;

        if (-1 == close(fd_isp_mp))
                errno_exit("close");

        fd_isp_mp = -1;
}

static void open_device_pp_oneframe(void)
{
        printf("-------- open pp input(video13) -------------\n");
        fd_pp_input = open("/dev/video13", O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == fd_pp_input) {
                ERR("Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        printf("-------- open isp mp(video0) -------------\n");
        fd_isp_mp = open("/dev/video0", O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == fd_isp_mp ) {
                ERR("Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

static void uninit_device_pp_onframe(void)
{
        unsigned int i;

        for (i = 0; i < n_buffers; ++i) {
                if (-1 == munmap(buffers_mp[i].start, buffers_mp[i].length))
                        errno_exit("munmap");
                close(buffers_mp[i].export_fd);
        }

        free(buffers_mp);
}

void parse_args(int argc, char **argv)
{
   int c;
   int digit_optind = 0;

   while (1) {
       int this_option_optind = optind ? optind : 1;
       int option_index = 0;
       static struct option long_options[] = {
           {"width",    required_argument, 0, 'w' },
           {"height",   required_argument, 0, 'h' },
           {"format",   required_argument, 0, 'f' },
           {"device",   required_argument, 0, 'd' },
           {"stream-to",   required_argument, 0, 'o' },
           {"stream-count",   required_argument, 0, 'n' },
           {"stream-skip",   required_argument, 0, 'k' },
           {"count",    required_argument, 0, 'c' },
           {"help",     no_argument,       0, 'p' },
           {"silent",   no_argument,       0, 's' },
           {"vop",   no_argument,       0, 'v' },
           {"rkaiq",   no_argument,       0, 'r' },
           {"pponeframe",   no_argument,       0, 'm' },
           {"hdr",   no_argument,       0, 'a' },
           {"sensor",   required_argument,       0, 'b' },
           {"sync-to-raw", no_argument, 0, 'e' },
           {0,          0,                 0,  0  }
       };

       //c = getopt_long(argc, argv, "w:h:f:i:d:o:c:ps",
       c = getopt_long(argc, argv, "w:h:f:i:d:o:c:n:k:m:pse",
           long_options, &option_index);
       if (c == -1)
           break;

       switch (c) {
       case 'c':
           frame_count = atoi(optarg);
           break;
       case 'w':
           width = atoi(optarg);
           break;
       case 'h':
           height = atoi(optarg);
           break;
       case 'f':
           format = v4l2_fourcc(optarg[0], optarg[1], optarg[2], optarg[3]);
           break;
       case 'd':
           strcpy(dev_name, optarg);
           break;
       case 'o':
           strcpy(out_file, optarg);
           writeFile = 1;
	   break;
       case 'n':
           outputCnt = atoi(optarg);
	   break;
       case 'k':
           skipCnt = atoi(optarg);
	   break;
       case 's':
           silent = 1;
           break;
       case 'v':
           vop = 1;
           break;
       case 'r':
           rkaiq = 1;
           break;
       case 'm':
           pponeframe = 1;
           break;
       case 'a':
           hdrmode = 1;
           break;
       case 'b':
           strcpy(sensor, optarg);
           break;
       case 'e':
	   writeFileSync = 1;
           break;
       case '?':
       case 'p':
           ERR("Usage: %s to capture rkisp1 frames\n"
                  "         --width,  default 640,             optional, width of image\n"
                  "         --height, default 480,             optional, height of image\n"
                  "         --format, default NV12,            optional, fourcc of format\n"
                  "         --count,  default 1000,            optional, how many frames to capture\n"
                  "         --device,                          required, path of video device\n"
                  "         --stream-to,                       optional, output file path, if <file> is '-', then the data is written to stdout\n"
                  "         --stream-count, default 3	       optional, how many frames to write files\n"
                  "         --stream-skip, default 30	       optional, how many frames to skip befor writing file\n"
                  "         --vop,                             optional, drm display\n"
                  "         --rkaiq,                           optional, auto image quality\n",
                  "         --silent,                          optional, subpress debug log\n",
                  "         --pponeframe,                      optional, pp oneframe readback mode\n",
                  "         --hdr,                             optional, hdr mode\n",
                  "         --sensor,  default os04a10,        optional, sensor names\n",
                  "         --sync-to-raw,		       optional, write yuv files in sync with raw\n",
                  argv[0]);
           exit(-1);

       default:
           ERR("?? getopt returned character code 0%o ??\n", c);
       }
   }

   if (strlen(dev_name) == 0) {
        ERR("arguments --output and --device are required\n");
        exit(-1);
   }

}

static void deinit()
{
    stop_capturing();
    if (pponeframe)
        stop_capturing_pp_oneframe();
	if (aiq_ctx) {
        printf("-------- stop aiq -------------\n");
		rk_aiq_uapi_sysctl_stop(aiq_ctx, false);
        printf("-------- deinit aiq -------------\n");
		rk_aiq_uapi_sysctl_deinit(aiq_ctx);
        printf("-------- deinit aiq end -------------\n");
	}

    //stop_capturing();
    uninit_device();
    if (pponeframe)
        uninit_device_pp_oneframe();
    close_device();
    if (pponeframe)
        close_device_pp_oneframe();

    if (fp)
        fclose(fp);
}
static void signal_handle(int signo)
{
    printf("force exit signo %d !!!\n",signo);
    deinit();
    exit(0);
}
static void main_exit(void)
{
    printf("client %s\n", __func__);
    call_fun_ipc_client_deinit();
}

void signal_crash_handler(int sig)
{
    exit(-1);
}

void signal_exit_handler(int sig)
{
    exit(0);
}

void* test_thread(void* args) {
    pthread_detach (pthread_self());
    disable_terminal_return();
    printf("begin test imgproc\n");
    while(1) {
        test_imgproc(aiq_ctx);
    }
    printf("end test imgproc\n");
    restore_terminal_settings();
}


int main(int argc, char **argv)
{
    signal(SIGINT, signal_handle);
    atexit(main_exit);
    signal(SIGTERM, signal_exit_handler);
    //signal(SIGINT, signal_exit_handler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, signal_crash_handler);
    signal(SIGSEGV, signal_crash_handler);
    signal(SIGFPE, signal_crash_handler);
    signal(SIGABRT, signal_crash_handler);
    strcpy(sensor, "os04a10");
    parse_args(argc, argv);

    printf("-------- open output dev -------------\n");
    open_device();
    if (pponeframe)
        open_device_pp_oneframe();
    init_device();
    if (pponeframe)
        init_device_pp_oneframe();
    if (writeFile) {
        fp = fopen(out_file, "w+");
        if (fp == NULL) {
            ERR("fopen output file %s failed!\n", out_file);
            exit(-1);
        }
    }

    const char* sns_entity_name = NULL;
    rk_aiq_working_mode_t work_mode = RK_AIQ_WORKING_MODE_NORMAL;

    if (strcasecmp(sensor, "ov4689") == 0) {
        if (hdrmode)
            work_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
        sns_entity_name = "m01_f_ov4689 1-0036";
    } else if (strcasecmp(sensor, "os04a10") == 0) {
        if (hdrmode)
            work_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
        sns_entity_name = "m01_f_os04a10 1-0036";
    } else if (strcasecmp(sensor, "gc4c33") == 0) {
        if (hdrmode)
            work_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
        sns_entity_name = "m01_f_gc4c33 1-0029";
    } else if (strcasecmp(sensor, "imx347") == 0) {
        if (hdrmode)
            work_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
        sns_entity_name = "m01_f_imx347 1-0037";
    }

    printf("sns_entity_name %s\n", sns_entity_name);
	if (rkaiq) {
        printf("+++++call_fun_ipc_client_init\n");
        //int r = 0xFEFEFEFE;
      //  while (r == 0xFEFEFEFE) {
        call_fun_ipc_client_init(DBUS_NAME, DBUS_IF, DBUS_PATH, SHARE_PATH, 1);
         //   usleep(1000*1000);
        //    printf("=======call_fun_ipc_client_init, r=0x%x\n",r);
      //  }

		aiq_ctx = rk_aiq_uapi_sysctl_init(sns_entity_name,NULL, NULL, NULL);

		if (aiq_ctx) {
            printf("-------- init mipi tx/rx -------------\n");
			XCamReturn ret = rk_aiq_uapi_sysctl_prepare(aiq_ctx, width, height, work_mode);
			if (ret != XCAM_RETURN_NO_ERROR)
				ERR("rk_aiq_uapi_sysctl_prepare failed: %d\n", ret);
			else {
                /* printf("-------- stream on mipi tx/rx -------------\n"); */
				/* ret = rk_aiq_uapi_sysctl_start(aiq_ctx ); */
				start_capturing();
                if (pponeframe)
                    start_capturing_pp_oneframe();
                printf("-------- stream on mipi tx/rx -------------\n");
				ret = rk_aiq_uapi_sysctl_start(aiq_ctx);
                printf("-------------stream on mipi end\n");
			}

			if (vop) {
				if (initDrmDsp() < 0) {
					printf("DRM display init failed\n");
					exit(0);
				}
			}

			usleep(500 * 1000);
		}
	} else {
		if (vop) {
			if (initDrmDsp() < 0) {
				printf("DRM display init failed\n");
				exit(0);
			}
		}

		usleep(500 * 1000);

		start_capturing();
        if (pponeframe)
            start_capturing_pp_oneframe();
	}
#if ENABLE_TEST
    pthread_t tid;
    pthread_create(&tid, NULL, (void*)test_thread, NULL);
#endif
    mainloop();

    deinit();

    return 0;
}

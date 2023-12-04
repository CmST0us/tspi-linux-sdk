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

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>

#include "uvc-gadget.h"
#include "uvc_log.h"
#include "uvc_video.h"
#include "uvc_encode.h"

#ifdef CAMERA_CONTROL
#include "camera_pu_control.h"
#include "camera_control.h"
#endif
#if USE_RK_AISERVER
#include "uvc_ipc_ext.h"
#endif
extern void camera_control_set_eptz(int val);
extern void camera_control_set_zoom(int val);

/* Enable debug prints. */
//#define ENABLE_BUFFER_DEBUG
#define ENABLE_USB_REQUEST_DEBUG

#define CLEAR(x)    memset (&(x), 0, sizeof (x))
#define max(a, b)   (((a) > (b)) ? (a) : (b))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define clamp(val, min, max) ({                 \
        typeof(val) __val = (val);              \
        typeof(min) __min = (min);              \
        typeof(max) __max = (max);              \
        (void) (&__val == &__min);              \
        (void) (&__val == &__max);              \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })

#define pixfmtstr(x)    (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, \
            ((x) >> 24) & 0xff

static int silent = 1;
#define DBG(...) do { if(silent) LOG_INFO(__VA_ARGS__); } while(0)

/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */
#define PU_BRIGHTNESS_MIN_VAL       0
#define PU_BRIGHTNESS_MAX_VAL       100
#define PU_BRIGHTNESS_STEP_SIZE     1
#define PU_BRIGHTNESS_DEFAULT_VAL   50

#define PU_CONTRAST_MIN_VAL         0
#define PU_CONTRAST_MAX_VAL         100
#define PU_CONTRAST_STEP_SIZE       1
#define PU_CONTRAST_DEFAULT_VAL     50

#define PU_HUE_MIN_VAL              0
#define PU_HUE_MAX_VAL              100
#define PU_HUE_STEP_SIZE            1
#define PU_HUE_DEFAULT_VAL          50

#define PU_SATURATION_MIN_VAL       0
#define PU_SATURATION_MAX_VAL       100
#define PU_SATURATION_STEP_SIZE     1
#define PU_SATURATION_DEFAULT_VAL   50

#define PU_SHARPNESS_MIN_VAL        0
#define PU_SHARPNESS_MAX_VAL        100
#define PU_SHARPNESS_STEP_SIZE      1
#define PU_SHARPNESS_DEFAULT_VAL    50

#define PU_GAMMA_MIN_VAL            1
#define PU_GAMMA_MAX_VAL            500
#define PU_GAMMA_STEP_SIZE          1
#define PU_GAMMA_DEFAULT_VAL        100 //1.0

#define PU_WHITE_BALANCE_TEMPERATURE_MIN_VAL        2800
#define PU_WHITE_BALANCE_TEMPERATURE_MAX_VAL        6500
#define PU_WHITE_BALANCE_TEMPERATURE_STEP_SIZE      37
#define PU_WHITE_BALANCE_TEMPERATURE_DEFAULT_VAL    4650

#define PU_GAIN_MIN_VAL             0
#define PU_GAIN_MAX_VAL             5
#define PU_GAIN_STEP_SIZE           1
#define PU_GAIN_DEFAULT_VAL         1

#define PU_HUE_AUTO_DEFAULT_VAL     0

//ZOOM
#define CT_ZOOM_ABSOLUTE_CONTROL_MIN_VAL         10
#define CT_ZOOM_ABSOLUTE_CONTROL_MAX_VAL         50
#define CT_ZOOM_ABSOLUTE_CONTROL_STEP_SIZE       1
#define CT_ZOOM_ABSOLUTE_CONTROL_DEFAULT_VAL     10

//PANTILT
#define CT_PANTILT_ABSOLUTE_CONTROL_MIN_VAL         -36000
#define CT_PANTILT_ABSOLUTE_CONTROL_MAX_VAL          36000
#define CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE        3600
#define CT_PANTILT_ABSOLUTE_CONTROL_DEFAULT_VAL      0

//ROLL
#define CT_ROLL_ABSOLUTE_CONTROL_MIN_VAL         0
#define CT_ROLL_ABSOLUTE_CONTROL_MAX_VAL         3
#define CT_ROLL_ABSOLUTE_CONTROL_STEP_SIZE       1
#define CT_ROLL_ABSOLUTE_CONTROL_DEFAULT_VAL     0

//EXPOSURE_TIME_ABSOLUTE
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_MIN_VAL         5
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_MAX_VAL         2500
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_STEP_SIZE       1    //100us
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_DEFAULT_VAL     156  // 5 10 20 39 78 156 312 625 1250 2500 (-11 ~ -2)

//IRIS_ABSOLUTE
#define CT_IRIS_ABSOLUTE_CONTROL_MIN_VAL         0
#define CT_IRIS_ABSOLUTE_CONTROL_MAX_VAL         10
#define CT_IRIS_ABSOLUTE_CONTROL_STEP_SIZE       1
#define CT_IRIS_ABSOLUTE_CONTROL_DEFAULT_VAL     5

#define PU_DIGITAL_MULTIPLIER_CONTROL_MIN_VAL         10
#define PU_DIGITAL_MULTIPLIER_CONTROL_MAX_VAL         50
#define PU_DIGITAL_MULTIPLIER_CONTROL_STEP_SIZE       1
#define PU_DIGITAL_MULTIPLIER_CONTROL_DEFAULT_VAL     10

#define XU_CAMERA_VERSION_DEFAULT  "1100010233010110010"
#define XU_EPTZ_FLAG_DEFAULT_VAL     0
#define XU_H265_DEFAULT_VAL     0

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */
extern struct uvc_encode uvc_enc;

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */

/* Represents a V4L2 based video capture device */
struct v4l2_device
{
    /* v4l2 device specific */
    int v4l2_fd;
    int is_streaming;
    char *v4l2_devname;

    /* v4l2 buffer specific */
    enum io_method io;
    struct buffer *mem;
    unsigned int nbufs;

    /* v4l2 buffer queue and dequeue counters */
    unsigned long long int qbuf_count;
    unsigned long long int dqbuf_count;

    /* uvc device hook */
    struct uvc_device *udev;
};

void update_camera_ip(struct uvc_device *dev)
{
    char cmd[32] = {0};
    char ip[20] = {0};
    int num = snprintf(ip, sizeof(ip), "%d.%d.%d.%d", dev->ex_ip_data[0], dev->ex_ip_data[1],
                       dev->ex_ip_data[2], dev->ex_ip_data[3]);
    snprintf(cmd, 32, "ifconfig usb0 %d.%d.%d.%d", dev->ex_ip_data[0], dev->ex_ip_data[1],
             dev->ex_ip_data[2], dev->ex_ip_data[3]);
    //system("ifconfig usb0 down");
    LOG_DEBUG("update_camera_ip num:%d,cmd:%s\n", num, cmd);
    system(cmd);
    //system("ifconfig usb0 up");

    char *next = "/data/uvc_xu_ip_save";
    FILE *fp_output = NULL;
    strncpy(cmd, next, 32);
    cmd[strlen(next)] = '\0';
    fp_output = fopen(cmd, "w+b");
    if (NULL == fp_output)
    {
        LOG_ERROR("failed to open uvc xu ip file %s\n", cmd);
    }
    else
    {
        ip[num + 1] = "\0";
        fwrite(ip, num + 1, 1, fp_output);
        fclose(fp_output);
    }
}

/* ---------------------------------------------------------------------------
 * V4L2 streaming related
 */

static int
v4l2_uninit_device(struct v4l2_device *dev)
{
    unsigned int i;
    int ret;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        for (i = 0; i < dev->nbufs; ++i)
        {
            ret = munmap(dev->mem[i].start, dev->mem[i].length);
            if (ret < 0)
            {
                LOG_ERROR("V4L2: munmap failed\n");
                return ret;
            }
        }

        free(dev->mem);
        break;

    case IO_METHOD_USERPTR:
    default:
        break;
    }

    return 0;
}

static int
v4l2_reqbufs_mmap(struct v4l2_device *dev, int nbufs)
{
    struct v4l2_requestbuffers req;
    unsigned int i = 0;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0)
    {
        if (ret == -EINVAL)
            LOG_ERROR("V4L2: does not support memory mapping\n");
        else
            LOG_ERROR("V4L2: VIDIOC_REQBUFS error %s (%d).\n",
                     strerror(errno), errno);
        goto err;
    }

    if (!req.count)
        return 0;

    if (req.count < 2)
    {
        LOG_ERROR("V4L2: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = calloc(req.count, sizeof dev->mem[0]);
    if (!dev->mem)
    {
        LOG_ERROR("V4L2: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < req.count; ++i)
    {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0)
        {
            LOG_ERROR("V4L2: VIDIOC_QUERYBUF failed for buf %d: "
                     "%s (%d).\n", i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].start = mmap(NULL /* start anywhere */,
                                 dev->mem[i].buf.length,
                                 PROT_READ | PROT_WRITE /* required */,
                                 MAP_SHARED /* recommended */,
                                 dev->v4l2_fd, dev->mem[i].buf.m.offset);

        if (MAP_FAILED == dev->mem[i].start)
        {
            LOG_ERROR("V4L2: Unable to map buffer %u: %s (%d).\n", i,
                     strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].length = dev->mem[i].buf.length;
        LOG_DEBUG("V4L2: Buffer %u mapped at address %p.\n", i,
                 dev->mem[i].start);
    }

    dev->nbufs = req.count;
    LOG_DEBUG("V4L2: %u buffers allocated.\n", req.count);

    return 0;

err_free:
    free(dev->mem);
err:
    return ret;
}

static int
v4l2_reqbufs_userptr(struct v4l2_device *dev, int nbufs)
{
    struct v4l2_requestbuffers req;
    int ret;

    CLEAR(req);

    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->v4l2_fd, VIDIOC_REQBUFS, &req);
    if (ret < 0)
    {
        if (ret == -EINVAL)
            LOG_ERROR("V4L2: does not support user pointer i/o\n");
        else
            LOG_ERROR("V4L2: VIDIOC_REQBUFS error %s (%d).\n",
                     strerror(errno), errno);
        return ret;
    }

    dev->nbufs = req.count;
    LOG_DEBUG("V4L2: %u buffers allocated.\n", req.count);

    return 0;
}

static int
v4l2_reqbufs(struct v4l2_device *dev, int nbufs)
{
    int ret = 0;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        ret = v4l2_reqbufs_mmap(dev, nbufs);
        break;

    case IO_METHOD_USERPTR:
        ret = v4l2_reqbufs_userptr(dev, nbufs);
        break;

    default:
        LOG_ERROR("no support such io:%d\n", dev->io);
        ret = -EINVAL;
        break;
    }

    return ret;
}

static int
v4l2_qbuf_mmap(struct v4l2_device *dev)
{
    unsigned int i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i)
    {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->v4l2_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0)
        {
            LOG_ERROR("V4L2: VIDIOC_QBUF failed : %s (%d).\n",
                     strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;
    }

    return 0;
}

static int
v4l2_qbuf(struct v4l2_device *dev)
{
    int ret = 0;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        ret = v4l2_qbuf_mmap(dev);
        break;

    case IO_METHOD_USERPTR:
        /* Empty. */
        ret = 0;
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

static int
v4l2_process_data(struct v4l2_device *dev)
{
    int ret;
    struct v4l2_buffer vbuf;
    struct v4l2_buffer ubuf;

    /* Return immediately if V4l2 streaming has not yet started. */
    if (!dev->is_streaming)
        return 0;

    if (dev->udev->first_buffer_queued)
        if (dev->dqbuf_count >= dev->qbuf_count)
            return 0;

    /* Dequeue spent buffer rom V4L2 domain. */
    CLEAR(vbuf);

    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    switch (dev->io)
    {
    case IO_METHOD_USERPTR:
        vbuf.memory = V4L2_MEMORY_USERPTR;
        break;

    case IO_METHOD_MMAP:
    default:
        vbuf.memory = V4L2_MEMORY_MMAP;
        break;
    }

    ret = ioctl(dev->v4l2_fd, VIDIOC_DQBUF, &vbuf);
    if (ret < 0)
        return ret;

    dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    LOG_INFO("Dequeueing buffer at V4L2 side = %d\n", vbuf.index);
#endif

    /* Queue video buffer to UVC domain. */
    CLEAR(ubuf);

    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->udev->io)
    {
    case IO_METHOD_MMAP:
        ubuf.memory = V4L2_MEMORY_MMAP;
        ubuf.length = vbuf.length;
        ubuf.index = vbuf.index;
        ubuf.bytesused = vbuf.bytesused;
        break;

    case IO_METHOD_USERPTR:
    default:
        ubuf.memory = V4L2_MEMORY_USERPTR;
        ubuf.m.userptr = (unsigned long) dev->mem[vbuf.index].start;
        ubuf.length = dev->mem[vbuf.index].length;
        ubuf.index = vbuf.index;
        ubuf.bytesused = vbuf.bytesused;
        break;
    }

    ret = ioctl(dev->udev->uvc_fd, VIDIOC_QBUF, &ubuf);
    if (ret < 0)
    {
        LOG_ERROR("UVC: Unable to queue buffer %d: %s (%d).\n",
                 ubuf.index, strerror(errno), errno);
        /* Check for a USB disconnect/shutdown event. */
        if (errno == ENODEV)
        {
            dev->udev->uvc_shutdown_requested = 1;
            LOG_INFO("UVC: Possible USB shutdown requested from "
                     "Host, seen during VIDIOC_QBUF\n");
            return 0;
        }
        else
        {
            return ret;
        }
    }

    dev->udev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
    LOG_INFO("Queueing buffer at UVC side = %d\n", ubuf.index);
#endif

    if (!dev->udev->first_buffer_queued && !dev->udev->run_standalone)
    {
        uvc_video_stream(dev->udev, 1);
        dev->udev->first_buffer_queued = 1;
        dev->udev->is_streaming = 1;
    }

    return 0;
}

/* ---------------------------------------------------------------------------
 * V4L2 generic stuff
 */

static int
v4l2_get_format(struct v4l2_device *dev)
{
    struct v4l2_format fmt;
    int ret;

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(dev->v4l2_fd, VIDIOC_G_FMT, &fmt);
    if (ret < 0)
    {
        LOG_ERROR("V4L2: Unable to get format: %s (%d).\n",
                 strerror(errno), errno);
        return ret;
    }

    LOG_DEBUG("V4L2: Getting current format: %c%c%c%c %ux%u\n",
             pixfmtstr(fmt.fmt.pix.pixelformat),
             fmt.fmt.pix.width, fmt.fmt.pix.height);

    return 0;
}

static int
v4l2_set_format(struct v4l2_device *dev, struct v4l2_format *fmt)
{
    int ret;

    ret = ioctl(dev->v4l2_fd, VIDIOC_S_FMT, fmt);
    if (ret < 0)
    {
        LOG_ERROR("V4L2: Unable to set format %s (%d).\n",
                 strerror(errno), errno);
        return ret;
    }

    LOG_DEBUG("V4L2: Setting format to: %c%c%c%c %ux%u\n",
             pixfmtstr(fmt->fmt.pix.pixelformat),
             fmt->fmt.pix.width, fmt->fmt.pix.height);

    return 0;
}

static int
v4l2_set_ctrl(struct v4l2_device *dev, int new_val, int ctrl)
{
    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;
    int ret;

    CLEAR(queryctrl);

    switch (ctrl)
    {
    case V4L2_CID_BRIGHTNESS:
        queryctrl.id = V4L2_CID_BRIGHTNESS;
        ret = ioctl(dev->v4l2_fd, VIDIOC_QUERYCTRL, &queryctrl);
        if (-1 == ret)
        {
            if (errno != EINVAL)
                LOG_ERROR("V4L2: VIDIOC_QUERYCTRL"
                         " failed: %s (%d).\n",
                         strerror(errno), errno);
            else
                LOG_ERROR("V4L2_CID_BRIGHTNESS is not"
                         " supported: %s (%d).\n",
                         strerror(errno), errno);

            return ret;
        }
        else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        {
            LOG_ERROR("V4L2_CID_BRIGHTNESS is not supported.\n");
            ret = -EINVAL;
            return ret;
        }
        else
        {
            CLEAR(control);
            control.id = V4L2_CID_BRIGHTNESS;
            control.value = new_val;

            ret = ioctl(dev->v4l2_fd, VIDIOC_S_CTRL, &control);
            if (-1 == ret)
            {
                LOG_ERROR("V4L2: VIDIOC_S_CTRL failed: %s (%d).\n",
                         strerror(errno), errno);
                return ret;
            }
        }
        LOG_INFO("V4L2: Brightness control changed to value = 0x%x\n",
                 new_val);
        break;

    default:
        /* TODO: We don't support any other controls. */
        return -EINVAL;
    }

    return 0;
}

static int
v4l2_start_capturing(struct v4l2_device *dev)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;

    ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMON, &type);
    if (ret < 0)
    {
        LOG_ERROR("V4L2: Unable to start streaming: %s (%d).\n",
                 strerror(errno), errno);
        return ret;
    }

    LOG_DEBUG("V4L2: Starting video stream.\n");

    return 0;
}

static int
v4l2_stop_capturing(struct v4l2_device *dev)
{
    enum v4l2_buf_type type;
    int ret;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ret = ioctl(dev->v4l2_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0)
        {
            LOG_ERROR("V4L2: VIDIOC_STREAMOFF failed: %s (%d).\n",
                     strerror(errno), errno);
            return ret;
        }

        break;
    default:
        /* Nothing to do. */
        break;
    }

    return 0;
}

static int
v4l2_open(struct v4l2_device **v4l2, char *devname, struct v4l2_format *s_fmt)
{
    struct v4l2_device *dev;
    struct v4l2_capability cap;
    int fd;
    int ret = -EINVAL;

    fd = open(devname, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1)
    {
        LOG_ERROR("V4L2: device open failed: %s (%d).\n",
                 strerror(errno), errno);
        return ret;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0)
    {
        LOG_ERROR("V4L2: VIDIOC_QUERYCAP failed: %s (%d).\n",
                 strerror(errno), errno);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOG_ERROR("V4L2: %s is no video capture device\n", devname);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        LOG_ERROR("V4L2: %s does not support streaming i/o\n",
                 devname);
        goto err;
    }

    dev = calloc(1, sizeof * dev);
    if (dev == NULL)
    {
        ret = -ENOMEM;
        goto err;
    }

    LOG_DEBUG("V4L2 device is %s on bus %s\n", cap.card, cap.bus_info);

    dev->v4l2_fd = fd;

    /* Get the default image format supported. */
    ret = v4l2_get_format(dev);
    if (ret < 0)
        goto err_free;

    /*
     * Set the desired image format.
     * Note: VIDIOC_S_FMT may change width and height.
     */
    ret = v4l2_set_format(dev, s_fmt);
    if (ret < 0)
        goto err_free;

    /* Get the changed image format. */
    ret = v4l2_get_format(dev);
    if (ret < 0)
        goto err_free;

    LOG_DEBUG("v4l2 open succeeded, file descriptor = %d\n", fd);

    *v4l2 = dev;

    return 0;

err_free:
    free(dev);
err:
    close(fd);

    return ret;
}

static void
v4l2_close(struct v4l2_device *dev)
{
    close(dev->v4l2_fd);
    free(dev);
}

/* ---------------------------------------------------------------------------
 * UVC generic stuff
 */

static int
uvc_video_set_format(struct uvc_device *dev)
{
    struct v4l2_format fmt;
    int ret;

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (dev->fcc == V4L2_PIX_FMT_MJPEG)
        fmt.fmt.pix.sizeimage = dev->width * dev->height * 2/*1.5*/;
    if ((dev->fcc == V4L2_PIX_FMT_H264) || (dev->fcc == V4L2_PIX_FMT_H265))
        fmt.fmt.pix.sizeimage = dev->width * dev->height * 2;

    ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0)
    {
        LOG_ERROR("UVC: Unable to set format %s (%d).\n",
                  strerror(errno), errno);
        return ret;
    }

    LOG_INFO("UVC: Setting format to: %c%c%c%c %ux%u\n",
             pixfmtstr(dev->fcc), dev->width, dev->height);

    return 0;
}

int uvc_video_stream(struct uvc_device *dev, int enable)
{
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int ret;

    if (!enable)
    {
        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0)
        {
            LOG_ERROR("UVC: VIDIOC_STREAMOFF failed: %s (%d).\n",
                      strerror(errno), errno);
            return ret;
        }

        LOG_DEBUG("UVC: Stopping video stream.\n");

        return 0;
    }

    ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
    if (ret < 0)
    {
        LOG_ERROR("UVC: Unable to start streaming %s (%d).\n",
                  strerror(errno), errno);
        return ret;
    }

    LOG_DEBUG("UVC: Starting video stream.\n");

    dev->uvc_shutdown_requested = 0;

    return 0;
}

static int
uvc_uninit_device(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        for (i = 0; i < dev->nbufs; ++i)
        {
            ret = munmap(dev->mem[i].start, dev->mem[i].length);
            if (ret < 0)
            {
                LOG_ERROR("UVC: munmap failed\n");
                return ret;
            }
        }

        free(dev->mem);
        break;

    case IO_METHOD_USERPTR:

        if (dev->run_standalone)
        {
            for (i = 0; i < dev->nbufs; ++i)
                free(dev->dummy_buf[i].start);

            free(dev->dummy_buf);
        }
        break;
    case IO_METHOD_DMA_BUFF:

        break;
    default:
        break;
    }

    return 0;
}

static int
uvc_open(struct uvc_device **uvc, char *devname)
{
    struct uvc_device *dev;
    struct v4l2_capability cap;
    int fd;
    int ret = -EINVAL;

    fd = open(devname, O_RDWR | O_NONBLOCK);
    if (fd == -1)
    {
        LOG_ERROR("UVC: device open failed: %s (%d).\n",
                  strerror(errno), errno);
        return ret;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0)
    {
        LOG_ERROR("UVC: unable to query uvc device: %s (%d)\n",
                  strerror(errno), errno);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT))
    {
        LOG_ERROR("UVC: %s is no video output device\n", devname);
        goto err;
    }

    dev = calloc(1, sizeof * dev);
    if (dev == NULL)
    {
        ret = -ENOMEM;
        goto err;
    }

    LOG_DEBUG("uvc device is %s on bus %s\n", cap.card, cap.bus_info);
    LOG_DEBUG("uvc open succeeded, file descriptor = %d\n", fd);

    dev->uvc_fd = fd;
    dev->eptz_flag = XU_EPTZ_FLAG_DEFAULT_VAL;
    dev->xu_h265 = XU_H265_DEFAULT_VAL;
#ifdef CAMERA_CONTROL
    camera_pu_control_init(UVC_PU_BRIGHTNESS_CONTROL, PU_BRIGHTNESS_DEFAULT_VAL
                           , PU_BRIGHTNESS_MIN_VAL, PU_BRIGHTNESS_MAX_VAL);
    dev->brightness_val = camera_pu_control_get(UVC_PU_BRIGHTNESS_CONTROL, PU_BRIGHTNESS_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_CONTRAST_CONTROL, PU_CONTRAST_DEFAULT_VAL
                           , PU_CONTRAST_MIN_VAL, PU_CONTRAST_MAX_VAL);
    dev->contrast_val = camera_pu_control_get(UVC_PU_CONTRAST_CONTROL, PU_CONTRAST_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_HUE_CONTROL, PU_HUE_DEFAULT_VAL
                           , PU_HUE_MIN_VAL, PU_HUE_MAX_VAL);
    dev->hue_val = camera_pu_control_get(UVC_PU_HUE_CONTROL, PU_HUE_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_SATURATION_CONTROL, PU_SATURATION_DEFAULT_VAL
                           , PU_SATURATION_MIN_VAL, PU_SATURATION_MAX_VAL);
    dev->saturation_val = camera_pu_control_get(UVC_PU_SATURATION_CONTROL, PU_SATURATION_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_SHARPNESS_CONTROL, PU_SHARPNESS_DEFAULT_VAL
                           , PU_SHARPNESS_MIN_VAL, PU_SHARPNESS_MAX_VAL);
    dev->sharpness_val = camera_pu_control_get(UVC_PU_SHARPNESS_CONTROL, PU_SHARPNESS_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_GAMMA_CONTROL, PU_GAMMA_DEFAULT_VAL
                           , PU_GAMMA_MIN_VAL, PU_GAMMA_MAX_VAL);
    dev->gamma_val = camera_pu_control_get(UVC_PU_GAMMA_CONTROL, PU_GAMMA_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL, PU_WHITE_BALANCE_TEMPERATURE_DEFAULT_VAL
                           , PU_WHITE_BALANCE_TEMPERATURE_MIN_VAL, PU_WHITE_BALANCE_TEMPERATURE_MAX_VAL);
    dev->white_balance_temperature_val = camera_pu_control_get(UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL
                                         , PU_WHITE_BALANCE_TEMPERATURE_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, 1, 0, 1);
    dev->white_balance_temperature_auto_val = camera_pu_control_get(UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL
                                         , 1);

    camera_pu_control_init(UVC_PU_GAIN_CONTROL, PU_GAIN_DEFAULT_VAL
                           , PU_GAIN_MIN_VAL, PU_GAIN_MAX_VAL);
    dev->gain_val = camera_pu_control_get(UVC_PU_GAIN_CONTROL, PU_GAIN_DEFAULT_VAL);

    camera_pu_control_init(UVC_PU_HUE_AUTO_CONTROL, 1
                           , 0, 1);
    dev->hue_auto_val = camera_pu_control_get(UVC_PU_HUE_AUTO_CONTROL, 1);
    camera_pu_control_set(UVC_PU_POWER_LINE_FREQUENCY_CONTROL,1);// set default AntiFlickerMode 1:50hz,2:60hz
#else
    dev->brightness_val = PU_BRIGHTNESS_DEFAULT_VAL;
    dev->contrast_val = PU_CONTRAST_DEFAULT_VAL;
    dev->hue_val = PU_HUE_DEFAULT_VAL;
    dev->saturation_val = PU_SATURATION_DEFAULT_VAL;
    dev->sharpness_val = PU_SHARPNESS_DEFAULT_VAL;
    dev->gamma_val = PU_GAMMA_DEFAULT_VAL;
    dev->white_balance_temperature_val = PU_WHITE_BALANCE_TEMPERATURE_DEFAULT_VAL;
    dev->white_balance_temperature_auto_val = 1;
    dev->gain_val = PU_GAIN_DEFAULT_VAL;
    dev->hue_auto_val = 1;
#endif
    dev->power_line_frequency_val = V4L2_CID_POWER_LINE_FREQUENCY_50HZ;

    //zoom
    dev->zoom_val = CT_ZOOM_ABSOLUTE_CONTROL_DEFAULT_VAL;
    //pan
    dev->pan_val = CT_PANTILT_ABSOLUTE_CONTROL_DEFAULT_VAL;
    //tilt
    dev->tilt_val = CT_PANTILT_ABSOLUTE_CONTROL_DEFAULT_VAL;
    //roll
    dev->roll_val = CT_ROLL_ABSOLUTE_CONTROL_DEFAULT_VAL;
    //exposure_time
    dev->exposure_time_val = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_DEFAULT_VAL;
    // ae mode
    dev->ae_mode_val = 0x02;//Auto Mode – auto Exposure Time, auto Iris

    char *ver = XU_CAMERA_VERSION_DEFAULT;
    strncpy(dev->ex_sn_data, ver, MAX_UVC_REQUEST_DATA_LENGTH);
    memset(dev->ex_tool_ctrl1, 0, sizeof(dev->ex_tool_ctrl1));

    *uvc = dev;
    return 0;

err:
    close(fd);
    return ret;
}

static void
uvc_close(struct uvc_device *dev)
{
    close(dev->uvc_fd);
#if (UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF)
    free(dev->vbuf_info);
#endif
    free(dev->imgdata);
    free(dev);
}

/* ---------------------------------------------------------------------------
 * UVC streaming related
 */
static void
uvc_video_set_write_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
    uvc_user_set_write_buffer(dev, buf, dev->video_id);
}

static void
uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
#if 0
    unsigned int bpl;
    unsigned int i;

    switch (dev->fcc)
    {
    case V4L2_PIX_FMT_YUYV:
        /* Fill the buffer with video data. */
        bpl = dev->width * 2;
        for (i = 0; i < dev->height; ++i)
            memset(dev->mem[buf->index].start + i * bpl,
                   dev->color++, bpl);

        buf->bytesused = bpl * dev->height;
        break;

    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
        memcpy(dev->mem[buf->index].start, dev->imgdata, dev->imgsize);
        buf->bytesused = dev->imgsize;
        break;

    }
#else
    uvc_user_fill_buffer(dev, buf, dev->video_id);
#endif
}

static int
uvc_video_process(struct uvc_device *dev)
{
    struct v4l2_buffer vbuf;
    unsigned int i;
    int ret;
    bool get_ok = false;

    /*
     * Return immediately if UVC video output device has not started
     * streaming yet.
     */
    if (!dev->is_streaming)
    {
        usleep(10000);
        return 0;
    }
#ifdef ENABLE_BUFFER_TIME_DEBUG
    struct timeval process_time;
    gettimeofday(&process_time, NULL);
    LOG_ERROR("UVC V4L2 READY TO WRITE BUFFER:%d.%d (s)",process_time.tv_sec,process_time.tv_usec);
#endif
    /* Prepare a v4l2 buffer to be dequeued from UVC domain. */
    CLEAR(dev->ubuf);

    dev->ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        dev->ubuf.memory = V4L2_MEMORY_MMAP;
        break;

    case IO_METHOD_USERPTR:
        dev->ubuf.memory = V4L2_MEMORY_USERPTR;
        break;
    case IO_METHOD_DMA_BUFF:
    default:
        dev->ubuf.memory = V4L2_MEMORY_DMABUF;
        break;
    }

    if (dev->run_standalone)
    {
#if UVC_SEND_BUF_WHEN_ENC_READY
       if (dev->get_buf_count < UVC_BUFFER_NUM)
       {
           if(uvc_user_fill_buffer_init(dev))
           {
               dev->get_buf_count += 1;
               LOG_INFO("%d: wait enc buf ok:get_buf_count=%d\n", dev->video_id, dev->get_buf_count);
           }
       }
       else
#endif
       {
REDQBUF:
            /* UVC stanalone setup. */
            do {
                ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &dev->ubuf);
                if (ret == 0) {
                    dev->dqbuf_count++;
                    uvc_video_set_write_buffer(dev, &dev->ubuf);
#ifdef ENABLE_BUFFER_DEBUG
                    LOG_INFO("dev->dqbuf_count:%lld, %d: DeQueued buffer at UVC side = %d\n",
                              dev->dqbuf_count, dev->video_id, dev->ubuf.index);
#endif
                }
                else if (ret < 0) {
                    if (errno == EAGAIN)
                         break;
                    LOG_ERROR("%d: UVC: Unable to DeQueued buffer: %s (%d).\n",
                               dev->video_id, strerror(errno), errno);
                    return ret;
                } else {
                    LOG_ERROR("%d: UVC: Unable to DeQueued buffer: %s (%d) ret(%d).\n",
                               dev->video_id, strerror(errno), errno, ret);
                    //return ret;
                }
            } while (ret == 0);

            do {
                get_ok = false;
                uvc_video_fill_buffer(dev, &dev->ubuf);
                if (!dev->ubuf.bytesused && !dev->ubuf.m.fd)
                {
                    struct uvc_buffer *uvc_buf;
                    dev->abandon_count ++;
                    LOG_DEBUG("%d: UVC: Unable to queue buffer length is 0 ,driver will drop it.%d\n",
                             dev->video_id, dev->abandon_count);
                    uvc_buf = uvc_buffer_write_get(dev->video_id);
                    if (!uvc_buf)
                    {
                        LOG_ERROR("uvc_buffer_write_get failed\n");
                        goto REDQBUF;
                        //return 0;
                    }
                    for (int i = 0; i < dev->nbufs; i++)
                    {
                        if (uvc_buf == dev->vbuf_info[i].uvc_buf)
                        {
                            dev->ubuf.index = dev->vbuf_info[i].index;
                            dev->ubuf.sequence = uvc_buf->seq;
                            dev->ubuf.m.fd = dev->vbuf_info[i].fd;
                            get_ok = true;
                            break;
                        }
                    }
                    if (get_ok == false) {
                        LOG_ERROR("get_ok failed\n");
                        return 0; //if return 0 make host err,here should change to wait the enc buf.
                    }
                    //return 0;
                }
                ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &dev->ubuf);
                if (ret < 0)
                {
                    LOG_ERROR("%d: UVC: Unable to queue buffer: %s (%d).\n",
                              dev->video_id, strerror(errno), errno);
                    return ret;
                }
                dev->qbuf_count++;
#ifdef ENABLE_BUFFER_DEBUG
                LOG_INFO("dev->qbuf_count:%lld,%d: ReQueueing buffer at UVC side = %d size = %d\n",
                          dev->qbuf_count, dev->video_id, dev->ubuf.index, dev->ubuf.bytesused);
#endif
#ifdef ENABLE_BUFFER_TIME_DEBUG
                struct timeval buffer_time;
                gettimeofday(&buffer_time, NULL);
                LOG_ERROR("UVC V4L2 BUFFER TIME END:%d.%d (s)",buffer_time.tv_sec,buffer_time.tv_usec);
#endif
            } while (uvc_buffer_read_enable(dev->video_id));
        }
    }
    else
    {
        /* UVC - V4L2 integrated path. */

        /*
         * Return immediately if V4L2 video capture device has not
         * started streaming yet or if QBUF was not called even once on
         * the UVC side.
         */
        if (!dev->vdev->is_streaming || !dev->first_buffer_queued)
            return 0;

        /*
         * Do not dequeue buffers from UVC side until there are atleast
         * 2 buffers available at UVC domain.
         */
        if (!dev->uvc_shutdown_requested)
            if ((dev->dqbuf_count + 1) >= dev->qbuf_count)
                return 0;

        /* Dequeue the spent buffer from UVC domain */
        ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &dev->ubuf);
        if (ret < 0)
            return ret;

        if (dev->io == IO_METHOD_USERPTR)
            for (i = 0; i < dev->nbufs; ++i)
                if (dev->ubuf.m.userptr ==
                        (unsigned long) dev->vdev->mem[i].start
                        && dev->ubuf.length == dev->vdev->mem[i].length)
                    break;

        dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
        LOG_INFO("DeQueued buffer at UVC side=%d\n", dev->ubuf.index);
#endif

        /*
         * If the dequeued buffer was marked with state ERROR by the
         * underlying UVC driver gadget, do not queue the same to V4l2
         * and wait for a STREAMOFF event on UVC side corresponding to
         * set_alt(0). So, now all buffers pending at UVC end will be
         * dequeued one-by-one and we will enter a state where we once
         * again wait for a set_alt(1) command from the USB host side.
         */
        if (dev->ubuf.flags & V4L2_BUF_FLAG_ERROR)
        {
            dev->uvc_shutdown_requested = 1;
            LOG_INFO("UVC: Possible USB shutdown requested from "
                     "Host, seen during VIDIOC_DQBUF\n");
            return 0;
        }

        /* Queue the buffer to V4L2 domain */
        CLEAR(vbuf);

        vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index = dev->ubuf.index;

        ret = ioctl(dev->vdev->v4l2_fd, VIDIOC_QBUF, &vbuf);
        if (ret < 0)
        {
            LOG_ERROR("V4L2: Unable to queue buffer: %s (%d).\n",
                      strerror(errno), errno);
            return ret;
        }

        dev->vdev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
        LOG_INFO("ReQueueing buffer at V4L2 side = %d\n", vbuf.index);
#endif
    }

    return 0;
}

static int
uvc_video_qbuf_mmap(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i)
    {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0)
        {
            LOG_ERROR("UVC: VIDIOC_QBUF failed : %s (%d).\n",
                      strerror(errno), errno);
            return ret;
        }

        dev->qbuf_count++;
    }

    return 0;
}

static int
uvc_video_qbuf_userptr(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    /* UVC standalone setup. */
    if (dev->run_standalone)
    {
        for (i = 0; i < dev->nbufs; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.m.userptr = (unsigned long)dev->dummy_buf[i].start;
            buf.length = dev->dummy_buf[i].length;
            buf.index = i;

            ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf);
            if (ret < 0)
            {
                LOG_ERROR("UVC: VIDIOC_QBUF failed : %s (%d).\n",
                          strerror(errno), errno);
                return ret;
            }

            dev->qbuf_count++;
        }
    }

    return 0;
}
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
static int
uvc_video_qbuf_dmabuff(struct uvc_device *dev)
{
    LOG_DEBUG("uvc_video_qbuf_dmabuff enter\n");
    unsigned int i;
    int ret;
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = dev->nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    req.memory = V4L2_MEMORY_DMABUF;
    dev->vbuf_info = calloc(dev->nbufs, sizeof(struct v4l2_buffer_info));
    /* UVC standalone setup. */
    if (dev->run_standalone)
    {
        for (i = 0; i < dev->nbufs; ++i)
        {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = req.type;
            buf.index = i;
            buf.memory = req.memory;

            // printf("uvc_enc.video_id=%d dev->video_id=%d\n", uvc_enc.video_id, dev->video_id);
            struct uvc_buffer *uvc_buf = uvc_buffer_write_get(dev->video_id);
            buf.m.fd = uvc_buf->fd;
            buf.length = uvc_buf->total_size;
            dev->vbuf_info[i].fd = buf.m.fd;
            dev->vbuf_info[i].uvc_buf = uvc_buf;

            if (ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf) < 0)
            {
                LOG_ERROR("%s ioctl(VIDIOC_QBUF): %m,buf.m.fd=%d,buf.length=%d\n", dev, buf.m.fd,  buf.length);
                return -1;
            }
            // uvc_buffer_all_set(dev->video_id, uvc_buf);

        }
        //dev->nbufs = rb.count;
    }
    LOG_DEBUG("V4L2: %u buffers allocated.\n", dev->nbufs);

    return 0;
}
#endif
int uvc_video_qbuf(struct uvc_device *dev)
{
    int ret = 0;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        ret = uvc_video_qbuf_mmap(dev);
        break;

    case IO_METHOD_USERPTR:
        ret = uvc_video_qbuf_userptr(dev);
        break;
    case IO_METHOD_DMA_BUFF:
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
        ret = uvc_video_qbuf_dmabuff(dev);
#endif
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

static int
uvc_video_reqbufs_mmap(struct uvc_device *dev, int nbufs)
{
    struct v4l2_requestbuffers rb;
    unsigned int i;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0)
    {
        if (ret == -EINVAL)
            LOG_ERROR("UVC: does not support memory mapping\n");
        else
            LOG_ERROR("UVC: Unable to allocate buffers: %s (%d).\n",
                     strerror(errno), errno);
        goto err;
    }

    if (!rb.count)
        return 0;

    if (rb.count < 2)
    {
        LOG_ERROR("UVC: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = calloc(rb.count, sizeof dev->mem[0]);
    if (!dev->mem)
    {
        LOG_ERROR("UVC: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < rb.count; ++i)
    {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;

        ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0)
        {
            LOG_ERROR("UVC: VIDIOC_QUERYBUF failed for buf %d: "
                      "%s (%d).\n", i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].start = mmap(NULL /* start anywhere */,
                                 dev->mem[i].buf.length,
                                 PROT_READ | PROT_WRITE /* required */,
                                 MAP_SHARED /* recommended */,
                                 dev->uvc_fd, dev->mem[i].buf.m.offset);

        if (MAP_FAILED == dev->mem[i].start)
        {
            LOG_ERROR("UVC: Unable to map buffer %u: %s (%d).\n", i,
                      strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }

        dev->mem[i].length = dev->mem[i].buf.length;
        LOG_DEBUG("UVC: Buffer %u mapped at address %p.\n", i,
                 dev->mem[i].start);
    }

    dev->nbufs = rb.count;
    LOG_DEBUG("UVC: %u buffers allocated.\n", rb.count);

    return 0;

err_free:
    free(dev->mem);
err:
    return ret;
}

static int
uvc_video_reqbufs_dmabuff(struct uvc_device *dev, int nbufs)
{
    struct v4l2_requestbuffers rb;
    unsigned int i, j, bpl = 0, payload_size;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_DMABUF;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0)
    {
        if (ret == -EINVAL)
            LOG_ERROR("UVC: does not support dma buff i/o\n");
        else
            LOG_ERROR("UVC: VIDIOC_REQBUFS error %s (%d).\n",
                      strerror(errno), errno);
        goto err;
    }

    if (!rb.count)
        return 0;
err:
    return ret;

}

static int
uvc_video_reqbufs_userptr(struct uvc_device *dev, int nbufs)
{
    struct v4l2_requestbuffers rb;
    unsigned int i, j, bpl = 0, payload_size;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0)
    {
        if (ret == -EINVAL)
            LOG_ERROR("UVC: does not support user pointer i/o\n");
        else
            LOG_ERROR("UVC: VIDIOC_REQBUFS error %s (%d).\n",
                      strerror(errno), errno);
        goto err;
    }

    if (!rb.count)
        return 0;

    dev->nbufs = rb.count;
    LOG_DEBUG("UVC: %u buffers allocated.\n", rb.count);

    if (dev->run_standalone)
    {
        /* Allocate buffers to hold dummy data pattern. */
        dev->dummy_buf = calloc(rb.count, sizeof dev->dummy_buf[0]);
        if (!dev->dummy_buf)
        {
            LOG_ERROR("UVC: Out of memory\n");
            ret = -ENOMEM;
            goto err;
        }

        switch (dev->fcc)
        {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
            bpl = dev->width * 2;
            payload_size = dev->width * dev->height * 2;
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            payload_size = dev->imgsize;
            break;
        default:
            return -1;
        }

        for (i = 0; i < rb.count; ++i)
        {
            dev->dummy_buf[i].length = payload_size;
            dev->dummy_buf[i].start = malloc(payload_size);
            if (!dev->dummy_buf[i].start)
            {
                LOG_ERROR("UVC: Out of memory\n");
                ret = -ENOMEM;
                goto err;
            }

            if (V4L2_PIX_FMT_YUYV == dev->fcc)
                for (j = 0; j < dev->height; ++j)
                    memset(dev->dummy_buf[i].start + j * bpl,
                           dev->color++, bpl);

            if (V4L2_PIX_FMT_MJPEG == dev->fcc)
                memcpy(dev->dummy_buf[i].start, dev->imgdata,
                       dev->imgsize);
        }
    }

    return 0;

err:
    return ret;

}

int uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
    int ret = 0;

    switch (dev->io)
    {
    case IO_METHOD_MMAP:
        ret = uvc_video_reqbufs_mmap(dev, nbufs);
        break;

    case IO_METHOD_USERPTR:
        ret = uvc_video_reqbufs_userptr(dev, nbufs);
        break;
    case IO_METHOD_DMA_BUFF:
        ret = uvc_video_reqbufs_dmabuff(dev, nbufs);
        break;

    default:
        ret = -EINVAL;
        break;
    }

    return ret;
}

/*
 * This function is called in response to either:
 *  - A SET_ALT(interface 1, alt setting 1) command from USB host,
 *    if the UVC gadget supports an ISOCHRONOUS video streaming endpoint
 *    or,
 *
 *  - A UVC_VS_COMMIT_CONTROL command from USB host, if the UVC gadget
 *    supports a BULK type video streaming endpoint.
 */
static int
uvc_handle_streamon_event(struct uvc_device *dev)
{
    int ret;
#if !UVC_SEND_BUF_WHEN_ENC_READY
    ret = uvc_video_reqbufs(dev, dev->nbufs);
    if (ret < 0)
        goto err;
#endif
    if (!dev->run_standalone)
    {
        /* UVC - V4L2 integrated path. */
        if (IO_METHOD_USERPTR == dev->vdev->io)
        {
            /*
             * Ensure that the V4L2 video capture device has already
             * some buffers queued.
             */
            ret = v4l2_reqbufs(dev->vdev, dev->vdev->nbufs);
            if (ret < 0)
                goto err;
        }
        ret = v4l2_qbuf(dev->vdev);
        if (ret < 0)
            goto err;


        /* Start V4L2 capturing now. */
        ret = v4l2_start_capturing(dev->vdev);
        if (ret < 0)
            goto err;

        dev->vdev->is_streaming = 1;
    }

    /* Common setup. */

#if !UVC_SEND_BUF_WHEN_ENC_READY
    /* Queue buffers to UVC domain and start streaming. */
    ret = uvc_video_qbuf(dev);
    if (ret < 0)
        goto err;
#endif
    if (dev->run_standalone)
    {
#if !UVC_SEND_BUF_WHEN_ENC_READY
        uvc_video_stream(dev, 1);
#endif
        dev->first_buffer_queued = 1;
        dev->is_streaming = 1;
    }

    uvc_control_init(dev->width, dev->height, dev->fcc, dev->xu_h265, dev->fps, dev->video_id);

    set_uvc_control_start(dev->video_id, dev->width, dev->height,
                          dev->fps, dev->fcc, dev->eptz_flag);
    return 0;

err:
    return ret;
}

/* ---------------------------------------------------------------------------
 * UVC Request processing
 */

static void
uvc_fill_streaming_control(struct uvc_device *dev,
                           struct uvc_streaming_control *ctrl,
                           int iformat, int iframe, unsigned int ival)
{
    const struct uvc_function_config_format *format;
    const struct uvc_function_config_frame *frame;
    unsigned int i;

    /*
     * Restrict the iformat, iframe and ival to valid values. Negative
     * values for iformat or iframe will result in the maximum valid value
     * being selected.
     */
    iformat = clamp((unsigned int)iformat, 1U,
                    dev->fc->streaming.num_formats);
    format = &dev->fc->streaming.formats[iformat-1];

    iframe = clamp((unsigned int)iframe, 1U, format->num_frames);
    frame = &format->frames[iframe-1];

    for (i = 0; i < frame->num_intervals; ++i) {
        if (ival <= frame->intervals[i]) {
            ival = frame->intervals[i];
            break;
        }
    }
    if (i == frame->num_intervals)
        ival = frame->intervals[frame->num_intervals-1];

    memset(ctrl, 0, sizeof * ctrl);

    ctrl->bmHint = 1;
    ctrl->bFormatIndex = iformat;
    ctrl->bFrameIndex = iframe;
    ctrl->dwFrameInterval = ival;

    dev->width = frame->width;
    dev->height = frame->height;
    dev->fcc = format->fcc;
    switch (format->fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
        break;
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        dev->imgsize = frame->width * frame->height * 2/*1.5*/;
        ctrl->dwMaxVideoFrameSize = dev->imgsize;
        break;
    }
    LOG_INFO("dev->fcc:%d,dev->width:%d,dev->height:%d,dev->imgsize:%d,ctrl->dwFrameInterval:%d\n",
              dev->fcc,dev->width, dev->height, dev->imgsize, ctrl->dwFrameInterval);

    /* TODO: the UVC maxpayload transfer size should be filled
     * by the driver.
     */
    if (!dev->bulk)
    {
        ctrl->dwMaxPayloadTransferSize = dev->fc->streaming.ep.wMaxPacketSize;/*get_uvc_streaming_maxpacket();/*(dev->maxpkt) *
                                         (dev->mult + 1) * (dev->burst + 1);*/
        LOG_INFO("+++++++++dwMaxPayloadTransferSize:%d\n",ctrl->dwMaxPayloadTransferSize);
    }
    else
    {
        ctrl->dwMaxPayloadTransferSize = ctrl->dwMaxVideoFrameSize;
    }

    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;
}

static void
uvc_events_process_standard(struct uvc_device *dev,
                            struct usb_ctrlrequest *ctrl,
                            struct uvc_request_data *resp)
{
    LOG_DEBUG("standard request\n");
    (void)dev;
    (void)ctrl;
    (void)resp;
}

static void
uvc_events_process_control(struct uvc_device *dev, uint8_t req,
                           uint8_t cs, uint8_t entity_id,
                           uint8_t len, struct uvc_request_data *resp)
{
    LOG_DEBUG("req = %d cs = %d entity_id =%d len = %d \n", req, cs, entity_id, len);
    dev->cs = cs;
    dev->entity_id = entity_id;

    switch (entity_id)
    {
    case UVC_CTRL_INTERFACE_ID:
        switch (cs)
        {
        case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            /* Send the request error code last prepared. */
            resp->data[0] = dev->request_error_code.data[0];
            resp->length = dev->request_error_code.length;
            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare an error code response.
             */
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
        }
        break;

    /* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
    case UVC_CTRL_CAMERA_TERMINAL_ID:
        switch (cs)
        {
        case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
            switch (req)
            {
                LOG_INFO("UVC_CT_ZOOM_ABSOLUTE_CONTROL:req=%d \n", req);
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = CT_ZOOM_ABSOLUTE_CONTROL_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = CT_ZOOM_ABSOLUTE_CONTROL_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->zoom_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = CT_ZOOM_ABSOLUTE_CONTROL_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = CT_ZOOM_ABSOLUTE_CONTROL_STEP_SIZE;//must be 1
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
            switch (req)
            {
                LOG_INFO("UVC_CT_PANTILT_ABSOLUTE_CONTROL:req=%d len:%d\n", req,len);
            case UVC_SET_CUR:
                //resp->data[0] = 0x0;
                //resp->data[4] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
               {
                int a = CT_PANTILT_ABSOLUTE_CONTROL_MIN_VAL;
                int b = CT_PANTILT_ABSOLUTE_CONTROL_MIN_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &a,4);
                memcpy(&resp->data[4], &b,4);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_MAX:
               {
                int c = CT_PANTILT_ABSOLUTE_CONTROL_MAX_VAL;
                int d = CT_PANTILT_ABSOLUTE_CONTROL_MAX_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &c,4);
                memcpy(&resp->data[4], &d,4);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_CUR:
               {
                resp->length = len;
                memset(resp->data, 0, sizeof(resp->data));
                memcpy(&resp->data[0], &dev->pan_val,4);
                memcpy(&resp->data[4], &dev->tilt_val,4);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_INFO:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x03;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
               {
                int e = CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE;
                int f = CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &e,4);
                memcpy(&resp->data[4], &f,4);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_CT_ROLL_ABSOLUTE_CONTROL:
            switch (req)
            {
                LOG_DEBUG("UVC_CT_ROLL_ABSOLUTE_CONTROL:req=%d len:%d\n", req,len);
            case UVC_SET_CUR:
                //resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
               {
                short min = CT_ROLL_ABSOLUTE_CONTROL_MIN_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &min,
                        resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1; 
                break;
               }
            case UVC_GET_MAX:
               {
                short max = CT_ROLL_ABSOLUTE_CONTROL_MAX_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len; 
                memcpy(&resp->data, &max,
                        resp->length);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_CUR:
                resp->length = len;
                memcpy(&resp->data[0], &dev->roll_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = CT_ROLL_ABSOLUTE_CONTROL_STEP_SIZE;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_CT_AE_MODE_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                /* Incase of auto exposure, attempts to
                 * programmatically set the auto-adjusted
                 * controls are ignored.
                 */
                resp->data[0] = 0x01;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_INFO:
                /*
                 * TODO: We support Set and Get requests, but
                 * don't support async updates on an video
                 * status (interrupt) endpoint as of
                 * now.
                 */
                resp->data[0] = 0x03;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_CUR:
                resp->length = 1;
                /* Auto Mode - auto Exposure Time, auto Iris. */
                memcpy(&resp->data[0], &dev->ae_mode_val,
                       resp->length);
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
            case UVC_GET_RES:
                /* Auto Mode - auto Exposure Time, auto Iris. */
                resp->data[0] = 0x02;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
            case UVC_CT_FOCUS_AUTO_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                /* Incase of auto exposure, attempts to
                 * programmatically set the auto-adjusted
                 * controls are ignored.
                 */
                resp->data[0] = 0x01;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_INFO:
                /*
                 * TODO: We support Set and Get requests, but
                 * don't support async updates on an video
                 * status (interrupt) endpoint as of
                 * now.
                 */
                resp->data[0] = 0x03;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;

            case UVC_GET_CUR:
            case UVC_GET_DEF:
            case UVC_GET_RES:
                resp->data[0] = 1;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
           case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
             {
                 /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
            }
            break;
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            LOG_DEBUG("++++++++UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL req:%d",req);
            switch (req)
            {
            case UVC_SET_CUR:
                //resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
               {
                int min_time = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_MIN_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &min_time,
                        resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_MAX:
               {
                int max_time = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_MAX_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &max_time,
                        resp->length);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_CUR:
                resp->length = len;
                memcpy(&resp->data[0], &dev->exposure_time_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_STEP_SIZE;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                {
                int def_time = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL_DEFAULT_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &def_time,
                        resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                }
            default:
                /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
            }
            break;
        case UVC_CT_IRIS_ABSOLUTE_CONTROL:
             LOG_DEBUG("++++++++UVC_CT_IRIS_ABSOLUTE_CONTROL req:%d",req);
            switch (req)
            {
            case UVC_SET_CUR:
                //resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
               {
                int min_time = CT_IRIS_ABSOLUTE_CONTROL_MIN_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &min_time,
                        resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_MAX:
               {
                int max_time = CT_IRIS_ABSOLUTE_CONTROL_MAX_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &max_time,
                        resp->length);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_CUR:
                resp->length = len;
                memcpy(&resp->data[0], &dev->iris_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = CT_IRIS_ABSOLUTE_CONTROL_STEP_SIZE;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                {
                int def_time = CT_IRIS_ABSOLUTE_CONTROL_DEFAULT_VAL;
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                memcpy(&resp->data, &def_time,
                        resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                }
            default:
                /*
                 * We don't support this control, so STALL the
                 * control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * value.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
            }
            break;

        default:
            LOG_DEBUG("++++++++Input Terminal usb default req ,cs:%d",cs);
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        }
        break;

    /* processing unit 'UVC_VC_PROCESSING_UNIT' */
    case UVC_CTRL_PROCESSING_UNIT_ID:
        switch (cs)
        {
        case UVC_PU_BRIGHTNESS_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                LOG_INFO("set brightness\n");
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_BRIGHTNESS_MIN_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_BRIGHTNESS_MAX_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->brightness_val,
                       resp->length);
                /*
                 * For every successfully handled control
                 * request set the request error code to no
                 * error
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                /*
                 * We support Set and Get requests and don't
                 * support async updates on an interrupt endpt
                 */
                resp->data[0] = 0x03;
                resp->length = 1;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_BRIGHTNESS_STEP_SIZE;
                resp->length = 2;
                /*
                 * For every successfully handled control
                 * request, set the request error code to no
                 * error.
                 */
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                /*
                 * We don't support this control, so STALL the
                 * default control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * code.
                 */
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_CONTRAST_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_CONTRAST_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_CONTRAST_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->contrast_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_CONTRAST_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_CONTRAST_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_HUE_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_HUE_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_HUE_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->hue_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_HUE_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_HUE_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_SATURATION_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_SATURATION_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_SATURATION_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->saturation_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_SATURATION_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_SATURATION_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_SHARPNESS_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_SHARPNESS_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_SHARPNESS_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->sharpness_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_SHARPNESS_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_SHARPNESS_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_GAMMA_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_GAMMA_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_GAMMA_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->gamma_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_GAMMA_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_GAMMA_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                {
                int wbt_min= PU_WHITE_BALANCE_TEMPERATURE_MIN_VAL;
                resp->length = 2;
                memcpy(&resp->data[0], &wbt_min,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                }
            case UVC_GET_MAX:
               {
                int wbt_max = PU_WHITE_BALANCE_TEMPERATURE_MAX_VAL;
                resp->length = 2;
                memcpy(&resp->data[0], &wbt_max,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
               }
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->white_balance_temperature_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                {
                int wbt_def = PU_WHITE_BALANCE_TEMPERATURE_DEFAULT_VAL;
                resp->length = 2;
                memcpy(&resp->data[0], &wbt_def,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                }
            case UVC_GET_RES:
                {
                int wbt_res = PU_WHITE_BALANCE_TEMPERATURE_STEP_SIZE;
                resp->length = 2;
                memcpy(&resp->data[0], &wbt_res,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                }
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 1;
                resp->data[0] = dev->white_balance_temperature_auto_val;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                break;
            case UVC_GET_DEF:
                resp->data[0] = 1;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_GAIN_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_GAIN_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_GAIN_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->gain_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_GAIN_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_GAIN_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_HUE_AUTO_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 1;
                resp->data[0] = dev->hue_auto_val;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = 1;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 1;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = 2;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 1;
                memcpy(&resp->data[0], &dev->power_line_frequency_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 3;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = 1;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->data[0] = PU_DIGITAL_MULTIPLIER_CONTROL_MIN_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->data[0] = PU_DIGITAL_MULTIPLIER_CONTROL_MAX_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                memcpy(&resp->data[0], &dev->zoom_val,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->data[0] = PU_DIGITAL_MULTIPLIER_CONTROL_DEFAULT_VAL;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->data[0] = PU_DIGITAL_MULTIPLIER_CONTROL_STEP_SIZE;
                resp->length = 2;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        default:
            LOG_DEBUG("++++++++Processing unit usb default req ,cs:%d",cs);
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        }

        break;

    /* Extension unit 'UVC_VC_Extension_Unit'. */
    case UVC_CTRL_XU_UNIT_ID:
        switch (cs)
        {
        case CMD_TOOLS_CTRL_1:
            switch (req)
            {
            case UVC_GET_LEN:
                resp->data[0] = 0x4;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = len < sizeof(dev->ex_tool_ctrl1) ? len : sizeof(dev->ex_tool_ctrl1);
                memcpy(resp->data, dev->ex_tool_ctrl1, resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
        case CMD_GET_CAMERA_VERSION:
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = len < sizeof(dev->ex_sn_data) ? len : sizeof(dev->ex_sn_data);
                memcpy(resp->data, dev->ex_sn_data, resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        case CMD_SET_CAMERA_IP:
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = len < sizeof(dev->ex_ip_data) ? len : sizeof(dev->ex_ip_data);
                memcpy(resp->data, dev->ex_ip_data, resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        case CMD_SET_EPTZ:
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = len < sizeof(dev->eptz_flag) ? len : sizeof(dev->eptz_flag);
                memcpy(&resp->data[0], &dev->eptz_flag,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        case CMD_SET_H265:
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = len < sizeof(dev->xu_h265) ? len : sizeof(dev->xu_h265);
                memcpy(&resp->data[0], &dev->xu_h265,
                       resp->length);
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = 4;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;

        default:
             LOG_DEBUG("+++++++++Extension unit usb default req ,cs:%d",cs);
            switch (req)
            {
            case UVC_GET_LEN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = sizeof(resp->data);
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_SET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                memset(resp->data, 0, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                memset(resp->data, 0xFF, sizeof(resp->data));
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0x0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 0;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                memset(resp->data, 0, sizeof(resp->data));
                resp->data[0] = 1;
                resp->length = len;
                dev->request_error_code.data[0] = 0x00;
                dev->request_error_code.length = 1;
                break;
            default:
                resp->length = -EL2HLT;
                dev->request_error_code.data[0] = 0x07;
                dev->request_error_code.length = 1;
                break;
            }
            break;
            /*
             * We don't support this control, so STALL the control
             * ep.
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;
             */
        }
        break;

    default:
        /*
         * If we were not supposed to handle this
         * 'cs', prepare a Request Error Code response.
         */
        dev->request_error_code.data[0] = 0x06;
        dev->request_error_code.length = 1;
        break;

    }
    if (resp->length == -EL2HLT)
    {
        LOG_DEBUG("unsupported: req=%02x, cs=%d, entity_id=%d, len=%d\n",
                  req, cs, entity_id, len);
        resp->length = 0;
    }
    LOG_DEBUG("control request (req %02x cs %02x)\n", req, cs);
}


static void
uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs,
                             struct uvc_request_data *resp)
{
    struct uvc_streaming_control *ctrl;

    LOG_DEBUG("streaming request (req %02x cs %02x)\n", req, cs);

    if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
        return;

    ctrl = (struct uvc_streaming_control *)&resp->data;
    resp->length = sizeof * ctrl;

    switch (req)
    {
    case UVC_SET_CUR:
        dev->control = cs;
        resp->length = 34;
        break;

    case UVC_GET_CUR:
        if (cs == UVC_VS_PROBE_CONTROL)
            memcpy(ctrl, &dev->probe, sizeof * ctrl);
        else
            memcpy(ctrl, &dev->commit, sizeof * ctrl);
#if 1
        LOG_DEBUG("bmHint: %u\n", ctrl->bmHint);
        LOG_DEBUG("bFormatIndex: %u\n", ctrl->bFormatIndex);
        LOG_DEBUG("bFrameIndex: %u\n", ctrl->bFrameIndex);
        LOG_DEBUG("dwFrameInterval: %u\n", ctrl->dwFrameInterval);
        LOG_DEBUG("wKeyFrameRate: %u\n", ctrl->wKeyFrameRate);
        LOG_DEBUG("wPFrameRate: %u\n", ctrl->wPFrameRate);
        LOG_DEBUG("wCompQuality: %u\n", ctrl->wCompQuality);
        LOG_DEBUG("wCompWindowSize: %u\n", ctrl->wCompWindowSize);
        LOG_DEBUG("wDelay: %u\n", ctrl->wDelay);
        LOG_DEBUG("dwMaxVideoFrameSize: %u\n", ctrl->dwMaxVideoFrameSize);
        LOG_DEBUG("dwMaxPayloadTransferSize: %u\n", ctrl->dwMaxPayloadTransferSize);
        LOG_DEBUG("dwClockFrequency: %u\n", ctrl->dwClockFrequency);
        LOG_DEBUG("bmFramingInfo: %u\n", ctrl->bmFramingInfo);
        LOG_DEBUG("bPreferedVersion: %u\n", ctrl->bPreferedVersion);
        LOG_DEBUG("bMinVersion: %u\n", ctrl->bMinVersion);
        LOG_DEBUG("bMaxVersion: %u\n", ctrl->bMaxVersion);
#endif
        break;

    case UVC_GET_MIN:
    case UVC_GET_MAX:
    case UVC_GET_DEF:
        if (req == UVC_GET_MAX)
            uvc_fill_streaming_control(dev, ctrl, -1, -1, UINT_MAX);
        else
            uvc_fill_streaming_control(dev, ctrl, 1, 1, 0);
        break;

    case UVC_GET_RES:
        CLEAR(ctrl);
        break;

    case UVC_GET_LEN:
        resp->data[0] = 0x00;
        resp->data[1] = 0x22;
        resp->length = 2;
        break;

    case UVC_GET_INFO:
        resp->data[0] = 0x03;
        resp->length = 1;
        break;
    }
}

static void
uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
                         struct uvc_request_data *resp)
{
    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return;
    unsigned int interface = ctrl->wIndex & 0xff;

    if (interface == dev->fc->control.intf.bInterfaceNumber)
    {
        uvc_events_process_control(dev, ctrl->bRequest,
                                   ctrl->wValue >> 8,
                                   ctrl->wIndex >> 8,
                                   ctrl->wLength, resp);
    }
    else if (interface == dev->fc->streaming.intf.bInterfaceNumber)
    {
        uvc_events_process_streaming(dev, ctrl->bRequest,
                                     ctrl->wValue >> 8, resp);
    }
}

static void
uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
                         struct uvc_request_data *resp)
{
    dev->control = 0;

#ifdef ENABLE_USB_REQUEST_DEBUG
    LOG_DEBUG("\nbRequestType %02x bRequest %02x wValue %04x wIndex %04x "
             "wLength %04x\n", ctrl->bRequestType, ctrl->bRequest,
             ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif

    switch (ctrl->bRequestType & USB_TYPE_MASK)
    {
    case USB_TYPE_STANDARD:
        uvc_events_process_standard(dev, ctrl, resp);
        break;

    case USB_TYPE_CLASS:
        uvc_events_process_class(dev, ctrl, resp);
        break;

    default:
        break;
    }
}

static int
uvc_events_process_control_data(struct uvc_device *dev,
                                uint8_t cs, uint8_t entity_id,
                                struct uvc_request_data *data)
{
    unsigned int *val = (unsigned int *)data->data;
    LOG_DEBUG(" data = %d, length = %d  , current_cs = %d\n", *val , data->length, dev->cs);
    switch (entity_id)
    {
    case 1:
        switch (cs)
        {
        case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
            if (sizeof(dev->zoom_val) >= data->length)
            {
                memcpy(&dev->zoom_val, data->data, data->length);
                LOG_INFO("set zoom :%d \n", dev->zoom_val);
#ifdef CAMERA_CONTROL
                camera_control_set_zoom(dev->zoom_val);
#endif
            }
            break;
        case UVC_CT_ROLL_ABSOLUTE_CONTROL:
            if (sizeof(dev->roll_val) >= data->length)
            {
                memcpy(&dev->roll_val, data->data, data->length);
                LOG_INFO("set roll :%d \n", dev->roll_val);
#ifdef CAMERA_CONTROL
                //camera_control_set_roll(dev->roll_val);
                camera_pu_control_set(UVC_PU_ROLL_CONTROL,dev->roll_val);
#endif
            }
            break;
        case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
            if (sizeof(dev->pan_val) >= (data->length/2)) {
              int old_pan_val = dev->pan_val;
              memcpy(&dev->pan_val, &data->data, data->length/2);
              if (old_pan_val != dev->pan_val) {
                LOG_INFO("set pan :%d \n", dev->pan_val/CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE);
#ifdef CAMERA_CONTROL
                camera_control_set_pan(dev->pan_val/CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE);
#endif
              }
            }
            if (sizeof(dev->tilt_val) >= (data->length/2)) {
               int old_tilt_val = dev->tilt_val;
               memcpy(&dev->tilt_val, &data->data[4], data->length/2);
               if (old_tilt_val != dev->tilt_val) {
                 LOG_INFO("set tilt :%d \n", dev->tilt_val/CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE);
#ifdef CAMERA_CONTROL
                 camera_control_set_tilt(dev->tilt_val/CT_PANTILT_ABSOLUTE_CONTROL_STEP_SIZE);
#endif
               }
            }
            break;
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            if (sizeof(dev->exposure_time_val) >= data->length)
            {
                memcpy(&dev->exposure_time_val, data->data, data->length);
                LOG_INFO("set exposure_time :%d \n", dev->exposure_time_val);
#ifdef CAMERA_CONTROL
                camera_pu_control_set(UVC_PU_EXPOSURE_TIME_CONTROL,dev->exposure_time_val);
#endif
            }
            break;
        case UVC_CT_IRIS_ABSOLUTE_CONTROL:
            if (sizeof(dev->iris_val) >= data->length)
            {
                memcpy(&dev->iris_val, data->data, data->length);
                LOG_INFO("set iris value :%d \n", dev->iris_val);
            }
            break;
        case UVC_CT_AE_MODE_CONTROL:
            if (sizeof(dev->ae_mode_val) >= data->length)
            {
                memcpy(&dev->ae_mode_val, data->data, data->length);
                LOG_INFO("set AE Mode :%d \n", dev->ae_mode_val);
                if (dev->ae_mode_val == 2) {
                   dev->gain_val = 1;//ae mode set auto,so gain mode shuld be auto too.
                }
#ifdef CAMERA_CONTROL
                camera_pu_control_set(UVC_PU_AE_MODE_CONTROL,dev->ae_mode_val);
#endif
            }
            break;

        default:
            break;
        }

        break;
    /* Processing unit 'UVC_VC_PROCESSING_UNIT'. */
    case 2:
        switch (cs)
        {
        case UVC_PU_BRIGHTNESS_CONTROL:
            if (sizeof(dev->brightness_val) >= data->length)
            {
                memcpy(&dev->brightness_val, data->data, data->length);
                //camera_pu_control_set(UVC_PU_BRIGHTNESS_CONTROL,*val);
            }
            if (!dev->run_standalone)
                /*
                 * Try to change the Brightness attribute on
                 * Video capture device. Note that this try may
                 * succeed or end up with some error on the
                 * video capture side. By default to keep tools
                 * like USBCV's UVC test suite happy, we are
                 * maintaining a local copy of the current
                 * brightness value in 'dev->brightness_val'
                 * variable and we return the same value to the
                 * Host on receiving a GET_CUR(BRIGHTNESS)
                 * control request.
                 *
                 * FIXME: Keeping in view the point discussed
                 * above, notice that we ignore the return value
                 * from the function call below. To be strictly
                 * compliant, we should return the same value
                 * accordingly.
                 */
                v4l2_set_ctrl(dev->vdev, dev->brightness_val,
                              V4L2_CID_BRIGHTNESS);

            break;
        case UVC_PU_CONTRAST_CONTROL:
            LOG_INFO("UVC_PU_CONTRAST_CONTROL receive\n");
            if (sizeof(dev->contrast_val) >= data->length)
            {
                memcpy(&dev->contrast_val, data->data, data->length);
                //video_record_set_time(dev->contrast_val);
                LOG_INFO("UVC_PU_CONTRAST_CONTROL: 0x%02x 0x%02x\n",
                         data->data[0], data->data[1]);
                //video_record_set_contrast(*val);
            }
            break;
        case UVC_PU_HUE_CONTROL:
            if (sizeof(dev->hue_val) >= data->length)
            {
                memcpy(&dev->hue_val, data->data, data->length);
                //video_record_set_hue(*val);
            }
            break;
        case UVC_PU_SATURATION_CONTROL:
            if (sizeof(dev->saturation_val) >= data->length)
            {
                memcpy(&dev->saturation_val, data->data, data->length);
                //video_record_set_saturation(*val);
            }
            break;
        case UVC_PU_SHARPNESS_CONTROL:
            if (sizeof(dev->sharpness_val) >= data->length)
                memcpy(&dev->sharpness_val, data->data, data->length);
            break;
        case UVC_PU_GAMMA_CONTROL:
            if (sizeof(dev->gamma_val) >= data->length)
                memcpy(&dev->gamma_val, data->data, data->length);
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            /* 0:auto, 1:Daylight 2:fluocrescence 3:cloudysky 4:tungsten */
           LOG_INFO("UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL: 0x%02x 0x%02x\n",
                         data->data[0], data->data[1]); 
            if (sizeof(dev->white_balance_temperature_val) >= data->length)
            {
                memcpy(&dev->white_balance_temperature_val, data->data, data->length);
            }
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            LOG_INFO("UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL: 0x%02x\n",
                         data->data[0]);
            if (sizeof(dev->white_balance_temperature_auto_val) >= data->length)
            {
                memcpy(&dev->white_balance_temperature_auto_val, data->data, data->length);
            }
            break;
        case UVC_PU_GAIN_CONTROL:
            if (sizeof(dev->gain_val) >= data->length){
                memcpy(&dev->gain_val, data->data, data->length);
                if(dev->ae_mode_val == 2) {
                  LOG_INFO("AE Mode is auto,so gain must be auto mode,force gain =1\n");
                  dev->gain_val=1;
                  *val = dev->gain_val;
                }
            }
            break;
        case UVC_PU_HUE_AUTO_CONTROL:
            if (sizeof(dev->hue_auto_val) >= data->length)
                memcpy(&dev->hue_auto_val, data->data, data->length);
            break;
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            if (sizeof(dev->power_line_frequency_val) >= data->length)
            {
                memcpy(&dev->power_line_frequency_val, data->data, data->length);
            }
            break;
        case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
            if (sizeof(dev->zoom_val) >= data->length)
            {
                memcpy(&dev->zoom_val, data->data, data->length);
#ifdef CAMERA_CONTROL
                camera_control_set_zoom(dev->zoom_val);
#endif
            }
            break;
        default:
            break;
        }
#ifdef CAMERA_CONTROL
        camera_pu_control_set(cs, *val);
#endif
        if (uvc_pu_control_cb)
            uvc_pu_control_cb(cs, *val);

        break;
    /* Extension unit 'UVC_VC_Extension_Unit'. */
    case 6:
        switch (cs)
        {
        case CMD_TOOLS_CTRL_1:
            if (sizeof(dev->ex_tool_ctrl1) >= data->length)
            {
                memset(dev->ex_tool_ctrl1, 0, sizeof(dev->ex_tool_ctrl1));
                memcpy(dev->ex_tool_ctrl1, data->data, data->length);
                LOG_INFO("Extension:CMD_TOOLS_CTRL_1 set cur data: 0x%02x 0x%02x 0x%02x 0x%02x\n",
                         data->data[0], data->data[1], data->data[2], data->data[3]);
                unsigned int command = 0;
                memcpy(&command, dev->ex_tool_ctrl1, sizeof(command));
                if ( command == 0xFFFFFFFF )
                {
                   LOG_INFO("recive host reboot loader cmd");
                   system("reboot loader &");
                }

            }
            break;
        case CMD_SET_EPTZ:
            if (sizeof(dev->eptz_flag) >= data->length)
            {
                memcpy(&dev->eptz_flag, data->data, data->length);
                LOG_INFO("Extension: CMD_SET_EPTZ set cur data: %d\n", dev->eptz_flag);
#ifdef CAMERA_CONTROL
                camera_control_set_eptz(dev->eptz_flag);
#endif
            }
            break;

        case CMD_GET_CAMERA_VERSION:
            if (sizeof(dev->ex_sn_data) >= data->length)
            {
                memset(dev->ex_sn_data, 0, sizeof(dev->ex_sn_data));
                memcpy(dev->ex_sn_data, data->data, data->length);
                LOG_INFO("Extension:CMD_GET_CAMERA_VERSION set cur data: 0x%02x 0x%02x 0x%02x\n",
                         data->data[0], data->data[1], data->data[2]);
            }
            break;

        case CMD_SET_CAMERA_IP:
            if (sizeof(dev->ex_ip_data) >= data->length)
            {
                memcpy(dev->ex_ip_data, data->data, data->length);
                LOG_INFO("CMD_SET_CAMERA_IP : %d.%d.%d.%d \n", dev->ex_ip_data[0], dev->ex_ip_data[1],
                         dev->ex_ip_data[2], dev->ex_ip_data[3]);
                update_camera_ip(dev);
            }
            break;

        case CMD_SET_H265:
            if (sizeof(dev->xu_h265) >= data->length)
            {
                memcpy(&dev->xu_h265, data->data, data->length);
                LOG_INFO("Extension: CMD_SET_H265 set cur data: %d\n", dev->xu_h265);
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
    LOG_DEBUG("Control Request data phase (cs %02x  data %d entity %02x)\n", cs, *val, entity_id);
    return 0;
}
#if USE_RK_AISERVER
void uvc_enc_format_to_ipc_enc_type(unsigned int fcc, struct CAMERA_INFO *camera_info)
{
    switch (fcc) {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
            camera_info->encode_type = UVC_IPC_ENC_YUV;
            break;
        case V4L2_PIX_FMT_MJPEG:
            if (camera_info->height > 1440)
                camera_info->encode_type = UVC_IPC_ENC_MJPEG_LOW_LATENCY;
            else
                camera_info->encode_type = UVC_IPC_ENC_MJPEG_NORMAL;
            break;
        case V4L2_PIX_FMT_H264:
            camera_info->encode_type = UVC_IPC_ENC_H264;
            break;
        case V4L2_PIX_FMT_H265:
            camera_info->encode_type = UVC_IPC_ENC_H265;
            break;
        default:
            LOG_ERROR("no such fcc = %d\n", fcc);
            break;
    }
}
#endif

static int
uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
    struct uvc_streaming_control *target;
    struct uvc_streaming_control *ctrl;
    struct v4l2_format fmt;
    int ret = 0;

    switch (dev->control)
    {
    case UVC_VS_PROBE_CONTROL:
        LOG_DEBUG("setting probe control, length = %d\n", data->length);
        target = &dev->probe;
        break;

    case UVC_VS_COMMIT_CONTROL:
        LOG_DEBUG("setting commit control, length = %d\n", data->length);
        target = &dev->commit;
        break;

    default:
        LOG_DEBUG("setting process control, length = %d\n", data->length);

        LOG_DEBUG("cs: %u, entity_id: %u\n", dev->cs, dev->entity_id);
        ret = uvc_events_process_control_data(dev,
                                              dev->cs,
                                              dev->entity_id, data);
        if (ret < 0)
            goto err;

        return 0;
    }

    ctrl = (struct uvc_streaming_control *)&data->data;
    if (dev->control == UVC_VS_PROBE_CONTROL)
    {
#if 1
        LOG_DEBUG("host probe want ++++vs config:\n");
        LOG_DEBUG("bmHint: %u\n", ctrl->bmHint);
        LOG_DEBUG("bFormatIndex: %u\n", ctrl->bFormatIndex);
        LOG_DEBUG("bFrameIndex: %u\n", ctrl->bFrameIndex);
        LOG_DEBUG("dwFrameInterval: %u\n", ctrl->dwFrameInterval);
        LOG_DEBUG("wKeyFrameRate: %u\n", ctrl->wKeyFrameRate);
        LOG_DEBUG("wPFrameRate: %u\n", ctrl->wPFrameRate);
        LOG_DEBUG("wCompQuality: %u\n", ctrl->wCompQuality);
        LOG_DEBUG("wCompWindowSize: %u\n", ctrl->wCompWindowSize);
        LOG_DEBUG("wDelay: %u\n", ctrl->wDelay);
        LOG_DEBUG("dwMaxVideoFrameSize: %u\n", ctrl->dwMaxVideoFrameSize);
        LOG_DEBUG("dwMaxPayloadTransferSize: %u\n", ctrl->dwMaxPayloadTransferSize);
        LOG_DEBUG("dwClockFrequency: %u\n", ctrl->dwClockFrequency);
        LOG_DEBUG("bmFramingInfo: %u\n", ctrl->bmFramingInfo);
        LOG_DEBUG("bPreferedVersion: %u\n", ctrl->bPreferedVersion);
        LOG_DEBUG("bMinVersion: %u\n", ctrl->bMinVersion);
        LOG_DEBUG("bMaxVersion: %u\n", ctrl->bMaxVersion);
#endif

    }
    uvc_fill_streaming_control(dev, target, ctrl->bFormatIndex,
                               ctrl->bFrameIndex, ctrl->dwFrameInterval);

    dev->fps = 10000000 / target->dwFrameInterval;

#if USE_RK_AISERVER
    int need_full_range = 1;
    char *full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range)
    {
        need_full_range = atoi(full_range);
        LOG_INFO("uvc full_range use env setting:%d \n", need_full_range);
    }
    struct CAMERA_INFO camera_info;
    camera_info.width = dev->width;
    camera_info.height = dev->height;
    camera_info.vir_width = dev->width;
    camera_info.vir_height = dev->height;
    camera_info.buf_size = dev->width * dev->height * 2;
    camera_info.range = need_full_range;
    camera_info.uvc_fps_set = dev->fps;
    uvc_enc_format_to_ipc_enc_type(dev->fcc, &camera_info);
    uvc_ipc_event(UVC_IPC_EVENT_CONFIG_CAMERA, (void *)&camera_info);
#endif

    if (dev->control == UVC_VS_COMMIT_CONTROL)
    {
#if USE_RK_AISERVER
        int needEPTZ = 0;
        char *enableEptz = getenv("ENABLE_EPTZ");
        LOG_DEBUG("enableEptz=%s", enableEptz);
        if (enableEptz)
        {
            LOG_INFO("%s :uvc eptz use evn setting \n", __FUNCTION__);
            needEPTZ = atoi(enableEptz);
        } else if (dev->eptz_flag) {
            LOG_INFO("uvc eptz use xu setting:%d\n", dev->eptz_flag);
            needEPTZ = 1;
        }
        int need_full_range = 1;
        char *full_range = getenv("ENABLE_FULL_RANGE");
        if (full_range)
        {
            need_full_range = atoi(full_range);
            LOG_INFO("uvc full_range use env setting:%d \n", need_full_range);
        }
#ifndef USE_ARM64
        if (dev->width == 2560 && dev->height == 1440) {
            int needBYPASS = 1;
            dev->need_bypass = 1;
            uvc_ipc_event(UVC_IPC_EVENT_ENABLE_BYPASS, (void *)&needBYPASS);
        } else if (dev->need_bypass){
            int needBYPASS = 0;
            dev->need_bypass = 0;
            uvc_ipc_event(UVC_IPC_EVENT_ENABLE_BYPASS, (void *)&needBYPASS);
        }
#endif
        if (needEPTZ)
            uvc_ipc_event(UVC_IPC_EVENT_ENABLE_ETPTZ, (void *)&needEPTZ);
        uvc_ipc_event(UVC_IPC_EVENT_START, NULL); //after the camera config
        //int val = 12; // for test
        //uvc_ipc_event(UVC_IPC_EVENT_SET_ZOOM, (void *)&val);                   // for test
#endif

        if (uvc_video_get_uvc_process(dev->video_id))
            return 0;
        /*
         * Try to set the default format at the V4L2 video capture
         * device as requested by the user.
         */
        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        fmt.fmt.pix.width = dev->width;
        fmt.fmt.pix.height = dev->height;
        fmt.fmt.pix.pixelformat = dev->fcc;

        switch (dev->fcc)
        {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
            fmt.fmt.pix.sizeimage = (fmt.fmt.pix.width * fmt.fmt.pix.height * 2);
            break;
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            fmt.fmt.pix.sizeimage = (fmt.fmt.pix.width * fmt.fmt.pix.height * 2/*1.5*/);//dev->imgsize;
            break;
        }

        uvc_set_user_resolution(fmt.fmt.pix.width, fmt.fmt.pix.height, dev->video_id);
        uvc_set_user_fcc(fmt.fmt.pix.pixelformat, dev->video_id);
        if (uvc_buffer_init(dev->video_id))
            goto err;

        /*
         * As per the new commit command received from the UVC host
         * change the current format selection at both UVC and V4L2
         * sides.
         */
        ret = uvc_video_set_format(dev);
        if (ret < 0)
            goto err;

        if (!dev->run_standalone)
        {
            /* UVC - V4L2 integrated path. */
            ret = v4l2_set_format(dev->vdev, &fmt);
            if (ret < 0)
                goto err;
        }

        if (dev->bulk)
        {
            ret = uvc_handle_streamon_event(dev);
            if (ret < 0)
                goto err;
        }
    }

    return 0;

err:
    return ret;
}

static void
uvc_events_process(struct uvc_device *dev)
{
    struct v4l2_event v4l2_event;
    struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int ret = 0;

    ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
    if (ret < 0)
    {
        LOG_ERROR("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
                  errno);
        return;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type)
    {
    case UVC_EVENT_CONNECT:
        return;

    case UVC_EVENT_DISCONNECT:
        dev->uvc_shutdown_requested = 1;
        LOG_DEBUG("UVC: Possible USB shutdown requested from "
                 "Host, seen via UVC_EVENT_DISCONNECT \n");
        return;

    case UVC_EVENT_SETUP:
        LOG_DEBUG("uvc_events_process:UVC_EVENT_SETUP \n");
        if(dev->suspend){
           dev->suspend = 0;
           set_uvc_control_suspend(0);
        }
        uvc_events_process_setup(dev, &uvc_event->req, &resp);
        break;

    case UVC_EVENT_DATA:
        LOG_DEBUG("uvc_events_process:UVC_EVENT_DATA \n");
        ret = uvc_events_process_data(dev, &uvc_event->data);
        if (ret < 0)
            break;

        return;

    case UVC_EVENT_STREAMON:
        LOG_INFO("uvc_events_process:UVC_EVENT_STREAMON dev->io=%d， dev->bulk:%d, video_id:%d\n", dev->io,dev->bulk, dev->video_id);
#ifdef ENABLE_BUFFER_TIME_DEBUG
    struct timeval buffer_time;
    gettimeofday(&buffer_time, NULL);
    LOG_ERROR("UVC STREAMON START TIME:%d.%d (us)",buffer_time.tv_sec,buffer_time.tv_usec);
#endif
        if (!dev->bulk)
            uvc_handle_streamon_event(dev);
        dev->abandon_count = 0;
        dev->get_buf_count = 0;
        dev->usb_state = USB_STATE_FIRST_GET_READY;

        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        dev->stream_on_pts = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us

#if UVC_DYNAMIC_DEBUG_FPS
        uvc_debug_info.stream_on_pts = dev->stream_on_pts;
        uvc_debug_info.first_frm = true;
        gettimeofday(&uvc_debug_info.enter_time, NULL);
#endif
        return;

    case UVC_EVENT_STREAMOFF:
        LOG_INFO("uvc_events_process:UVC_EVENT_STREAMOFF enter\n");
#if USE_RK_AISERVER
        uvc_ipc_event(UVC_IPC_EVENT_STOP, NULL);
        //sleep(1); //make sure rkispp deint
#endif
        /* Stop V4L2 streaming... */
        if (!dev->run_standalone && dev->vdev->is_streaming)
        {
            /* UVC - V4L2 integrated path. */
            v4l2_stop_capturing(dev->vdev);
            v4l2_uninit_device(dev->vdev);
            v4l2_reqbufs(dev->vdev, 0);
            dev->vdev->is_streaming = 0;
        }

        set_uvc_control_stop();
        /* ... and now UVC streaming.. */
        if (dev->is_streaming)
        {
            uvc_video_stream(dev, 0);
            uvc_uninit_device(dev);
            uvc_video_reqbufs(dev, 0);
            dev->is_streaming = 0;
            dev->first_buffer_queued = 0;
        }
        //join mpp thread
        uvc_control_exit();
        uvc_buffer_deinit(dev->video_id);

        LOG_INFO("UVC_EVENT_STREAMOFF exit\n");
        return;
    case UVC_EVENT_RESUME:
        LOG_INFO("UVC_EVENT_RESUME:\n");
        if(dev->suspend){
           dev->suspend = 0;
           set_uvc_control_suspend(0);
        }
        return;
    case UVC_EVENT_SUSPEND:
        LOG_INFO("UVC_EVENT_SUSPEND\n");
        set_uvc_control_suspend(1);
        dev->suspend = 1;
        return;
    }

    ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0)
    {
        LOG_ERROR("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
                  errno);
        return;
    }
}

static void
uvc_events_init(struct uvc_device *dev)
{
    struct v4l2_event_subscription sub;
    unsigned int payload_size;

    switch (dev->fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        payload_size = dev->width * dev->height * 2;
        break;
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        payload_size = dev->imgsize;
        break;
    default:
        return;
    }

    uvc_fill_streaming_control(dev, &dev->probe, 1, 1, 0);
    uvc_fill_streaming_control(dev, &dev->commit, 1, 1, 0);

    if (dev->bulk)
    {
        /* FIXME Crude hack, must be negotiated with the driver. */
        dev->probe.dwMaxPayloadTransferSize =
            dev->commit.dwMaxPayloadTransferSize = payload_size;
    }

    memset(&sub, 0, sizeof sub);
    sub.type = UVC_EVENT_SETUP;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_RESUME;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_SUSPEND;
    ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

/* ---------------------------------------------------------------------------
 * main
 */

static void
image_load(struct uvc_device *dev, const char *img)
{
    int fd = -1;

    if (img == NULL)
        return;

    fd = open(img, O_RDONLY);
    if (fd == -1)
    {
        LOG_ERROR("Unable to open MJPEG image '%s'\n", img);
        return;
    }

    dev->imgsize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    dev->imgdata = malloc(dev->imgsize);
    if (dev->imgdata == NULL)
    {
        LOG_ERROR("Unable to allocate memory for MJPEG image\n");
        dev->imgsize = 0;
        return;
    }

    read(fd, dev->imgdata, dev->imgsize);
    close(fd);
}

/*
static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [options]\n", argv0);
    fprintf(stderr, "Available options are\n");
    fprintf(stderr, " -b        Use bulk mode\n");
    fprintf(stderr, " -d        Do not use any real V4L2 capture device\n");
    fprintf(stderr, " -f <format>    Select frame format\n\t"
            "0 = V4L2_PIX_FMT_YUYV\n\t"
            "1 = V4L2_PIX_FMT_MJPEG\n\t"
            "2 = V4L2_PIX_FMT_H264\n");
    fprintf(stderr, " -h        Print this help screen and exit\n");
    fprintf(stderr, " -i image  MJPEG image\n");
    fprintf(stderr, " -m        Streaming mult for ISOC (b/w 0 and 2)\n");
    fprintf(stderr, " -n        Number of Video buffers (b/w 2 and 32)\n");
    fprintf(stderr, " -o <IO method> Select UVC IO method:\n\t"
            "0 = MMAP\n\t"
            "1 = USER_PTR\n");
    fprintf(stderr, " -r <resolution> Select frame resolution:\n\t"
            "0 = 360p, VGA (640x360)\n\t"
            "1 = 720p, WXGA (1280x720)\n");
    fprintf(stderr, " -s <speed>    Select USB bus speed (b/w 0 and 2)\n\t"
            "0 = Full Speed (FS)\n\t"
            "1 = High Speed (HS)\n\t"
            "2 = Super Speed (SS)\n");
    fprintf(stderr, " -t        Streaming burst (b/w 0 and 15)\n");
    fprintf(stderr, " -u device UVC Video Output device\n");
    fprintf(stderr, " -v device V4L2 Video Capture device\n");

}
*/

extern int app_quit;

int
uvc_gadget_main(struct uvc_function_config *fc)
{
    struct uvc_device *udev = NULL;
    struct v4l2_device *vdev;
    struct timeval tv;
    struct v4l2_format fmt;
    char uvc_devname[32] = {0};
    char *v4l2_devname = "/dev/video0";
    char *mjpeg_image = NULL;
    fd_set fdsv, fdsu;
    int ret, nfds;
    int bulk_mode = 0;
    int dummy_data_gen_mode = 1;
    /* Frame format/resolution related params. */
    int default_format = 1;
    int default_resolution = 1;
    int nbufs = UVC_BUFFER_NUM;
    /* USB speed related params */
    int mult = 0;
    int burst = 0;
    enum usb_device_speed speed = USB_SPEED_HIGH;  /* High-Speed  1109 is usb 2.0*/
#if (UVC_IO_METHOD == UVC_IO_METHOD_MMAP)
    enum io_method uvc_io_method = IO_METHOD_MMAP;
#elif (UVC_IO_METHOD == UVC_IO_METHOD_USERPTR)
    enum io_method uvc_io_method = IO_METHOD_USERPTR;
#else
    enum io_method uvc_io_method = IO_METHOD_DMA_BUFF;
#endif
    int id = fc->video;
    snprintf(uvc_devname, sizeof(uvc_devname), "/dev/video%d", id);
    /************************************************************************************
     * int opt;
    while ((opt = getopt(argc, argv, "bdf:hi:m:n:o:r:s:t:u:v:")) != -1) {
      switch (opt) {
      case 'b':
          bulk_mode = 1;
          break;

      case 'd':
          dummy_data_gen_mode = 1;
          break;

      case 'f':
          if (atoi(optarg) < 0 && atoi(optarg) > 1) {
              usage(argv[0]);
              return 1;
          }

          default_format = atoi(optarg);
          break;

      case 'h':
          usage(argv[0]);
          return 1;

      case 'i':
          mjpeg_image = optarg;
          break;

      case 'm':
          if (atoi(optarg) < 0 && atoi(optarg) > 2) {
              usage(argv[0]);
              return 1;
          }

          mult = atoi(optarg);
          LOG_INFO("Requested Mult value = %d\n", mult);
          break;

      case 'n':
          if (atoi(optarg) < 2 && atoi(optarg) > 32) {
              usage(argv[0]);
              return 1;
          }

          nbufs = atoi(optarg);
          LOG_INFO("Number of buffers requested = %d\n", nbufs);
          break;

      case 'o':
          if (atoi(optarg) < 0 && atoi(optarg) > 1) {
              usage(argv[0]);
              return 1;
          }

          uvc_io_method = atoi(optarg);
          LOG_INFO("UVC: IO method requested is %s\n",
                  (uvc_io_method == IO_METHOD_MMAP) ?
                  "MMAP": "USER_PTR");
          break;

      case 'r':
          if (atoi(optarg) < 0 && atoi(optarg) > 1) {
              usage(argv[0]);
              return 1;
          }

          default_resolution = atoi(optarg);
          break;

      case 's':
          if (atoi(optarg) < 0 && atoi(optarg) > 2) {
              usage(argv[0]);
              return 1;
          }

          speed = atoi(optarg);
          break;

      case 't':
          if (atoi(optarg) < 0 && atoi(optarg) > 15) {
              usage(argv[0]);
              return 1;
          }

          burst = atoi(optarg);
          LOG_INFO("Requested Burst value = %d\n", burst);
          break;

      case 'u':
          uvc_devname = optarg;
          break;

      case 'v':
          v4l2_devname = optarg;
          break;

      default:
          LOG_INFO("Invalid option '-%c'\n", opt);
          usage(argv[0]);
          return 1;
      }
    }
    ************************************************************************************/

    if (!dummy_data_gen_mode && !mjpeg_image)
    {
        /*
         * Try to set the default format at the V4L2 video capture
         * device as requested by the user.
         */
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = (default_resolution == 0) ? 640 : 1280;
        fmt.fmt.pix.height = (default_resolution == 0) ? 480 : 720;
        fmt.fmt.pix.sizeimage = (default_format == 0) ?
                                (fmt.fmt.pix.width * fmt.fmt.pix.height * 2) :
                                (fmt.fmt.pix.width * fmt.fmt.pix.height * 1.5);
        switch (default_format)
        {
        case 1:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            break;

        case 2:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
            break;

        case 3:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H265;
            break;

        case 0:
        default:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
            break;
        }
        fmt.fmt.pix.field = V4L2_FIELD_ANY;

        /* Open the V4L2 device. */
        ret = v4l2_open(&vdev, v4l2_devname, &fmt);
        if (vdev == NULL || ret < 0)
            return 1;
    }

    /* Open the UVC device. */
    ret = uvc_open(&udev, uvc_devname);
    if (udev == NULL || ret < 0)
        return 1;

    udev->uvc_devname = uvc_devname;
    udev->video_id = id;
    udev->fc = fc;

    if (!dummy_data_gen_mode && !mjpeg_image)
    {
        vdev->v4l2_devname = v4l2_devname;
        /* Bind UVC and V4L2 devices. */
        udev->vdev = vdev;
        vdev->udev = udev;
    }

    /* Set parameters as passed by user. */
    udev->width = (default_resolution == 0) ? 640 : 1280;
    udev->height = (default_resolution == 0) ? 480 : 720;
    udev->imgsize = (default_format == 0) ?
                    (udev->width * udev->height * 2) :
                    (udev->width * udev->height * 2/*1.5*/);
    switch (default_format)
    {
    case 1:
        udev->fcc = V4L2_PIX_FMT_MJPEG;
        break;

    case 2:
        udev->fcc = V4L2_PIX_FMT_H264;
        break;

    case 3:
        udev->fcc = V4L2_PIX_FMT_H265;
        break;

    case 0:
    default:
        udev->fcc = V4L2_PIX_FMT_YUYV;
        break;
    }
    uvc_set_user_fcc(udev->fcc, udev->video_id);
    udev->io = uvc_io_method;
    udev->bulk = bulk_mode;
    udev->nbufs = nbufs;
    udev->mult = mult;
    udev->burst = burst;
    udev->speed = speed;

    udev->control = 0;

    if (dummy_data_gen_mode || mjpeg_image)
        /* UVC standalone setup. */
        udev->run_standalone = 1;

    if (!dummy_data_gen_mode && !mjpeg_image)
    {
        /* UVC - V4L2 integrated path */
        vdev->nbufs = nbufs;

        /*
         * IO methods used at UVC and V4L2 domains must be
         * complementary to avoid any memcpy from the CPU.
         */
        switch (uvc_io_method)
        {
        case IO_METHOD_MMAP:
            vdev->io = IO_METHOD_USERPTR;
            break;
        case IO_METHOD_USERPTR:
            vdev->io = IO_METHOD_MMAP;
            break;
        case IO_METHOD_DMA_BUFF:
        default:
            vdev->io = IO_METHOD_DMA_BUFF;
            //v4l2_reqbufs(vdev, vdev->nbufs);
            break;
        }
    }

    switch (speed)
    {
    case USB_SPEED_FULL:
        /* Full Speed. */
        if (bulk_mode)
            udev->maxpkt = 64;
        else
            udev->maxpkt = 1023;
        break;

    case USB_SPEED_HIGH:
        /* High Speed. */
        if (bulk_mode)
            udev->maxpkt = 512;
        else
            udev->maxpkt = 1024;
        break;

    case USB_SPEED_SUPER:
    default:
        /* Super Speed. */
        if (bulk_mode)
            udev->maxpkt = 1024;
        else
            udev->maxpkt = 1024;
        break;
    }
    if (!dummy_data_gen_mode && !mjpeg_image //&&
       )  //(IO_METHOD_MMAP == vdev->io)
    {
        /*
         * Ensure that the V4L2 video capture device has already some
         * buffers queued.
         */
        v4l2_reqbufs(vdev, vdev->nbufs);
    }

    if (mjpeg_image)
        image_load(udev, mjpeg_image);

    /* Init UVC events. */
    uvc_events_init(udev);

    uvc_set_user_run_state(true, udev->video_id);

    while (uvc_get_user_run_state(udev->video_id))
    {
        if (!dummy_data_gen_mode && !mjpeg_image)
            FD_ZERO(&fdsv);

        FD_ZERO(&fdsu);

        /* We want both setup and data events on UVC interface.. */
        FD_SET(udev->uvc_fd, &fdsu);

        fd_set efds = fdsu;
        fd_set dfds = fdsu;

        /* ..but only data events on V4L2 interface */
        if (!dummy_data_gen_mode && !mjpeg_image)
            FD_SET(vdev->v4l2_fd, &fdsv);

        /* Timeout. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        if (!dummy_data_gen_mode && !mjpeg_image)
        {
            nfds = max(vdev->v4l2_fd, udev->uvc_fd);
            ret = select(nfds + 1, &fdsv, &dfds, &efds, &tv);
        }
        else
        {
            ret = select(udev->uvc_fd + 1, NULL,
                         &dfds, &efds, &tv);
        }

        if (-1 == ret)
        {
            LOG_ERROR("select error %d, %s\n",
                      errno, strerror(errno));
            if (EINTR == errno)
                continue;

            break;
        }

        if (0 == ret)
        {
            if (udev->bulk)
                continue;
            LOG_ERROR("select timeout\n");
            if (!access("/tmp/uvc_no_timeout", 0))
                continue;
            break;
        }

        if(app_quit) {
           LOG_ERROR("app quit=%d...\n",app_quit);
           if(3 == app_quit) {
               uvc_control_inbuf_deinit();
#if USE_RK_AISERVER
               uvc_ipc_reconnect();
#else
               break;
#endif
               app_quit = 0;
           } else
             break;
        }

        if (FD_ISSET(udev->uvc_fd, &efds))
            uvc_events_process(udev);
        if (FD_ISSET(udev->uvc_fd, &dfds))
            uvc_video_process(udev);
        if (!dummy_data_gen_mode && !mjpeg_image)
            if (FD_ISSET(vdev->v4l2_fd, &fdsv))
                v4l2_process_data(vdev);

    }

    if (!dummy_data_gen_mode && !mjpeg_image && vdev->is_streaming)
    {
        /* Stop V4L2 streaming... */
        v4l2_stop_capturing(vdev);
        v4l2_uninit_device(vdev);
        v4l2_reqbufs(vdev, 0);
        vdev->is_streaming = 0;
    }

    set_uvc_control_stop();
    if (udev->is_streaming)
    {
        /* ... and now UVC streaming.. */
        uvc_video_stream(udev, 0);
        uvc_uninit_device(udev);
        uvc_video_reqbufs(udev, 0);
        udev->is_streaming = 0;
    }

    if (!dummy_data_gen_mode && !mjpeg_image)
        v4l2_close(vdev);

    uvc_close(udev);
    //join mpp thread
    uvc_control_exit();
    uvc_buffer_deinit(id);
#if USE_RK_AISERVER
    uvc_ipc_event(UVC_IPC_EVENT_STOP, NULL);
#endif
    set_uvc_control_restart();

    LOG_INFO("uvc_gadget_main exit");

    return 0;
}


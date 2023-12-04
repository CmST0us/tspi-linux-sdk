//#define LOG_NDEBUG 0
#define LOG_TAG "CamCaptureHelper"
#include <utils/Log.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/select.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "CamCaptureHelper.h"

#define FMT_NUM_PLANES 1

static enum v4l2_buf_type m_cur_type;

CamCaptureHelper::CamCaptureHelper() : mDevfd(-1)
{
    ALOGD("CamCaptureHelper enter");
    // default v4l2 request buffer count
    mPixBufCnt = 4;
}

CamCaptureHelper::~CamCaptureHelper()
{
    ALOGD("~CamCaptureHelper enter");
    deinit();
}

// Wrap ioctl() to spin on EINTR
int CamCaptureHelper::v4l2Ioctl(int fd, int req, void* arg)
{
    struct timespec poll_time;
    int ret;

    while ((ret = ioctl(fd, req, arg))) {
        if (ret == -1 && (EINTR != errno && EAGAIN != errno)) {
            break;
        }
        // 10 milliseconds
        poll_time.tv_sec = 0;
        poll_time.tv_nsec = 10000000;
        nanosleep(&poll_time, NULL);
    }

    return ret;
}

bool CamCaptureHelper::init(MetaInfo* meta)
{
    struct v4l2_capability cap;
    struct v4l2_format vfmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;

    int i, buf_len;

    if (meta == NULL) {
        ALOGE("Failed to get metaData");
        return false;
    }

    ALOGD("camera capture init start: dev %s, wh %dx%d, format %d, bufcnt %d", meta->video_dev, meta->width,
          meta->height, meta->format, mPixBufCnt);

    mDevfd = open(meta->video_dev, O_RDWR, 0);
    if (mDevfd < 0) {
        ALOGE("Cannot open device");
        goto _FAIL;
    }

    // Determine if fd is a V4L2 Device
    if (0 != v4l2Ioctl(mDevfd, VIDIOC_QUERYCAP, &cap)) {
        ALOGE("Not v4l2 compatible");
        goto _FAIL;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ALOGE("Capture not supported");
        goto _FAIL;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ALOGE("Streaming IO Not Supported");
        goto _FAIL;
    }

    // Preserve original settings as set by v4l2-ctl for example
    // vfmt = (struct v4l2_format) {0};
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    vfmt.fmt.pix.width = meta->width;
    vfmt.fmt.pix.height = meta->height;

    // vfmt.fmt.pix.pixelformat = meta->format;
    vfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;

    type = (v4l2_buf_type)vfmt.type;
    m_cur_type = (v4l2_buf_type)vfmt.type;

    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_S_FMT, &vfmt)) {
        ALOGE("VIDIOC_S_FMT");
        goto _FAIL;
    }

    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_G_FMT, &vfmt)) {
        ALOGE("VIDIOC_G_FMT");
        goto _FAIL;
    }

    ALOGD("VIDIOC_G_FMT w %d h %d", vfmt.fmt.pix.width, vfmt.fmt.pix.height);

    // Request memory-mapped buffers
    // req = (struct v4l2_requestbuffers) {0};
    req.count = mPixBufCnt;
    req.type = type;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_REQBUFS, &req)) {
        ALOGE("Device does not support mmap");
        goto _FAIL;
    }

    if (req.count != mPixBufCnt) {
        ALOGE("Device buffer count mismatch");
        goto _FAIL;
    }

    // mmap() the buffers into userspace memory
    for (i = 0; i < mPixBufCnt; i++) {
        // buf = (struct v4l2_buffer) {0};
        buf.type = type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        buf.memory = V4L2_MEMORY_MMAP;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == v4l2Ioctl(mDevfd, VIDIOC_QUERYBUF, &buf)) {
            ALOGE("ERROR: VIDIOC_QUERYBUF");
            goto _FAIL;
        }

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf.type) {
            // tmp_buffers[n_buffers].length = buf.m.planes[0].length;
            buf_len = buf.m.planes[0].length;
            mCambuf[i].start =
                mmap(NULL /* start anywhere */, buf.m.planes[0].length, PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */, mDevfd, buf.m.planes[0].m.mem_offset);
        } else {
            buf_len = buf.length;
            mCambuf[i].start = mmap(NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */,
                                    MAP_SHARED /* recommended */, mDevfd, buf.m.offset);
        }
        if (MAP_FAILED == mCambuf[i].start) {
            ALOGE("ERROR: Failed to map device frame buffers");
            goto _FAIL;
        }
        mCambuf[i].length = buf_len;

        struct v4l2_exportbuffer expbuf;
        // struct v4l2_exportbuffer expbuf = (struct v4l2_exportbuffer) {0} ;
        // xcam_mem_clear (expbuf);
        expbuf.type = type;
        expbuf.index = i;
        expbuf.flags = O_CLOEXEC;
        if (v4l2Ioctl(mDevfd, VIDIOC_EXPBUF, &expbuf) < 0) {
            ALOGE("get dma buf failed");
            goto _FAIL;
        } else {
            ALOGD("get dma buf(%d)-fd: %d", i, expbuf.fd);
        }
        mCambuf[i].export_fd = expbuf.fd;
    }

    for (i = 0; i < mPixBufCnt; i++) {
        struct v4l2_plane planes[FMT_NUM_PLANES];

        // buf = (struct v4l2_buffer) {0};
        buf.type = type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == v4l2Ioctl(mDevfd, VIDIOC_QBUF, &buf)) {
            ALOGE("ERROR: VIDIOC_QBUF %d", i);
            goto _FAIL;
        }
    }

    // Start capturing
    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_STREAMON, &m_cur_type)) {
        ALOGE("ERROR: VIDIOC_STREAMON");
        return false;
    }

    return true;

_FAIL:
    deinit();
    return false;
}

void CamCaptureHelper::deinit()
{
    enum v4l2_buf_type type;
    int i;

    if (mDevfd < 0)
        return;

    // Stop capturing
    type = m_cur_type;

    v4l2Ioctl(mDevfd, VIDIOC_STREAMOFF, &type);

    // un-mmap() buffers
    for (i = 0; i < mPixBufCnt; i++) {
        munmap(mCambuf[i].start, mCambuf[i].length);
        close(mCambuf[i].export_fd);
    }

    // Close v4l2 device
    close(mDevfd);
    mDevfd = -1;
}

// Returns a pointer to a captured frame and its meta-data. NOT thread-safe.
int CamCaptureHelper::v4l2DequeueBuf()
{
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;

    if (!mDevfd) {
        ALOGE("Init first");
        return -1;
    }

    type = m_cur_type;
    // buf = (struct v4l2_buffer) {0};
    buf.type = type;
    buf.memory = V4L2_MEMORY_MMAP;

    struct v4l2_plane planes[FMT_NUM_PLANES];
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_DQBUF, &buf)) {
        ALOGE("VIDIOC_DQBUF");
        return -1;
    }

    if (buf.index > mPixBufCnt) {
        ALOGE("buffer index out of bounds");
        return -1;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == type)
        buf.bytesused = buf.m.planes[0].bytesused;

    return buf.index;
}

// It's OK to capture into this framebuffer now
void CamCaptureHelper::v4l2QueueBuf(int idx)
{
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;

    if (!mDevfd) {
        ALOGE("Init first");
        return;
    }

    if (idx < 0)
        return;

    type = m_cur_type;
    // buf = (struct v4l2_buffer) {0};
    buf.type = type;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = idx;

    struct v4l2_plane planes[FMT_NUM_PLANES];
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    // Tell kernel it's ok to overwrite this frame
    if (-1 == v4l2Ioctl(mDevfd, VIDIOC_QBUF, &buf)) {
        ALOGE("VIDIOC_QBUF");
    }
}

bool CamCaptureHelper::getCameraBuffer(QMediaBuffer* buffer)
{
    int idx = v4l2DequeueBuf();
    if (idx < 0)
        return false;

    if (buffer == NULL) {
        ALOGE("getCameraBuffer: buffer NULL");
        return false;
    }

    buffer->setData(mCambuf[idx].start, mCambuf[idx].length);
    buffer->setFd(mCambuf[idx].export_fd);
    buffer->setBufferID(idx);

    return true;
}

bool CamCaptureHelper::putCameraBuffer(QMediaBuffer* buffer)
{
    if (buffer->getBufferID() > 0) {
        v4l2QueueBuf(buffer->getBufferID());
    }
    return true;
}

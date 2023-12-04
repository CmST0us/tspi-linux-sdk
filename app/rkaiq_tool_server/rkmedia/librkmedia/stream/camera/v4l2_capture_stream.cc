// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/mman.h>
#include <vector>

#include "buffer.h"
#include "utils.h"
#include "v4l2_stream.h"

namespace easymedia
{

    class V4L2CaptureStream : public V4L2Stream
    {
      public:
        V4L2CaptureStream(const char* param);
        virtual ~V4L2CaptureStream()
        {
            V4L2CaptureStream::Close();
            assert(!started);
        }
        static const char* GetStreamName()
        {
            return "v4l2_capture_stream";
        }
        virtual std::shared_ptr<MediaBuffer> Read();
        virtual int Open() final;
        virtual int Close() final;

      private:
        int BufferExport(enum v4l2_buf_type bt, int index, int* dmafd);
        enum v4l2_memory memory_type;
        std::string data_type;
        PixelFormat pix_fmt;
        int width, height;
        int colorspace;
        int loop_num;
        int quantization;
        std::vector<MediaBuffer> buffer_vec;
        bool started;
    };

    V4L2CaptureStream::V4L2CaptureStream(const char* param)
        : V4L2Stream(param), memory_type(V4L2_MEMORY_MMAP), data_type(IMAGE_NV12), pix_fmt(PIX_FMT_NONE), width(0),
          height(0), colorspace(-1), loop_num(2), quantization(-1), started(false)
    {
        if (device.empty()) {
            return;
        }
        std::map<std::string, std::string> params;
        std::list<std::pair<const std::string, std::string&>> req_list;

        std::string mem_type, str_loop_num;
        std::string str_width, str_height, str_color_space, str_quantization;
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_V4L2_MEM_TYPE, mem_type));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_FRAMES, str_loop_num));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_OUTPUTDATATYPE, data_type));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_BUFFER_WIDTH, str_width));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_BUFFER_HEIGHT, str_height));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_V4L2_COLORSPACE, str_color_space));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_V4L2_QUANTIZATION, str_quantization));
        int ret = parse_media_param_match(param, params, req_list);
        if (ret == 0) {
            return;
        }
        if (!mem_type.empty()) {
            memory_type = static_cast<enum v4l2_memory>(GetV4L2Type(mem_type.c_str()));
        }
        if (!str_loop_num.empty()) {
            loop_num = std::stoi(str_loop_num);
        }
        assert(loop_num >= 2);
        if (!str_width.empty()) {
            width = std::stoi(str_width);
        }
        if (!str_height.empty()) {
            height = std::stoi(str_height);
        }
        if (!str_color_space.empty()) {
            colorspace = std::stoi(str_color_space);
        }
        if (!str_quantization.empty()) {
            quantization = std::stoi(str_quantization);
        }
    }

    int V4L2CaptureStream::BufferExport(enum v4l2_buf_type bt, int index, int* dmafd)
    {
        struct v4l2_exportbuffer expbuf;

        memset(&expbuf, 0, sizeof(expbuf));
        expbuf.type = bt;
        expbuf.index = index;
        if (v4l2_ctx->IoCtrl(VIDIOC_EXPBUF, &expbuf) == -1) {
            LOG("VIDIOC_EXPBUF  %d failed, %m\n", index);
            return -1;
        }
        *dmafd = expbuf.fd;

        return 0;
    }

    class V4L2Buffer
    {
      public:
        V4L2Buffer() : dmafd(-1), ptr(nullptr), length(0), munmap_f(nullptr)
        {
        }
        ~V4L2Buffer()
        {
            if (dmafd >= 0) {
                close(dmafd);
            }
            if (ptr && ptr != MAP_FAILED && munmap_f) {
                munmap_f(ptr, length);
            }
        }
        int dmafd;
        void* ptr;
        size_t length;
        int (*munmap_f)(void* _start, size_t length);
    };

    static int __free_v4l2buffer(void* arg)
    {
        delete static_cast<V4L2Buffer*>(arg);
        return 0;
    }

    int V4L2CaptureStream::Open()
    {
        const char* dev = device.c_str();
        if (width <= 0 || height <= 0) {
            LOG("Invalid param, device=%s, width=%d, height=%d\n", dev, width, height);
            return -EINVAL;
        }
        int ret = V4L2Stream::Open();
        if (ret) {
            return ret;
        }

        struct v4l2_capability cap;
        memset(&cap, 0, sizeof(cap));
        if (v4l2_ctx->IoCtrl(VIDIOC_QUERYCAP, &cap) < 0) {
            LOG("Failed to ioctl(VIDIOC_QUERYCAP): %m\n");
            return -1;
        }
        if ((capture_type == V4L2_BUF_TYPE_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            LOG("%s, Not a video capture device.\n", dev);
            return -1;
        }
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            LOG("%s does not support the streaming I/O method.\n", dev);
            return -1;
        }
        const char* data_type_str = data_type.c_str();
        struct v4l2_format fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = capture_type;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = GetV4L2FmtByString(data_type_str);
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        if (quantization >= 0) {
            fmt.fmt.pix.priv = V4L2_PIX_FMT_PRIV_MAGIC;
            fmt.fmt.pix.quantization = quantization;
        }
        if (colorspace >= 0) {
            fmt.fmt.pix.colorspace = colorspace;
        }
        if (fmt.fmt.pix.pixelformat == 0) {
            LOG("unsupport input format : %s\n", data_type_str);
            return -1;
        }
        if (v4l2_ctx->IoCtrl(VIDIOC_S_FMT, &fmt) < 0) {
            LOG("%s, s fmt failed(cap type=%d, %c%c%c%c), %m\n", dev, capture_type,
                DUMP_FOURCC(fmt.fmt.pix.pixelformat));
            return -1;
        }
        if (GetV4L2FmtByString(data_type_str) != fmt.fmt.pix.pixelformat) {
            LOG("%s, expect %s, return %c%c%c%c\n", dev, data_type_str, DUMP_FOURCC(fmt.fmt.pix.pixelformat));
            return -1;
        }
        pix_fmt = StringToPixFmt(data_type_str);
        if (width != (int)fmt.fmt.pix.width || height != (int)fmt.fmt.pix.height) {
            LOG("%s change res from %dx%d to %dx%d\n", dev, width, height, fmt.fmt.pix.width, fmt.fmt.pix.height);
            width = fmt.fmt.pix.width;
            height = fmt.fmt.pix.height;
            return -1;
        }
        if (fmt.fmt.pix.field == V4L2_FIELD_INTERLACED) {
            LOG("%s is using the interlaced mode\n", dev);
        }

        struct v4l2_requestbuffers req;
        req.type = capture_type;
        req.count = loop_num;
        req.memory = memory_type;
        if (v4l2_ctx->IoCtrl(VIDIOC_REQBUFS, &req) < 0) {
            LOG("%s, count=%d, ioctl(VIDIOC_REQBUFS): %m\n", dev, loop_num);
            return -1;
        }
        int w = UPALIGNTO16(width);
        int h = UPALIGNTO16(height);
        if (memory_type == V4L2_MEMORY_DMABUF) {
            int size = 0;
            if (pix_fmt != PIX_FMT_NONE) {
                size = CalPixFmtSize(pix_fmt, w, h, 16);
            }
            if (size == 0) { // unknown pixel format
                size = w * h * 4;
            }
            for (size_t i = 0; i < req.count; i++) {
                struct v4l2_buffer buf;
                memset(&buf, 0, sizeof(buf));
                buf.type = req.type;
                buf.index = i;
                buf.memory = req.memory;

                auto&& buffer = MediaBuffer::Alloc2(size, MediaBuffer::MemType::MEM_HARD_WARE);
                if (buffer.GetSize() == 0) {
                    errno = ENOMEM;
                    return -1;
                }
                buffer_vec.push_back(buffer);
                buf.m.fd = buffer.GetFD();
                buf.length = buffer.GetSize();
                if (v4l2_ctx->IoCtrl(VIDIOC_QBUF, &buf) < 0) {
                    LOG("%s ioctl(VIDIOC_QBUF): %m\n", dev);
                    return -1;
                }
            }
        } else if (memory_type == V4L2_MEMORY_MMAP) {
            for (size_t i = 0; i < req.count; i++) {
                struct v4l2_buffer buf;
                void* ptr = MAP_FAILED;

                V4L2Buffer* buffer = new V4L2Buffer();
                if (!buffer) {
                    errno = ENOMEM;
                    return -1;
                }
                buffer_vec.push_back(MediaBuffer(nullptr, 0, -1, buffer, __free_v4l2buffer));
                memset(&buf, 0, sizeof(buf));
                buf.type = req.type;
                buf.index = i;
                buf.memory = req.memory;
                if (v4l2_ctx->IoCtrl(VIDIOC_QUERYBUF, &buf) < 0) {
                    LOG("%s ioctl(VIDIOC_QUERYBUF): %m\n", dev);
                    return -1;
                }
                ptr = v4l2_mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
                if (ptr == MAP_FAILED) {
                    LOG("%s v4l2_mmap (%d): %m\n", dev, (int)i);
                    return -1;
                }
                MediaBuffer& mb = buffer_vec[i];
                buffer->munmap_f = vio.munmap_f;
                buffer->ptr = ptr;
                mb.SetPtr(ptr);
                buffer->length = buf.length;
                mb.SetSize(buf.length);
                LOGD("query buf.length=%d\n", (int)buf.length);
            }
            for (size_t i = 0; i < req.count; ++i) {
                struct v4l2_buffer buf;
                int dmafd = -1;

                memset(&buf, 0, sizeof(buf));
                buf.type = req.type;
                buf.memory = req.memory;
                buf.index = i;
                if (v4l2_ctx->IoCtrl(VIDIOC_QBUF, &buf) < 0) {
                    LOG("%s, ioctl(VIDIOC_QBUF): %m\n", dev);
                    return -1;
                }
                if (!BufferExport(capture_type, i, &dmafd)) {
                    MediaBuffer& mb = buffer_vec[i];
                    V4L2Buffer* buffer = static_cast<V4L2Buffer*>(mb.GetUserData().get());
                    buffer->dmafd = dmafd;
                    mb.SetFD(dmafd);
                }
            }
        }

        SetReadable(true);
        return 0;
    }
    int V4L2CaptureStream::Close()
    {
        started = false;
        return V4L2Stream::Close();
    }

    class V4L2AutoQBUF
    {
      public:
        V4L2AutoQBUF(std::shared_ptr<V4L2Context> ctx, struct v4l2_buffer buf) : v4l2_ctx(ctx), v4l2_buf(buf)
        {
        }
        ~V4L2AutoQBUF()
        {
            if (v4l2_ctx->IoCtrl(VIDIOC_QBUF, &v4l2_buf) < 0) {
                LOG("index=%d, ioctl(VIDIOC_QBUF): %m\n", v4l2_buf.index);
            }
        }

      private:
        std::shared_ptr<V4L2Context> v4l2_ctx;
        struct v4l2_buffer v4l2_buf;
    };

    class AutoQBUFMediaBuffer : public MediaBuffer
    {
      public:
        AutoQBUFMediaBuffer(const MediaBuffer& mb, std::shared_ptr<V4L2Context> ctx, struct v4l2_buffer buf)
            : MediaBuffer(mb), auto_qbuf(ctx, buf)
        {
        }

      private:
        V4L2AutoQBUF auto_qbuf;
    };

    class AutoQBUFImageBuffer : public ImageBuffer
    {
      public:
        AutoQBUFImageBuffer(const MediaBuffer& mb, const ImageInfo& info, std::shared_ptr<V4L2Context> ctx,
                            struct v4l2_buffer buf)
            : ImageBuffer(mb, info), auto_qbuf(ctx, buf)
        {
        }

      private:
        V4L2AutoQBUF auto_qbuf;
    };

    std::shared_ptr<MediaBuffer> V4L2CaptureStream::Read()
    {
        const char* dev = device.c_str();
        if (!started && v4l2_ctx->SetStarted(true)) {
            started = true;
        }

        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = capture_type;
        buf.memory = memory_type;
        int ret = v4l2_ctx->IoCtrl(VIDIOC_DQBUF, &buf);
        if (ret < 0) {
            LOG("%s, ioctl(VIDIOC_DQBUF): %m\n", dev);
            return nullptr;
        }
        struct timeval buf_ts = buf.timestamp;
        MediaBuffer& mb = buffer_vec[buf.index];
        std::shared_ptr<MediaBuffer> ret_buf;
        if (buf.bytesused > 0) {
            if (pix_fmt != PIX_FMT_NONE) {
                ImageInfo info{pix_fmt, width, height, width, height};
                ret_buf = std::make_shared<AutoQBUFImageBuffer>(mb, info, v4l2_ctx, buf);
            } else {
                ret_buf = std::make_shared<AutoQBUFMediaBuffer>(mb, v4l2_ctx, buf);
            }
        }
        if (ret_buf) {
            assert(ret_buf->GetFD() == mb.GetFD());
            if (buf.memory == V4L2_MEMORY_DMABUF) {
                assert(ret_buf->GetFD() == buf.m.fd);
            }
            ret_buf->SetFrameSequenceNumber(buf.sequence);
            ret_buf->SetAtomicTimeVal(buf_ts);
            ret_buf->SetTimeVal(buf_ts);
            ret_buf->SetValidSize(buf.bytesused);
        } else {
            if (v4l2_ctx->IoCtrl(VIDIOC_QBUF, &buf) < 0) {
                LOG("%s, index=%d, ioctl(VIDIOC_QBUF): %m\n", dev, buf.index);
            }
        }

        return ret_buf;
    }

    DEFINE_STREAM_FACTORY(V4L2CaptureStream, Stream)

    const char* FACTORY(V4L2CaptureStream)::ExpectedInputDataType()
    {
        return nullptr;
    }

    const char* FACTORY(V4L2CaptureStream)::OutPutDataType()
    {
        return GetStringOfV4L2Fmts().c_str();
    }

} // namespace easymedia

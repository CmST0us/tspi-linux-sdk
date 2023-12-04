// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_BUFFER_H_
#define EASYMEDIA_BUFFER_H_

#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include <memory>

#include "image.h"
#include "lock.h"
#include "media_type.h"
#include "rknn_user.h"
#include "sound.h"

typedef int (*DeleteFun)(void* arg);

namespace easymedia
{

    // wrapping existing buffer
    class _API MediaBuffer
    {
      public:
        // video flags
        static const uint32_t kExtraIntra = (1 << 0); // special, such as sps pps
        static const uint32_t kIntra = (1 << 1);
        static const uint32_t kPredicted = (1 << 2);
        static const uint32_t kBiPredictive = (1 << 3);
        static const uint32_t kBiDirectional = (1 << 4);
        static const uint32_t kSingleNalUnit = (1 << 5);

        // special flags
        static const uint32_t kBuildinLibvorbisenc = (1 << 16);

        MediaBuffer()
            : ptr(nullptr), size(0), fd(-1), valid_size(0), type(Type::None), user_flag(0), ustimestamp(0), eof(false),
              tsvc_level(-1)
        {
        }
        // Set userdata and delete function if you want free resource when destrut.
        MediaBuffer(void* buffer_ptr, size_t buffer_size, int buffer_fd = -1, void* user_data = nullptr,
                    DeleteFun df = nullptr)
            : ptr(buffer_ptr), size(buffer_size), fd(buffer_fd), valid_size(0), type(Type::None), user_flag(0),
              ustimestamp(0), eof(false), tsvc_level(-1)
        {
            SetUserData(user_data, df);
        }
        virtual ~MediaBuffer() = default;
        virtual PixelFormat GetPixelFormat() const
        {
            return PIX_FMT_NONE;
        }
        virtual SampleFormat GetSampleFormat() const
        {
            return SAMPLE_FMT_NONE;
        }
        void BeginCPUAccess(bool readonly);
        void EndCPUAccess(bool readonly);
        int GetFD() const
        {
            return fd;
        }
        void SetFD(int new_fd)
        {
            fd = new_fd;
        }
        void* GetPtr() const
        {
            return ptr;
        }
        void SetPtr(void* addr)
        {
            ptr = addr;
        }
        size_t GetSize() const
        {
            return size;
        }
        void SetSize(size_t s)
        {
            size = s;
        }
        size_t GetValidSize() const
        {
            return valid_size;
        }
        void SetValidSize(size_t s)
        {
            valid_size = s;
        }
        Type GetType() const
        {
            return type;
        }
        // be careful to set type, depends on final buffer class.
        // Maybe it should be set Protected.
        void SetType(Type t)
        {
            type = t;
        }
        uint32_t GetUserFlag() const
        {
            return user_flag;
        }
        void SetUserFlag(uint32_t flag)
        {
            user_flag = flag;
        }
        // microsecond
        int64_t GetUSTimeStamp() const
        {
            return ustimestamp;
        }
        struct timeval GetTimeVal() const
        {
            struct timeval ret;
            ret.tv_sec = ustimestamp / 1000000LL;
            ret.tv_usec = ustimestamp % 1000000LL;
            return ret;
        }
        void SetUSTimeStamp(int64_t us)
        {
            ustimestamp = us;
        }
        void SetTimeVal(const struct timeval& val)
        {
            ustimestamp = val.tv_sec * 1000000LL + val.tv_usec;
        }
        bool IsEOF() const
        {
            return eof;
        }
        void SetEOF(bool val)
        {
            eof = val;
        }
        int GetTsvcLevel()
        {
            return tsvc_level;
        }
        void SetTsvcLevel(int _level)
        {
            tsvc_level = _level;
        }

        void SetUserData(void* user_data, DeleteFun df)
        {
            if (user_data) {
                if (df) {
                    userdata.reset(user_data, df);
                } else
                    userdata.reset(user_data, [](void*) {}); // do nothing when delete
            } else {
                userdata.reset();
            }
        }
        int64_t GetAtomicClock() const
        {
            return atomic_clock;
        }
        struct timeval GetAtomicTimeVal() const
        {
            struct timeval ret;
            ret.tv_sec = atomic_clock / 1000000LL;
            ret.tv_usec = atomic_clock % 1000000LL;
            return ret;
        }
        void SetAtomicClock(int64_t us)
        {
            atomic_clock = us;
        }
        void SetAtomicTimeVal(const struct timeval& val)
        {
            atomic_clock = val.tv_sec * 1000000LL + val.tv_usec;
        }

        void SetUserData(std::shared_ptr<void> user_data)
        {
            userdata = user_data;
        }
        std::shared_ptr<void> GetUserData()
        {
            return userdata;
        }

        void SetFrameSequenceNumber(uint32_t seq_number)
        {
            frame_sequence_number = seq_number;
        }
        uint32_t GetFrameSequenceNumber()
        {
            return frame_sequence_number;
        }

        void SetRelatedSPtr(const std::shared_ptr<void>& rdata, int index = -1)
        {
            if (index < 0) {
                related_sptrs.push_back(rdata);
                return;
            } else if (index >= (int)related_sptrs.size()) {
                related_sptrs.resize(index + 1);
            }
            related_sptrs[index] = rdata;
        }
        std::vector<std::shared_ptr<void>>& GetRelatedSPtrs()
        {
            return related_sptrs;
        }

        bool IsValid()
        {
            return valid_size > 0;
        }
        bool IsHwBuffer()
        {
            return fd >= 0;
        }

        enum class MemType
        {
            MEM_COMMON,
            MEM_HARD_WARE,
        };
        static std::shared_ptr<MediaBuffer> Alloc(size_t size, MemType type = MemType::MEM_COMMON);
        static MediaBuffer Alloc2(size_t size, MemType type = MemType::MEM_COMMON);
        static std::shared_ptr<MediaBuffer> Clone(MediaBuffer& src, MemType dst_type = MemType::MEM_COMMON);

      private:
        // copy attributs except buffer
        void CopyAttribute(MediaBuffer& src_attr);

        uint32_t frame_sequence_number;
        void* ptr; // buffer virtual address
        size_t size;
        int fd;            // buffer fd
        size_t valid_size; // valid data size, less than above size
        Type type;
        uint32_t user_flag;
        int64_t ustimestamp;
        int64_t atomic_clock;
        bool eof;
        int tsvc_level; // for avc/hevc encoder
        std::shared_ptr<void> userdata;
        std::vector<std::shared_ptr<void>> related_sptrs;
    };

    MediaBuffer::MemType StringToMemType(const char* s);

    // Audio sample buffer
    class _API SampleBuffer : public MediaBuffer
    {
      public:
        SampleBuffer()
        {
            ResetValues();
        }
        SampleBuffer(const MediaBuffer& buffer, SampleFormat fmt = SAMPLE_FMT_NONE) : MediaBuffer(buffer)
        {
            ResetValues();
            sample_info.fmt = fmt;
        }
        SampleBuffer(const MediaBuffer& buffer, const SampleInfo& info) : MediaBuffer(buffer), sample_info(info)
        {
            SetType(Type::Audio);
        }
        virtual ~SampleBuffer() = default;
        virtual SampleFormat GetSampleFormat() const override
        {
            return sample_info.fmt;
        }

        SampleInfo& GetSampleInfo()
        {
            return sample_info;
        }
        size_t GetSampleSize() const
        {
            return ::GetSampleSize(sample_info);
        }
        void SetSamples(int num)
        {
            sample_info.nb_samples = num;
            SetValidSize(num * GetSampleSize());
        }
        int GetSamples() const
        {
            return sample_info.nb_samples;
        }

      private:
        void ResetValues()
        {
            SetType(Type::Audio);
            memset(&sample_info, 0, sizeof(sample_info));
            sample_info.fmt = SAMPLE_FMT_NONE;
        }
        SampleInfo sample_info;
    };

    // Image buffer
    class _API ImageBuffer : public MediaBuffer
    {
      public:
        ImageBuffer()
        {
            ResetValues();
        }
        ImageBuffer(const MediaBuffer& buffer) : MediaBuffer(buffer)
        {
            ResetValues();
        }
        ImageBuffer(const MediaBuffer& buffer, const ImageInfo& info) : MediaBuffer(buffer), image_info(info)
        {
            SetType(Type::Image);
            // if set a valid info, set valid size
            size_t s = CalPixFmtSize(info);
            if (s > 0) {
                SetValidSize(s);
            }
        }
        virtual ~ImageBuffer() = default;
        virtual PixelFormat GetPixelFormat() const override
        {
            return image_info.pix_fmt;
        }
        int GetWidth() const
        {
            return image_info.width;
        }
        int GetHeight() const
        {
            return image_info.height;
        }
        int GetVirWidth() const
        {
            return image_info.vir_width;
        }
        int GetVirHeight() const
        {
            return image_info.vir_height;
        }
        ImageInfo& GetImageInfo()
        {
            return image_info;
        }
        std::list<RknnResult>& GetRknnResult()
        {
            return nn_result;
        };

      private:
        void ResetValues()
        {
            SetType(Type::Image);
            memset(&image_info, 0, sizeof(image_info));
            image_info.pix_fmt = PIX_FMT_NONE;
        }
        ImageInfo image_info;
        std::list<RknnResult> nn_result;
    };

    class MediaGroupBuffer
    {
      public:
        MediaGroupBuffer() : pool(nullptr), ptr(nullptr), size(0), fd(-1)
        {
        }
        // Set userdata and delete function if you want free resource when destrut.
        MediaGroupBuffer(void* buffer_ptr, size_t buffer_size, int buffer_fd = -1, void* user_data = nullptr,
                         DeleteFun df = nullptr)
            : pool(nullptr), ptr(buffer_ptr), size(buffer_size), fd(buffer_fd)
        {
            SetUserData(user_data, df);
        }
        virtual ~MediaGroupBuffer() = default;

        void SetUserData(void* user_data, DeleteFun df)
        {
            if (user_data) {
                if (df) {
                    userdata.reset(user_data, df);
                } else
                    userdata.reset(user_data, [](void*) {}); // do nothing when delete
            } else {
                userdata.reset();
            }
        }

        void SetBufferPool(void* bp)
        {
            pool = bp;
        }

        int GetFD() const
        {
            return fd;
        }
        void* GetPtr() const
        {
            return ptr;
        }
        size_t GetSize() const
        {
            return size;
        }

        static MediaGroupBuffer* Alloc(size_t size, MediaBuffer::MemType type = MediaBuffer::MemType::MEM_COMMON);

      public:
        void* pool;

      private:
        void* ptr; // buffer virtual address
        size_t size;
        int fd; // buffer fd

        std::shared_ptr<void> userdata;
    };

    class _API BufferPool
    {
      public:
        BufferPool(int cnt, int size, MediaBuffer::MemType type);
        ~BufferPool();

        std::shared_ptr<MediaBuffer> GetBuffer(bool block = true);
        int PutBuffer(MediaGroupBuffer* mgb);

        void DumpInfo();

      private:
        std::list<MediaGroupBuffer*> ready_buffers;
        std::list<MediaGroupBuffer*> busy_buffers;
        ConditionLockMutex mtx;
        int buf_cnt;
        int buf_size;
    };

} // namespace easymedia

#endif // EASYMEDIA_BUFFER_H_

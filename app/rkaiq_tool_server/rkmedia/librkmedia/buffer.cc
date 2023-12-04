// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer.h"

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "key_string.h"
#include "utils.h"

namespace easymedia
{

    MediaBuffer::MemType StringToMemType(const char* s)
    {
        if (s) {
#ifdef LIBION
            if (!strcmp(s, KEY_MEM_ION) || !strcmp(s, KEY_MEM_HARDWARE)) {
                return MediaBuffer::MemType::MEM_HARD_WARE;
            }
#endif
#ifdef LIBDRM
            if (!strcmp(s, KEY_MEM_DRM) || !strcmp(s, KEY_MEM_HARDWARE)) {
                return MediaBuffer::MemType::MEM_HARD_WARE;
            }
#endif
            LOG("warning: %s is not supported or not integrated, fallback to common\n", s);
        }
        return MediaBuffer::MemType::MEM_COMMON;
    }

    static int free_common_memory(void* buffer)
    {
        if (buffer) {
            free(buffer);
        }

        return 0;
    }

    static MediaBuffer alloc_common_memory(size_t size)
    {
        void* buffer = malloc(size);
        if (!buffer) {
            return MediaBuffer();
        }
        return MediaBuffer(buffer, size, -1, buffer, free_common_memory);
    }

    static MediaGroupBuffer* alloc_common_memory_group(size_t size)
    {
        void* buffer = malloc(size);
        if (!buffer) {
            return nullptr;
        }
        MediaGroupBuffer* mgb = new MediaGroupBuffer(buffer, size, -1, buffer, free_common_memory);

        return mgb;
    }

#ifdef LIBION
    #include <ion/ion.h>
    class IonBuffer
    {
      public:
        IonBuffer(int param_client, ion_user_handle_t param_handle, int param_fd, void* param_map_ptr, size_t param_len)
            : client(param_client), handle(param_handle), fd(param_fd), map_ptr(param_map_ptr), len(param_len)
        {
        }
        ~IonBuffer();

      private:
        int client;
        ion_user_handle_t handle;
        int fd;
        void* map_ptr;
        size_t len;
    };

    IonBuffer::~IonBuffer()
    {
        if (map_ptr) {
            munmap(map_ptr, len);
        }
        if (fd >= 0) {
            close(fd);
        }
        if (client < 0) {
            return;
        }
        if (handle) {
            int ret = ion_free(client, handle);
            if (ret) {
                LOG("ion_free() failed <handle: %d>: %m!\n", handle);
            }
        }
        ion_close(client);
    }

    static int free_ion_memory(void* buffer)
    {
        assert(buffer);
        IonBuffer* ion_buffer = static_cast<IonBuffer*>(buffer);
        delete ion_buffer;
        return 0;
    }

    static MediaBuffer alloc_ion_memory(size_t size)
    {
        ion_user_handle_t handle;
        int ret;
        int fd;
        void* ptr;
        IonBuffer* buffer;
        int client = ion_open();
        if (client < 0) {
            LOG("ion_open() failed: %m\n");
            goto err;
        }
        ret = ion_alloc(client, size, 0, ION_HEAP_TYPE_DMA_MASK, 0, &handle);
        if (ret) {
            LOG("ion_alloc() failed: %m\n");
            ion_close(client);
            goto err;
        }
        ret = ion_share(client, handle, &fd);
        if (ret < 0) {
            LOG("ion_share() failed: %m\n");
            ion_free(client, handle);
            ion_close(client);
            goto err;
        }
        ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
        if (!ptr) {
            LOG("ion mmap() failed: %m\n");
            ion_free(client, handle);
            ion_close(client);
            close(fd);
            goto err;
        }
        buffer = new IonBuffer(client, handle, fd, ptr, size);
        if (!buffer) {
            ion_free(client, handle);
            ion_close(client);
            close(fd);
            munmap(ptr, size);
            goto err;
        }

        return MediaBuffer(ptr, size, fd, buffer, free_ion_memory);
    err:
        return MediaBuffer();
    }
#endif

#ifdef LIBDRM

    #include <drm_fourcc.h>
    #include <xf86drm.h>
    /**
     * Copy from libdrm_macros.h while is not exposed by libdrm,
     * be replaced by #include "libdrm_macros.h" someday.
     */
    /**
     * Static (compile-time) assertion.
     * Basically, use COND to dimension an array.  If COND is false/zero the
     * array size will be -1 and we'll get a compilation error.
     */
    #define STATIC_ASSERT(COND)                                                                                        \
        do {                                                                                                           \
            (void)sizeof(char[1 - 2 * !(COND)]);                                                                       \
        } while (0)

    #if defined(ANDROID) && !defined(__LP64__)
        #include <errno.h> /* for EINVAL */

    extern void* __mmap2(void*, size_t, int, int, int, size_t);

    static inline void* drm_mmap(void* addr, size_t length, int prot, int flags, int fd, loff_t offset)
    {
        /* offset must be aligned to 4096 (not necessarily the page size) */
        if (offset & 4095) {
            errno = EINVAL;
            return MAP_FAILED;
        }

        return __mmap2(addr, length, prot, flags, fd, (size_t)(offset >> 12));
    }

        #define drm_munmap(addr, length) munmap(addr, length)

    #else

        /* assume large file support exists */
        #define drm_mmap(addr, length, prot, flags, fd, offset) mmap(addr, length, prot, flags, fd, offset)

    static inline int drm_munmap(void* addr, size_t length)
    {
            /* Copied from configure code generated by AC_SYS_LARGEFILE */
        #define LARGE_OFF_T ((((off_t)1 << 31) << 31) - 1 + (((off_t)1 << 31) << 31))
        STATIC_ASSERT(LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1);
        #undef LARGE_OFF_T

        return munmap(addr, length);
    }
    #endif

    // default
    static int card_index = 0;
    static int drm_device_open(const char* device = nullptr)
    {
        drmVersionPtr version;
        char drm_dev[] = "/dev/dri/card0000";
        uint64_t has_dumb;

        if (!device) {
            snprintf(drm_dev, sizeof(drm_dev), DRM_DEV_NAME, DRM_DIR_NAME, card_index);
            device = drm_dev;
        }
        int fd = open(device, O_RDWR);
        if (fd < 0) {
            return fd;
        }
        version = drmGetVersion(fd);
        if (!version) {
            LOG("Failed to get version information "
                "from %s: probably not a DRM device?\n",
                device);
            close(fd);
            return -1;
        }
        LOG("Opened DRM device %s: driver %s "
            "version %d.%d.%d.\n",
            device, version->name, version->version_major, version->version_minor, version->version_patchlevel);
        drmFreeVersion(version);
        if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
            LOG("drm device '%s' "
                "does not support dumb buffers\n",
                device);
            close(fd);
            return -1;
        }
        return fd;
    }

    class DrmDevice
    {
      public:
        DrmDevice()
        {
            fd = drm_device_open();
        }
        ~DrmDevice()
        {
            if (fd >= 0) {
                close(fd);
            }
        }
        bool Valid()
        {
            return fd >= 0;
        }

        const static std::shared_ptr<DrmDevice>& GetInstance()
        {
            const static std::shared_ptr<DrmDevice> mDrmDevice = std::make_shared<DrmDevice>();

            return mDrmDevice;
        }

        int fd;
    };

    class DrmBuffer
    {
      public:
        DrmBuffer(std::shared_ptr<DrmDevice> dev, size_t s, __u32 flags = 0)
            : device(dev), handle(0), len(UPALIGNTO(s, PAGE_SIZE)), fd(-1), map_ptr(nullptr)
        {
            struct drm_mode_create_dumb dmcb;
            memset(&dmcb, 0, sizeof(struct drm_mode_create_dumb));
            dmcb.bpp = 8;
            dmcb.width = len;
            dmcb.height = 1;
            dmcb.flags = flags;
            int ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &dmcb);
            if (ret < 0) {
                LOG("Failed to create dumb<w,h,bpp: %d,%d,%d>: %m\n", dmcb.width, dmcb.height, dmcb.bpp);
                return;
            }
            assert(dmcb.handle > 0);
            assert(dmcb.size >= dmcb.width * dmcb.height * dmcb.bpp / 8);
            handle = dmcb.handle;
            len = dmcb.size;
            ret = drmPrimeHandleToFD(dev->fd, dmcb.handle, DRM_CLOEXEC, &fd);
            if (ret) {
                LOG("Failed to convert drm handle to fd: %m\n");
                return;
            }
            assert(fd >= 0);
        }
        ~DrmBuffer()
        {
            if (map_ptr) {
                drm_munmap(map_ptr, len);
            }
            int ret;
            if (handle > 0) {
                struct drm_mode_destroy_dumb data = {
                    .handle = handle,
                };
                ret = drmIoctl(device->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &data);
                if (ret) {
                    LOG("Failed to free drm handle <%d>: %m\n", handle);
                }
            }
            if (fd >= 0) {
                ret = close(fd);
                if (ret) {
                    LOG("Failed to close drm buffer fd <%d>: %m\n", fd);
                }
            }
        }
        bool MapToVirtual()
        {
            struct drm_mode_map_dumb dmmd;
            memset(&dmmd, 0, sizeof(dmmd));
            dmmd.handle = handle;
            int ret = drmIoctl(device->fd, DRM_IOCTL_MODE_MAP_DUMB, &dmmd);
            if (ret) {
                LOG("Failed to map dumb: %m\n");
                return false;
            }
            // default read and write
            void* ptr = drm_mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, device->fd, dmmd.offset);
            if (ptr == MAP_FAILED) {
                LOG("Failed to drm_mmap: %m\n");
                return false;
            }
            assert(ptr);
            map_ptr = ptr;
            return true;
        }
        bool Valid()
        {
            return fd >= 0;
        }

        std::shared_ptr<DrmDevice> device;
        __u32 handle;
        size_t len;
        int fd;
        void* map_ptr;
    };

    static int free_drm_memory(void* buffer)
    {
        assert(buffer);
        delete static_cast<DrmBuffer*>(buffer);
        return 0;
    }

    /* memory type definitions. */
    enum drm_rockchip_gem_mem_type
    {
        /* Physically Continuous memory. */
        ROCKCHIP_BO_CONTIG = 1 << 0,
        /* cachable mapping. */
        ROCKCHIP_BO_CACHABLE = 1 << 1,
        /* write-combine mapping. */
        ROCKCHIP_BO_WC = 1 << 2,
        ROCKCHIP_BO_SECURE = 1 << 3,
        ROCKCHIP_BO_MASK = ROCKCHIP_BO_CONTIG | ROCKCHIP_BO_CACHABLE | ROCKCHIP_BO_WC
    };

    static MediaBuffer alloc_drm_memory(size_t size, bool map = true)
    {
        const static std::shared_ptr<DrmDevice>& drm_dev = DrmDevice::GetInstance();
        DrmBuffer* db = nullptr;
        do {
            if (!drm_dev || !drm_dev->Valid()) {
                break;
            }
            db = new DrmBuffer(drm_dev, size, ROCKCHIP_BO_CACHABLE);
            if (!db || !db->Valid()) {
                break;
            }
            if (map && !db->MapToVirtual()) {
                break;
            }
            return MediaBuffer(db->map_ptr, db->len, db->fd, db, free_drm_memory);
        } while (false);
        if (db) {
            delete db;
        }
        return MediaBuffer();
    }

    static MediaGroupBuffer* alloc_drm_memory_group(size_t size, bool map = true)
    {
        const static std::shared_ptr<DrmDevice>& drm_dev = DrmDevice::GetInstance();
        DrmBuffer* db = nullptr;

        do {
            if (!drm_dev || !drm_dev->Valid()) {
                break;
            }
            db = new DrmBuffer(drm_dev, size, ROCKCHIP_BO_CACHABLE);
            if (!db || !db->Valid()) {
                break;
            }
            if (map && !db->MapToVirtual()) {
                break;
            }
            MediaGroupBuffer* mgb = nullptr;
            mgb = new MediaGroupBuffer(db->map_ptr, db->len, db->fd, db, free_drm_memory);
            return mgb;
        } while (false);
        if (db) {
            delete db;
        }

        return nullptr;
    }

#endif

    std::shared_ptr<MediaBuffer> MediaBuffer::Alloc(size_t size, MemType type)
    {
        MediaBuffer&& mb = Alloc2(size, type);
        if (mb.GetSize() == 0) {
            return nullptr;
        }
        return std::make_shared<MediaBuffer>(mb);
    }

    MediaBuffer MediaBuffer::Alloc2(size_t size, MemType type)
    {
        switch (type) {
            case MemType::MEM_COMMON:
                return alloc_common_memory(size);
#ifdef LIBION
            case MemType::MEM_HARD_WARE:
                return alloc_ion_memory(size);
#endif
#ifdef LIBDRM
            case MemType::MEM_HARD_WARE:
                return alloc_drm_memory(size);
#endif
            default:
                LOG("unknown memtype\n");
                return MediaBuffer();
        }
    }

    std::shared_ptr<MediaBuffer> MediaBuffer::Clone(MediaBuffer& src, MemType dst_type)
    {
        size_t size = src.GetValidSize();
        if (!size) {
            return nullptr;
        }
        auto new_buffer = Alloc(size, dst_type);
        if (!new_buffer) {
            LOG_NO_MEMORY();
            return nullptr;
        }
        if (src.IsHwBuffer() && new_buffer->IsHwBuffer()) {
            LOG_TODO(); // TODO: fd -> fd by RGA
        }
        memcpy(new_buffer->GetPtr(), src.GetPtr(), size);
        new_buffer->SetValidSize(size);
        new_buffer->CopyAttribute(src);
        return new_buffer;
    }

    void MediaBuffer::CopyAttribute(MediaBuffer& src_attr)
    {
        type = src_attr.GetType();
        user_flag = src_attr.GetUserFlag();
        ustimestamp = src_attr.GetUSTimeStamp();
        eof = src_attr.IsEOF();
    }

    struct dma_buf_sync {
        uint64_t flags;
    };

#define DMA_BUF_SYNC_READ (1 << 0)
#define DMA_BUF_SYNC_WRITE (2 << 0)
#define DMA_BUF_SYNC_RW (DMA_BUF_SYNC_READ | DMA_BUF_SYNC_WRITE)
#define DMA_BUF_SYNC_START (0 << 2)
#define DMA_BUF_SYNC_END (1 << 2)
#define DMA_BUF_SYNC_VALID_FLAGS_MASK (DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END)
#define DMA_BUF_BASE 'b'
#define DMA_BUF_IOCTL_SYNC _IOW(DMA_BUF_BASE, 0, struct dma_buf_sync)

    void MediaBuffer::BeginCPUAccess(bool readonly)
    {
        struct dma_buf_sync sync = {0};

        if (fd < 0) {
            return;
        }

        if (readonly) {
            sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
        } else {
            sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_START;
        }

        int ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
        if (ret < 0) {
            LOG("%s: %s\n", __func__, strerror(errno));
        }
    }

    void MediaBuffer::EndCPUAccess(bool readonly)
    {
        struct dma_buf_sync sync = {0};

        if (fd < 0) {
            return;
        }

        if (readonly) {
            sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
        } else {
            sync.flags = DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END;
        }

        int ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
        if (ret < 0) {
            LOG("%s: %s\n", __func__, strerror(errno));
        }
    }

    MediaGroupBuffer* MediaGroupBuffer::Alloc(size_t size, MediaBuffer::MemType type)
    {
        switch (type) {
            case MediaBuffer::MemType::MEM_COMMON:
                return alloc_common_memory_group(size);
#ifdef LIBDRM
            case MediaBuffer::MemType::MEM_HARD_WARE:
                return alloc_drm_memory_group(size);
#endif
            default:
                LOG("unknown memtype\n");
                return nullptr;
        }
    }

    BufferPool::BufferPool(int cnt, int size, MediaBuffer::MemType type)
    {
        bool sucess = true;

        if (cnt <= 0) {
            LOG("ERROR: BufferPool: cnt:%d is invalid!\n", cnt);
            return;
        }

        for (int i = 0; i < cnt; i++) {
            auto mgb = MediaGroupBuffer::Alloc(size, type);
            if (!mgb) {
                sucess = false;
                break;
            }
            mgb->SetBufferPool(this);
            LOGD("Create: pool:%p, mgb:%p, ptr:%p, fd:%d, size:%zu\n", this, mgb, mgb->GetPtr(), mgb->GetFD(),
                 mgb->GetSize());
            ready_buffers.push_back(mgb);
        }

        if (!sucess) {
            while (ready_buffers.size() > 0) {
                ready_buffers.pop_front();
            }
            LOG("ERROR: BufferPool: Create buffer pool failed! Please check space is "
                "enough!\n");
            return;
        }
        buf_cnt = cnt;
        buf_size = size;
        LOGD("BufferPool: Create buffer pool:%p, size:%d, cnt:%d\n", this, size, cnt);
    }

    BufferPool::~BufferPool()
    {
        int cnt = 0;
        int wait_times = 30;

        while (busy_buffers.size() > 0) {
            if (wait_times-- <= 0) {
                LOG("ERROR: BufferPool: waiting bufferpool free for 900ms, TimeOut!\n");
                break;
            }
            easymedia::usleep(30000); // wait 30ms
        }

        MediaGroupBuffer* mgb = NULL;
        while (ready_buffers.size() > 0) {
            mgb = ready_buffers.front();
            ready_buffers.pop_front();
            LOGD("BufferPool: #%02d Destroy buffer pool(ready):[%p,%p]\n", cnt, this, mgb);
            delete mgb;
            cnt++;
        }

        while (busy_buffers.size() > 0) {
            mgb = busy_buffers.front();
            busy_buffers.pop_front();
            LOG("WARN: BufferPool: #%02d Destroy buffer pool(busy):[%p,%p]\n", cnt, this, mgb);
            delete mgb;
            cnt++;
        }
    }

    static int __groupe_buffer_free(void* data)
    {
        assert(data);
        if (data == NULL) {
            LOG("ERROR: BufferPool: free ptr is null!\n");
            return 0;
        }
        MediaGroupBuffer* mgb = (MediaGroupBuffer*)data;
        if (mgb->pool == NULL) {
            LOG("ERROR: BufferPool: free pool ptr is null!\n");
            return 0;
        }

        BufferPool* bp = (BufferPool*)mgb->pool;

        return bp->PutBuffer(mgb);
    }

    std::shared_ptr<MediaBuffer> BufferPool::GetBuffer(bool block)
    {
        AutoLockMutex _alm(mtx);

        while (1) {
            if (!ready_buffers.size()) {
                if (block) {
                    mtx.wait();
                } else {
                    return nullptr;
                }
            }
            // mtx.notify wake up all mtx.wait.
            if (ready_buffers.size() > 0) {
                break;
            }
        }

        auto mgb = ready_buffers.front();
        ready_buffers.pop_front();
        busy_buffers.push_back(mgb);

        auto&& mb =
            std::make_shared<MediaBuffer>(mgb->GetPtr(), mgb->GetSize(), mgb->GetFD(), mgb, __groupe_buffer_free);
        return mb;
    }

    int BufferPool::PutBuffer(MediaGroupBuffer* mgb)
    {
        std::list<MediaGroupBuffer*>::iterator it;
        bool sucess = false;
        AutoLockMutex _alm(mtx);

        for (it = busy_buffers.begin(); it != busy_buffers.end();) {
            if (*it == mgb) {
                sucess = true;
                it = busy_buffers.erase(it);
                ready_buffers.push_back(mgb);
                mtx.notify();
                break;
            }
            it++;
        }

        if (!sucess) {
            LOG("ERROR: BufferPool: Unknow media group buffer:%p\n", mgb);
        }

        return sucess ? 0 : -1;
    }

    void BufferPool::DumpInfo()
    {
        int id = 0;
        LOG("##BufferPool DumpInfo:%p\n", this);
        LOG("\tcnt:%d\n", buf_cnt);
        LOG("\tsize:%zu\n", buf_size);
        LOG("\tready buffers(%d):\n", ready_buffers.size());
        for (auto dev : ready_buffers)
            LOG("\t  #%02d Pool:%p, mgb:%p, ptr:%p\n", id++, dev->pool, dev, dev->GetPtr());
        LOG("\tbusy buffers(%d):\n", busy_buffers.size());
        id = 0;
        for (auto dev : busy_buffers)
            LOG("\t  #%02d Pool:%p, mgb:%p, ptr:%p\n", id++, dev->pool, dev, dev->GetPtr());
    }

} // namespace easymedia

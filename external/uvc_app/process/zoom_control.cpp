#include "eptz_control.h"
#include "camera_control.h"
#include <pthread.h>
#include <pwd.h>
#include "uvc_log.h"

static RockchipRga gRkRga_zoom;

static int get_rga_format_zoom(PixelFormat f) {
  static std::map<PixelFormat, int> rga_format_map = {
      {PIX_FMT_YUV420P, RK_FORMAT_YCbCr_420_P},
      {PIX_FMT_NV12, RK_FORMAT_YCbCr_420_SP},
      {PIX_FMT_NV21, RK_FORMAT_YCrCb_420_SP},
      {PIX_FMT_YUV422P, RK_FORMAT_YCbCr_422_P},
      {PIX_FMT_NV16, RK_FORMAT_YCbCr_422_SP},
      {PIX_FMT_NV61, RK_FORMAT_YCrCb_422_SP},
      {PIX_FMT_YUYV422, -1},
      {PIX_FMT_UYVY422, -1},
      {PIX_FMT_RGB565, RK_FORMAT_RGB_565},
      {PIX_FMT_BGR565, -1},
      {PIX_FMT_RGB888, RK_FORMAT_BGR_888},
      {PIX_FMT_BGR888, RK_FORMAT_RGB_888},
      {PIX_FMT_ARGB8888, RK_FORMAT_BGRA_8888},
      {PIX_FMT_ABGR8888, RK_FORMAT_RGBA_8888}};
  auto it = rga_format_map.find(f);
  if (it != rga_format_map.end())
    return it->second;
  return -1;
}

static int rga_blit_zoom(std::shared_ptr<easymedia::ImageBuffer> src,
                    std::shared_ptr<easymedia::ImageBuffer> dst,
                    ImageRect *src_rect, ImageRect *dst_rect, int rotate) {
  if (!src)
    return -EINVAL;
  if (!dst)
    return -EINVAL;
  rga_info_t src_info, dst_info;
  memset(&src_info, 0, sizeof(src_info));
  src_info.fd = src->GetFD();
  if (src_info.fd < 0)
    src_info.virAddr = src->GetPtr();
  src_info.mmuFlag = 1;
  src_info.rotation = rotate;
  if (src_rect) {
    rga_set_rect(&src_info.rect, src_rect->x, src_rect->y, src_rect->w,
                 src_rect->h, src->GetVirWidth(), src->GetVirHeight(),
                 get_rga_format_zoom(src->GetPixelFormat()));
  } else {
    LOG_INFO("%s %d src_rect error \n", __FUNCTION__, __LINE__);
  }
  memset(&dst_info, 0, sizeof(dst_info));
  dst_info.fd = dst->GetFD();
  if (dst_info.fd < 0)
    dst_info.virAddr = dst->GetPtr();
  dst_info.mmuFlag = 1;
  if (dst_rect) {
    rga_set_rect(&dst_info.rect, dst_rect->x, dst_rect->y, dst_rect->w,
                 dst_rect->h, dst->GetVirWidth(), dst->GetVirHeight(),
                 get_rga_format_zoom(dst->GetPixelFormat()));
  } else {
    LOG_INFO("%s %d dst_rect error\n", __FUNCTION__, __LINE__);
  }

  int ret = gRkRga_zoom.RkRgaBlit(&src_info, &dst_info, NULL);
  if (ret) {
    dst->SetValidSize(0);
    LOG("Fail to RkRgaBlit, ret=%d\n", ret);
  } else {
    size_t valid_size = CalPixFmtSize(dst->GetPixelFormat(), dst->GetVirWidth(),
                                      dst->GetVirHeight(), 16);
    dst->SetValidSize(valid_size);
    if (src->GetUSTimeStamp() > dst->GetUSTimeStamp())
      dst->SetUSTimeStamp(src->GetUSTimeStamp());
  }
  return ret;
}

static float dynamic_zoom = 1;
static pthread_rwlock_t zoomlock = PTHREAD_RWLOCK_INITIALIZER;

bool do_zoom(easymedia::Flow *f,
                     easymedia::MediaBufferVector &input_vector) {
  auto flow = static_cast<ZoomFlow *>(f);
  auto input = input_vector[0];
  if (!input)
    return false;
  auto src = std::static_pointer_cast<easymedia::ImageBuffer>(input);
  if(dynamic_zoom == 1)
     return flow->SetOutput(src, 0);
  // fmt,w,h,vw,vh
  ImageInfo out_img_info = {PIX_FMT_NV12, flow->dst_width, flow->dst_height,
                            flow->dst_width, flow->dst_height};
  const auto &info = out_img_info;

  std::shared_ptr<easymedia::MediaBuffer> out_buffer;
  if (info.vir_width > 0 && info.vir_height > 0) {
    size_t size = CalPixFmtSize(out_img_info.pix_fmt, out_img_info.vir_width,
                                out_img_info.vir_height, 16);
    // size_t size = CalPixFmtSize(info);
    auto &&mb = easymedia::MediaBuffer::Alloc2(
        size, easymedia::MediaBuffer::MemType::MEM_HARD_WARE);
    out_buffer = std::make_shared<easymedia::ImageBuffer>(mb, info);
  }

  auto dst = std::static_pointer_cast<easymedia::ImageBuffer>(out_buffer);
  ImageRect *src_rect = (ImageRect *)malloc(sizeof(ImageRect));
  ImageRect *dst_rect = (ImageRect *)malloc(sizeof(ImageRect));

  if (!src_rect || !dst_rect) {
    LOG_INFO( "%s %d error \n");
    return false;
  } else {
    pthread_rwlock_wrlock(&zoomlock);
    src_rect->x = ceil((flow->dst_width - (float)(flow->dst_width/dynamic_zoom))/2 - 0.001);
    src_rect->y = ceil((flow->dst_height - (float)(flow->dst_height/dynamic_zoom))/2 - 0.001);
    src_rect->w = ceil(((float)flow->dst_width/dynamic_zoom) - 0.01);
    src_rect->h = ceil(((float)flow->dst_height/dynamic_zoom) - 0.01);
    if (src_rect->x % 2 != 0)
      src_rect->x = src_rect->x - 1;
    if (src_rect->y % 2 != 0)
      src_rect->y = src_rect->y - 1;
    if (src_rect->w % 2 != 0)
      src_rect->w = src_rect->w - 1;
    if (src_rect->h % 2 != 0)
      src_rect->h = src_rect->h - 1;
    dst_rect->x = 0;
    dst_rect->y = 0;
    dst_rect->w = flow->dst_width;
    dst_rect->h = flow->dst_height;
    pthread_rwlock_unlock(&zoomlock);
  }
  rga_blit_zoom(src, dst, src_rect, dst_rect, 0);
out:
  if (src_rect)
    free(src_rect);
  src_rect = nullptr;
  if (dst_rect)
    free(dst_rect);
  dst_rect = nullptr;
  return flow->SetOutput(dst, 0);
}

ZoomFlow::ZoomFlow(uint32_t dst_w, uint32_t dst_h)
    : dst_width(dst_w), dst_height(dst_h) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(1);
  sm.output_slots.push_back(0);
  sm.process = do_zoom;
  if (!InstallSlotMap(sm, "zoom", -1)) {
    LOG_INFO( "Fail to InstallSlotMap, %s\n", "zoom");
    SetError(-EINVAL);
    return;
  }
}


std::shared_ptr<ZoomFlow> zoom = nullptr;

int zoom_config( int stream_width, int stream_height) {
  LOG_INFO("%s: enter \n", __FUNCTION__);
  // zoom
  zoom = std::make_shared<ZoomFlow>(stream_width,stream_height);
  if (!zoom || zoom->GetError()) {
    LOG_INFO( "Fail to create zoom flow\n");
    return -1;
  }
  return 0;
}

int set_zoom(float val) {
  LOG_INFO("zoom_control.cpp: set zoom:%2f \n",val);
  dynamic_zoom = val;
  return 0;
}

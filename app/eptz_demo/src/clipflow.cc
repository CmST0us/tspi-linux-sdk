#include "clipflow.h"

namespace NPU_UVC_CLIP_FLOW {

static RockchipRga gRkRga;

static int get_rga_format(PixelFormat f) {
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

static int rga_blit(std::shared_ptr<easymedia::ImageBuffer> src,
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
                 get_rga_format(src->GetPixelFormat()));
  } else {
    printf("%s %d src_rect error \n", __FUNCTION__, __LINE__);
  }
  memset(&dst_info, 0, sizeof(dst_info));
  dst_info.fd = dst->GetFD();
  if (dst_info.fd < 0)
    dst_info.virAddr = dst->GetPtr();
  dst_info.mmuFlag = 1;
  if (dst_rect) {
    rga_set_rect(&dst_info.rect, dst_rect->x, dst_rect->y, dst_rect->w,
                 dst_rect->h, dst->GetVirWidth(), dst->GetVirHeight(),
                 get_rga_format(dst->GetPixelFormat()));
  } else {
    printf("%s %d dst_rect error\n", __FUNCTION__, __LINE__);
  }

  int ret = gRkRga.RkRgaBlit(&src_info, &dst_info, NULL);
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

DynamicClipFlow::DynamicClipFlow(uint32_t dst_w, uint32_t dst_h)
    : dst_width(dst_w), dst_height(dst_h), isXMoving(false), isYMoving(false),
      isAmplify(false), isShrink(false) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(1);
  sm.output_slots.push_back(0);
  sm.process = do_dynamic_clip;
  if (!InstallSlotMap(sm, "dynamic_clip", -1)) {
    fprintf(stderr, "Fail to InstallSlotMap, %s\n", "sdl_draw");
    SetError(-EINVAL);
    return;
  }
}

DynamicClipFlow::~DynamicClipFlow() {
  StopAllThread();
  fprintf(stderr, "dynamic clip flow quit\n");
}

static float iterate_step_x = 0;
static float iterate_step_y = 0;
static float iterate_step_w = 0;
static float iterate_step_h = 0;

static bool do_dynamic_clip(easymedia::Flow *f,
                            easymedia::MediaBufferVector &input_vector) {
  auto flow = static_cast<DynamicClipFlow *>(f);
  auto input = input_vector[0];
  if (!input)
    return false;
  auto src = std::static_pointer_cast<easymedia::ImageBuffer>(input);
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
  LOGD("%s %d, arrayXY [%.2f %.2f %.2f %.2f]  lastXY [%.2f %.2f %.2f %.2f] "
       "tempXY[%.2f %.2f %.2f %.2f]\n",
       __FUNCTION__, __LINE__, arrayXY[0], arrayXY[1], arrayXY[2], arrayXY[3],
       lastXY[0], lastXY[1], lastXY[2], lastXY[3], tempXY[0], tempXY[1],
       tempXY[2], tempXY[3]);

  //对本次计算结果，与上次坐标点进行比对,计算clip wh
  if (current_focus_state != last_focus_state) {
    if (current_focus_state) {
      flow->isShrink = true;
      iterate_step_x = (abs(arrayXY[0] - lastXY[0]) / STEP);
      iterate_step_y = (abs(arrayXY[1] - lastXY[1]) / STEP);
      iterate_step_w = (abs(arrayXY[2] - lastXY[2]) / STEP);
      iterate_step_h = (abs(arrayXY[3] - lastXY[3]) / STEP);
    } else {
      flow->isAmplify = true;
      iterate_step_x = (abs(arrayXY[0] - lastXY[0]) / STEP);
      iterate_step_y = (abs(arrayXY[1] - lastXY[1]) / STEP);
      iterate_step_w = (abs(arrayXY[2] - lastXY[2]) / STEP);
      iterate_step_h = (abs(arrayXY[3] - lastXY[3]) / STEP);
    }
    if (iterate_step_x == 0 && arrayXY[0] != lastXY[0])
      iterate_step_x = 1.0;
    if (iterate_step_y == 0 && arrayXY[1] != lastXY[1])
      iterate_step_y = 1.0;
    tempXY[0] = arrayXY[0];
    tempXY[1] = arrayXY[1];
    tempXY[2] = arrayXY[2];
    tempXY[3] = arrayXY[3];
  }
  LOGD("%s %d state isShrink %d, isAmplify %d, isXMoving %d, isYMoving %d , "
       "step_x  %.2f, step_y %.2f, step_w  %.2f, step_h %.2f \n",
       __FUNCTION__, __LINE__, flow->isShrink, flow->isAmplify, flow->isXMoving,
       flow->isYMoving, iterate_step_x, iterate_step_y, iterate_step_w,
       iterate_step_h);
  if (flow->isShrink || flow->isAmplify) {
    if (tempXY[2] != lastXY[2]) {
      lastXY[2] = tempXY[2] > lastXY[2] ? iterate_step_w + lastXY[2]
                                        : lastXY[2] - iterate_step_w;
    }
    if (tempXY[0] != lastXY[0])
      lastXY[0] = tempXY[0] > lastXY[0] ? iterate_step_x + lastXY[0]
                                        : lastXY[0] - iterate_step_x;
    lastXY[0] = lastXY[0] < 0 ? 0 : lastXY[0];
    if (lastXY[0] + lastXY[2] > SENSOR_REVOLUSION_W)
      lastXY[0] = SENSOR_REVOLUSION_W - lastXY[2];

    if (tempXY[3] != lastXY[3]) {
      lastXY[3] = tempXY[3] > lastXY[3] ? iterate_step_h + lastXY[3]
                                        : lastXY[3] - iterate_step_h;
    }
    if (tempXY[1] != lastXY[1])
      lastXY[1] = tempXY[1] > lastXY[1] ? iterate_step_y + lastXY[1]
                                        : lastXY[1] - iterate_step_y;
    lastXY[1] = lastXY[1] < 0 ? 0 : lastXY[1];
    if (lastXY[1] + lastXY[3] > SENSOR_REVOLUSION_H)
      lastXY[1] = SENSOR_REVOLUSION_H - lastXY[3];
  }

  if (!(flow->isShrink) && !(flow->isAmplify)) {

    // if (!flow->isShrink && !flow->isAmplify){
    //对本次计算结果，与上次坐标点进行比对,确保能够移动到最边缘处
    if (abs(arrayXY[0] - lastXY[0]) >= THRESHOLD_X ||
        int(ceil(arrayXY[0])) % (THRESHOLD_X * 2) == 0) {
      flow->isXMoving = true;
      tempXY[0] = arrayXY[0];
    }

    if (abs(arrayXY[1] - lastXY[1]) >= THRESHOLD_Y ||
        int(ceil(arrayXY[1])) % (THRESHOLD_Y * 2) == 0) {
      flow->isYMoving = true;
      tempXY[1] = arrayXY[1];
    }

    if (flow->isXMoving && tempXY[0] != lastXY[0]) {
      for (int i = 0; i < iterate_step_x; i++) {
        if (tempXY[0] != lastXY[0]) {
          lastXY[0] = tempXY[0] > lastXY[0] ? ++lastXY[0] : --lastXY[0];
        } else {
          break;
        }
      }
    } else {
      flow->isXMoving = false;
    }

    if (flow->isYMoving && tempXY[1] != lastXY[1]) {
      for (int i = 0; i < iterate_step_y; i++) {
        if (tempXY[1] != lastXY[1]) {
          lastXY[1] = tempXY[1] > lastXY[1] ? ++lastXY[1] : --lastXY[1];
        } else {
          break;
        }
      }
    } else {
      flow->isYMoving = false;
    }
  }

  if (fabs(tempXY[2] - lastXY[2]) < 0.1 && fabs(tempXY[3] - lastXY[3]) < 0.1) {
    flow->isShrink = false;
    flow->isAmplify = false;
    iterate_step_x = ITERATE_X;
    iterate_step_y = ITERATE_Y;
  }

  if (!src_rect || !dst_rect) {
    fprintf(stderr, "%s %d error \n", __FUNCTION__, __LINE__);
    return false;
  } else {
    if (ceil(lastXY[0]) + ceil(lastXY[2]) <= SENSOR_REVOLUSION_W &&
        ceil(lastXY[1]) + ceil(lastXY[3]) <= SENSOR_REVOLUSION_H &&
        lastXY[0] >= 0 && lastXY[1] >= 0) {
      LOGD("%s %d src_rec[%.2f %.2f %.2f %.2f] \n", __FUNCTION__, __LINE__,
           lastXY[0], lastXY[1], lastXY[2], lastXY[3]);
    } else {
      LOGD("%s %d ************************ src_rec[%.2f %.2f %.2f %.2f] \n",
           __FUNCTION__, __LINE__, lastXY[0], lastXY[1], lastXY[2], lastXY[3]);
      if (lastXY[0] < 0.01)
        lastXY[0] = 0;
      if (lastXY[1] < 0.01)
        lastXY[1] = 0;
      if (ceil(lastXY[0]) + ceil(lastXY[2]) > SENSOR_REVOLUSION_W)
        lastXY[0] = SENSOR_REVOLUSION_W - ceil(lastXY[2]) - 0.01;
      if (ceil(lastXY[1]) + ceil(lastXY[3]) > SENSOR_REVOLUSION_H)
        lastXY[1] = SENSOR_REVOLUSION_H - ceil(lastXY[3]) - 0.01;
      if (ceil(lastXY[0]) + ceil(lastXY[2]) > SENSOR_REVOLUSION_W)
        lastXY[2] = SENSOR_REVOLUSION_W - ceil(lastXY[0]) - 0.01;
      if (ceil(lastXY[1]) + ceil(lastXY[3]) > SENSOR_REVOLUSION_H)
        lastXY[3] = SENSOR_REVOLUSION_H - ceil(lastXY[1]) - 0.01;
      // exit(0);
    }
    src_rect->x = ceil(lastXY[0] - 0.001);
    src_rect->y = ceil(lastXY[1] - 0.001);
    src_rect->w = ceil(lastXY[2] - 0.01);
    src_rect->h = ceil(lastXY[3] - 0.01);
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
  }
#if 0
  assert(dst->IsValid());
  if (!dst->IsValid()) {
    // the same to src
    ImageInfo info = src->GetImageInfo();
    info.pix_fmt = dst->GetPixelFormat();
    //size RGB888 vw*vh*3
    size_t size = CalPixFmtSize(info.pix_fmt, info.vir_width,
                                info.vir_height, 16);//1280 * 720 *3;
    fprintf(stderr, "%s %d imgaeInfo w h vw vh [%d %d %d %d] fmt %d , dst size [%ld]\n",__FUNCTION__, __LINE__,
                    info.width,info.height,info.vir_width,info.vir_height,
                    get_rga_format(info.pix_fmt),
                    size);
    if (size == 0)
      return -EINVAL;
    auto &&mb = easymedia::MediaBuffer::Alloc2(size, easymedia::MediaBuffer::MemType::MEM_HARD_WARE);
    easymedia::ImageBuffer ib(mb, info);
    if (ib.GetSize() >= size) {
      ib.SetValidSize(size);
      *dst.get() = ib;
    }
    assert(dst->IsValid());
  }
#endif
  rga_blit(src, dst, src_rect, dst_rect, 0);
out:
  if (src_rect)
    free(src_rect);
  src_rect = nullptr;
  if (dst_rect)
    free(dst_rect);
  dst_rect = nullptr;
  return flow->SetOutput(dst, 0);
}
}

#include "eptz_control.h"
#include "uvc_log.h"

static RockchipRga gRkRga;
struct eptz_frame_info eptz_info;

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
    LOG_ERROR("%s %d src_rect error \n", __FUNCTION__, __LINE__);
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
    LOG_ERROR("%s %d dst_rect error\n", __FUNCTION__, __LINE__);
  }

  int ret = gRkRga.RkRgaBlit(&src_info, &dst_info, NULL);
  if (ret) {
    dst->SetValidSize(0);
    LOG_ERROR("Fail to RkRgaBlit, ret=%d\n", ret);
  } else {
    size_t valid_size = CalPixFmtSize(dst->GetPixelFormat(), dst->GetVirWidth(),
                                      dst->GetVirHeight(), 16);
    dst->SetValidSize(valid_size);
    if (src->GetUSTimeStamp() > dst->GetUSTimeStamp())
      dst->SetUSTimeStamp(src->GetUSTimeStamp());
  }
  return ret;
}

static char *enable_skip_frame = nullptr;

bool do_rockx(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector) {

  if (enable_skip_frame) {
    int static count = 0;
    int static divisor = atoi(enable_skip_frame);
    if (count++ % divisor != 0)
      return -1;
    if (count == 10000)
      count = 0;
  }

  assert(sizeof(float) == 4);
  RockxFlow *flow = (RockxFlow *)f;
  auto input =
      std::static_pointer_cast<easymedia::ImageBuffer>(input_vector[0]);
  if (!input)
    return false;
  rockx_image_t input_image;
  input_image.width = input->GetWidth();
  input_image.height = input->GetHeight();
  input_image.data = (uint8_t *)input->GetPtr();
  input_image.pixel_format =
      ROCKX_PIXEL_FORMAT_YUV420SP_NV12; // ROCKX_PIXEL_FORMAT_RGB888;
  auto &handles = flow->rockx_handles;

  rockx_handle_t &face_det_handle = handles[0];
  rockx_handle_t &object_track_handle = handles[1];
  rockx_object_array_t face_array;
  rockx_object_array_t face_array_track;
  memset(&face_array, 0, sizeof(rockx_object_array_t));
  rockx_ret_t ret =
      rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
  if (ret != ROCKX_RET_SUCCESS) {
    LOG_ERROR("rockx_face_detect error %d\n", ret);
    return false;
  }
  if (face_array.count <= 0)
    return false;
  ret =
      rockx_object_track(object_track_handle, input_image.width,
                         input_image.height, 4, &face_array, &face_array_track);
  if (ret != ROCKX_RET_SUCCESS) {
    LOG_ERROR("rockx_face_detect error %d\n", ret);
    return false;
  }
  size_t ret_buf_size =
      face_array_track.count * sizeof(struct aligned_rockx_face_rect);
  auto ret_buf = easymedia::MediaBuffer::Alloc(ret_buf_size);
  if (!ret_buf)
    return false;
  auto face_rects = (struct aligned_rockx_face_rect *)ret_buf->GetPtr();
  memset(face_rects, 0, ret_buf_size);
  for (int i = 0; i < face_array_track.count; i++) {
    auto array = &face_rects[i];
    array->left = face_array_track.object[i].box.left;
    array->top = face_array_track.object[i].box.top;
    array->right = face_array_track.object[i].box.right;
    array->bottom = face_array_track.object[i].box.bottom;
    memcpy(array->score, &face_array_track.object[i].score, 4);
  }

  ret_buf->SetSize(ret_buf_size);
  ret_buf->SetValidSize(face_array_track.count);

  do {
    current_focus_state = count_rectXY(
        ret_buf, arrayXY, lastXY, eptz_info.src_width, eptz_info.src_height,
        eptz_info.dst_width, eptz_info.dst_height);
  } while (0);

  return true;
}

RockxFlow::RockxFlow() {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::SYNC;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(1);
  sm.fetch_block.push_back(true);
  sm.output_slots.push_back(0);
  // if (hand_over_big_pic)
  sm.hold_input.push_back(easymedia::HoldInputMode::INHERIT_FORM_INPUT);
  sm.process = do_rockx;
  if (!InstallSlotMap(sm, "rockx", -1)) {
    LOG_ERROR("Fail to InstallSlotMap, %s\n", "rockx");
    SetError(-EINVAL);
    return;
  }
  std::vector<rockx_module_t> models;
  void *config = nullptr;
  size_t config_size = 0;
  models.push_back(ROCKX_MODULE_FACE_DETECTION_V3);
  models.push_back(ROCKX_MODULE_OBJECT_TRACK);

  for (size_t i = 0; i < models.size(); i++) {
    rockx_handle_t npu_handle = nullptr;
    rockx_module_t &model = models[i];
    rockx_ret_t ret = rockx_create(&npu_handle, model, config, config_size);
    if (ret != ROCKX_RET_SUCCESS) {
      LOG_ERROR("init rockx module %d error=%d\n", model, ret);
      SetError(-EINVAL);
      return;
    }
    rockx_handles.push_back(npu_handle);
  }
  enable_skip_frame = getenv("ENABLE_EPTZ_SKIP_FRAME");
}

bool do_dynamic_clip(easymedia::Flow *f,
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
  if (!src_rect || !dst_rect)
    return false;
  output_result(src_rect, dst_rect);
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

DynamicClipFlow::DynamicClipFlow(uint32_t dst_w, uint32_t dst_h)
    : dst_width(dst_w), dst_height(dst_h) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(1);
  sm.output_slots.push_back(0);
  sm.process = do_dynamic_clip;
  if (!InstallSlotMap(sm, "dynamic_clip", -1)) {
    LOG_ERROR("Fail to InstallSlotMap, %s\n", "dynamic_clip");
    SetError(-EINVAL);
    return;
  }
}

std::shared_ptr<easymedia::Flow> eptz_source = nullptr;
std::shared_ptr<easymedia::Flow> rknn = nullptr;
std::shared_ptr<DynamicClipFlow> dclip = nullptr;

int eptz_config(int stream_width, int stream_height, int eptz_width,
                int eptz_height) {
  eptz_info.src_width = stream_width;
  eptz_info.src_height = stream_height;
  eptz_info.dst_width = eptz_width;
  eptz_info.dst_height = eptz_height;
  eptz_info.threshold_x = (eptz_info.src_width - eptz_info.dst_width) / 4;
  eptz_info.threshold_y = (eptz_info.src_height - eptz_info.dst_height) / 4;
  if (eptz_info.dst_width == 1920) {
    eptz_info.iterate_x = 6;
    eptz_info.iterate_y = 3;
  } else if (eptz_info.dst_width == 1280) {
    eptz_info.iterate_x = 4;
    eptz_info.iterate_y = 2;
  } else {
    eptz_info.iterate_x = 2;
    eptz_info.iterate_y = 2;
  }
  LOG_INFO("%s: eptz_info src_wh [%d %d] dst_wh[%d %d], threshold_xy[%d %d] "
           "iterate_xy[%d %d]\n",
           __func__, eptz_info.src_width, eptz_info.src_height,
           eptz_info.dst_width, eptz_info.dst_height, eptz_info.threshold_x,
           eptz_info.threshold_y, eptz_info.iterate_x, eptz_info.iterate_y);

  tempXY =
      new float[4]{(float)(eptz_info.src_width - eptz_info.dst_width) / 2,
                   (float)(eptz_info.src_height - eptz_info.dst_height) / 2,
                   (float)(eptz_info.dst_width), (float)(eptz_info.dst_height)};
  arrayXY =
      new float[4]{(float)(eptz_info.src_width - eptz_info.dst_width) / 2,
                   (float)(eptz_info.src_height - eptz_info.dst_height) / 2,
                   (float)(eptz_info.dst_width), (float)(eptz_info.dst_height)};
  lastXY =
      new float[4]{(float)(eptz_info.src_width - eptz_info.dst_width) / 2,
                   (float)(eptz_info.src_height - eptz_info.dst_height) / 2,
                   (float)(eptz_info.dst_width), (float)(eptz_info.dst_height)};

  do {
    std::string flow_name("source_stream");
    std::string flow_param("");

    PARAM_STRING_APPEND(flow_param, KEY_NAME, "v4l2_capture_stream");
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_SYNC);
    PARAM_STRING_APPEND(flow_param, KEK_INPUT_MODEL, KEY_DROPFRONT);
    PARAM_STRING_APPEND_TO(flow_param, KEY_INPUT_CACHE_NUM, 5);
    std::string stream_param = "";
    PARAM_STRING_APPEND_TO(stream_param, KEY_USE_LIBV4L2, 1);
    PARAM_STRING_APPEND(stream_param, KEY_DEVICE, "rkispp_scale2");
    // PARAM_STRING_APPEND(param, KEY_SUB_DEVICE, sub_input_path);
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_CAP_TYPE,
                        KEY_V4L2_C_TYPE(VIDEO_CAPTURE));
    PARAM_STRING_APPEND(stream_param, KEY_V4L2_MEM_TYPE,
                        KEY_V4L2_M_TYPE(MEMORY_DMABUF));
    PARAM_STRING_APPEND_TO(stream_param, KEY_FRAMES,
                           3); // if not set, default is 2
    PARAM_STRING_APPEND(stream_param, KEY_OUTPUTDATATYPE, "image:nv12");
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_WIDTH, 640);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_HEIGHT, 480);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_WIDTH, 640);
    PARAM_STRING_APPEND_TO(stream_param, KEY_BUFFER_VIR_HEIGHT, 480);

    flow_param = easymedia::JoinFlowParam(flow_param, 1, stream_param);
    LOG_INFO("\n#VideoCapture flow param:\n%s\n", flow_param.c_str());
    eptz_source = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
        flow_name.c_str(), flow_param.c_str());
    if (!eptz_source) {
      LOG_ERROR("Create flow %s failed\n", flow_name.c_str());
      return -1;
    }
  } while (0);

  rknn = std::make_shared<RockxFlow>();
  if (!rknn || rknn->GetError()) {
    LOG_ERROR("Fail to create rockx flow\n");
    return -1;
  }

  // dynamic_clip
  dclip = std::make_shared<DynamicClipFlow>(eptz_info.dst_width,
                                            eptz_info.dst_height);
  if (!dclip || dclip->GetError()) {
    LOG_ERROR("Fail to create dynamic_clip flow\n");
    return -1;
  }
  return 0;
}

std::shared_ptr<easymedia::Flow> create_flow(const std::string &flow_name,
                                             const std::string &flow_param,
                                             const std::string &elem_param) {
  auto &&param = easymedia::JoinFlowParam(flow_param, 1, elem_param);
  auto ret = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
      flow_name.c_str(), param.c_str());
  if (!ret)
    LOG_ERROR("Create flow %s failed\n", flow_name.c_str());
  return ret;
}

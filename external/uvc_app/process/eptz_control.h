#ifndef EPTZ_CONTROL_
#define EPTZ_CONTROL_

#define _API __attribute__((visibility("default")))

#include <rga/RockchipRga.h>
#if EPTZ_ENABLE
#include <rockx/rockx.h>
#endif
#include <easymedia/buffer.h>
#include <easymedia/flow.h>
#include <easymedia/image.h>
#include <easymedia/key_string.h>
#include <easymedia/media_config.h>
#include <easymedia/utils.h>
#include <math.h>
#include <memory>

extern struct eptz_frame_info eptz_info;
extern float *tempXY;
extern float *arrayXY;
extern float *lastXY;
extern bool last_focus_state;
extern bool current_focus_state;

struct eptz_frame_info {
  int src_width;
  int src_height;
  int dst_width;
  int dst_height;
  int threshold_x;
  int threshold_y;
  int iterate_x;
  int iterate_y;
};

// rockx output
struct aligned_rockx_face_rect {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  uint8_t score[4]; // assert(sizeof(float) == 4);
} __attribute__((packed));

std::shared_ptr<easymedia::Flow> create_flow(const std::string &flow_name,
                                             const std::string &flow_param,
                                             const std::string &elem_param);

class DynamicClipFlow : public easymedia::Flow {
public:
  DynamicClipFlow(uint32_t dst_w, uint32_t dst_h);
  virtual ~DynamicClipFlow() {
    StopAllThread();
    fprintf(stderr, "~dynamic clip flow quit\n");
  }
  int dst_width;
  int dst_height;
  friend bool do_dynamic_clip(easymedia::Flow *f,
                              easymedia::MediaBufferVector &input_vector);
};

bool do_dynamic_clip(easymedia::Flow *f,
                     easymedia::MediaBufferVector &input_vector);

class RockxFlow : public easymedia::Flow {
public:
  RockxFlow();
  virtual ~RockxFlow() {
    StopAllThread();
    for (auto handle : rockx_handles) {
      rockx_destroy(handle);
    }
    fprintf(stderr, "~rockx flow quit\n");
  }

private:
  std::vector<rockx_handle_t> rockx_handles;
  friend bool do_rockx(easymedia::Flow *f,
                       easymedia::MediaBufferVector &input_vector);
};

bool do_rockx(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector);

extern std::shared_ptr<easymedia::Flow> eptz_source;
extern std::shared_ptr<easymedia::Flow> rknn;
extern std::shared_ptr<DynamicClipFlow> dclip;

// zoom
int zoom_config(int stream_width, int stream_height);
int set_zoom(float val);

class ZoomFlow : public easymedia::Flow {
public:
  ZoomFlow(uint32_t dst_w, uint32_t dst_h);
  virtual ~ZoomFlow() {
    StopAllThread();
    fprintf(stderr, "~ZoomFlow flow quit\n");
  }
  int dst_width;
  int dst_height;
  friend bool do_zoom(easymedia::Flow *f,
                      easymedia::MediaBufferVector &input_vector);
};

bool do_zoom(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector);

extern std::shared_ptr<ZoomFlow> zoom;
int eptz_config(int stream_width, int stream_height, int eptz_width,
                int eptz_height);
int get_env(const char *name, int *value, int default_value);
bool count_rectXY(std::shared_ptr<easymedia::MediaBuffer> output,
                  float *resultArray, float *lastXY, int src_w, int src_h,
                  int clip_w, int clip_h);
void output_result(ImageRect *src_rect, ImageRect *dst_rect);

#endif

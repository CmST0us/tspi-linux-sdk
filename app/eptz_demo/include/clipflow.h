#ifndef NPU_UVC_CLIP_FLOW_
#define NPU_UVC_CLIP_FLOW_

#include "globle.h"

#include <rga/RockchipRga.h>
#include <math.h>

#include <easymedia/flow.h>
#include <easymedia/buffer.h>
#include <easymedia/key_string.h>
#include <easymedia/media_config.h>
#include <easymedia/utils.h>
#include <easymedia/image.h>
namespace NPU_UVC_CLIP_FLOW {
    
class DynamicClipFlow : public easymedia::Flow {
public:
  DynamicClipFlow(uint32_t dst_w, uint32_t dst_h);
  virtual ~DynamicClipFlow(); 
  int dst_width;
  int dst_height;
  bool isXMoving;
  bool isYMoving;
  bool isAmplify;
  bool isShrink;
  friend bool do_dynamic_clip(easymedia::Flow *f,
                              easymedia::MediaBufferVector &input_vector);
};

static bool do_dynamic_clip(easymedia::Flow *f,
                            easymedia::MediaBufferVector &input_vector);
}
#endif

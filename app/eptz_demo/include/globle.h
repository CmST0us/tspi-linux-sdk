#ifndef NPU_UVC_GLOBLE_
#define NPU_UVC_GLOBLE_

#define NDEBUG
#define _API __attribute__((visibility("default")))

#ifndef NDEBUG
_API void LOGD(const char *format, ...);
#else
#define LOGD(...)
#endif

#if 1
#define SENSOR_REVOLUSION_W 2560
#define SENSOR_REVOLUSION_H 1440

#define DCLIP_REVOLUSION_W 1920
#define DCLIP_REVOLUSION_H 1080
#define DCLIP_REVOLUSION_VIR_W 1920
#define DCLIP_REVOLUSION_VIR_H 1088

#define THRESHOLD_X 192
#define THRESHOLD_Y 110
#define ITERATE_X 8
#define ITERATE_Y 4
#else
#define SENSOR_REVOLUSION_W 1920
#define SENSOR_REVOLUSION_H 1080

#define DCLIP_REVOLUSION_W 1280
#define DCLIP_REVOLUSION_H 720
#define DCLIP_REVOLUSION_VIR_W 1280
#define DCLIP_REVOLUSION_VIR_H 720

#define THRESHOLD_X 160
#define THRESHOLD_Y 90
#define ITERATE_X 8
#define ITERATE_Y 4
#endif


//scale ITERATE_W = ITERATE_H x 320 / 180
#define STEP 60.0f

#if 1
#define FACEDETECT_SCROE_THRESHOLD 0.75f
#define FAST_MOVE_FRMAE_JUDGE 10
#define PERSON_FRMAE_JUDGE_SINGLE 10
#define PERSON_FRMAE_JUDGE_MULTI 2
#else
#define FACEDETECT_SCROE_THRESHOLD 0.65f
#define FAST_MOVE_FRMAE_JUDGE 20
#define PERSON_FRMAE_JUDGE_SINGLE 30
#define PERSON_FRMAE_JUDGE_MULTI 5
#endif

extern float* tempXY;
extern float* arrayXY;
extern float* lastXY;
extern bool last_focus_state;
extern bool current_focus_state;
extern int frame_count;


#endif

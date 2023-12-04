#ifndef __RKFACIAL_H__
#define __RKFACIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <rockface/rockface.h>

enum aiq_control_type {
    AIQ_CONTROL_RGB,
    AIQ_CONTROL_IR,
};

enum user_state {
    USER_STATE_FAKE,
    USER_STATE_REAL_UNREGISTERED,
    USER_STATE_REAL_REGISTERED_WHITE,
    USER_STATE_REAL_REGISTERED_BLACK,
    USER_STATE_REAL_IDENTITY,
};

struct user_info {
    char sIdentityPath[256];
    char has_mask;
    unsigned int id;
    char sPicturePath[256];
    char sRegistrationTime[32];
    unsigned int iAge;
    char sListType[32];
    char sType[32];
    char sName[256];
    char sGender[32];
    char sNation[32];
    char sCertificateType[32];
    char sCertificateNumber[32];
    char sBirthday[32];
    char sTelephoneNumber[32];
    char sHometown[32];
    char sAddress[32];
    unsigned int iAccessCardNumber;
    enum user_state state;
    char snap_path[256];
    rockface_det_t ir_face;
    rockface_det_t rgb_face;
};

struct test_result {
    bool en;
    int rgb_detect_total;
    int rgb_detect_ok;
    int rgb_track_total;
    int rgb_track_ok;
    int rgb_landmark_total;
    int rgb_landmark_ok;
    int rgb_align_total;
    int rgb_align_ok;
    int rgb_extract_total;
    int rgb_extract_ok;
    int rgb_search_total;
    int rgb_search_ok;
    int ir_detect_total;
    int ir_detect_ok;
    int ir_liveness_total;
    int ir_liveness_ok;
};

void rockface_start_test(void);
typedef void (*get_test_callback)(struct test_result *test);
void register_get_test_callback(get_test_callback cb);

typedef void (*display_callback)(void *ptr, int fd, int fmt, int w, int h, int rotation);

void set_rgb_param(int width, int height, display_callback cb, bool expo);
void set_ir_param(int width, int height, display_callback cb);
void set_usb_param(int width, int height, display_callback cb);
void set_face_param(int width, int height, int cnt);
void set_rgb_display(display_callback cb);
void set_ir_display(display_callback cb);
void set_usb_display(display_callback cb);

void set_rgb_rotation(int angle);
void set_ir_rotation(int angle);
void set_usb_rotation(int angle);

int rkfacial_init(void);
void rkfacial_exit(void);
void rkfacial_register(void);
void rkfacial_delete(void);

typedef void (*rkfacial_paint_box_callback)(int left, int top, int right, int bottom);
void register_rkfacial_paint_box(rkfacial_paint_box_callback cb);
extern rkfacial_paint_box_callback rkfacial_paint_box_cb;

typedef void (*rkfacial_paint_info_callback)(struct user_info *info, bool real);
void register_rkfacial_paint_info(rkfacial_paint_info_callback cb);
extern rkfacial_paint_info_callback rkfacial_paint_info_cb;

void save_ir_real(bool flag);
void save_ir_fake(bool flag);

typedef void (*rkfacial_paint_face_callback)(void *ptr, int fmt, int width, int height, int x, int y, int w, int h);
void register_rkfacial_paint_face(rkfacial_paint_face_callback cb);
extern rkfacial_paint_face_callback rkfacial_paint_face_cb;

typedef void (*get_face_config_region_callback)(int x, int y, int w, int h);
void register_get_face_config_region(get_face_config_region_callback cb);
extern get_face_config_region_callback get_face_config_region_cb;

#ifdef __cplusplus
}
#endif

#endif

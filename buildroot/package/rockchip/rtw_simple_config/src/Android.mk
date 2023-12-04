L_CFLAGS = -DANDROID_ENV
L_CFLAGS += -DSIMPLE_CONFIG_PBC_SUPPORT
#L_CFLAGS += -DPLATFORM_MSTAR             #mStar platform doesn't support system() function.
#L_CFLAGS += -DCONFIG_IOCTL_CFG80211 	#Using iw command, not iwconfig command.
#L_CFLAGS += -DCONFIG_NOFCS

LOCAL_PATH := $(call my-dir)

#LOCAL_MODULE := rtw_simple_config
#LOCAL_STATIC_LIBRARIES := libsc
#LOCAL_CFLAGS := $(L_CFLAGS)
#LOCAL_SRC_FILES := main.c
#LOCAL_C_INCLUDES := $(INCLUDES)
#include $(BUILD_EXECUTABLE)



include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libsc.a
LOCAL_STATIC_LIBRARIES := libsc
include $(BUILD_MULTI_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE    := rtw_simple_config
LOCAL_STATIC_LIBRARIES := libsc
LOCAL_CFLAGS := $(L_CFLAGS)
LOCAL_SRC_FILES := main.c
include $(BUILD_EXECUTABLE)

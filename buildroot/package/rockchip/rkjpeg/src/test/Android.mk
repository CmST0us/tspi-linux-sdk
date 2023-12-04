# Copyright 2006 The Android Open Source Project
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_TOP_DIR := $(LOCAL_PATH)/../
LOCAL_SRC_DIR := $(LOCAL_PATH)/../dec
LOCAL_TST_DIR := $(LOCAL_PATH)/../test
LOCAL_VPU_DIR := $(LOCAL_PATH)/../../common
LOCAL_INC_DIR := $(LOCAL_PATH)/../../common/include

LOCAL_MODULE := librkmjpeghwdec

ifeq ($(PLATFORM_VERSION),4.0.4)
	LOCAL_CFLAGS := -DAVS40
endif

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 
                            
LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(LOCAL_VPU_DIR) \
					$(LOCAL_INC_DIR) \
					$(LOCAL_SRC_DIR) \
					$(LOCAL_SRC_DIR)/config \
					$(LOCAL_SRC_DIR)/inc \
					$(LOCAL_SRC_DIR)/jpeg \
					$(LOCAL_TST_DIR) \

LOCAL_SRC_FILES :=  ../dec/jpeg/jpegdecapi.c \
                    ../dec/jpeg/jpegdechdrs.c \
                    ../dec/jpeg/jpegdecinternal.c \
                    ../dec/jpeg/jpegdecscan.c \
                    ../dec/jpeg/jpegdecutils.c \
                    ../dec/jpeg/dwl_test.c      \
                    hw_jpegdecapi.c \
                    pv_jpegdec_api.cpp
                    
include $(BUILD_STATIC_LIBRARY)


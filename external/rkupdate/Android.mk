LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
prebuilt_stdcxx_PATH := prebuilts/ndk/current/sources/cxx-stl
LOCAL_SRC_FILES := \
	CRC.cpp \
	MD5Checksum.cpp \
	RKBoot.cpp \
	RKImage.cpp \
	RKLog.cpp\
	RKComm.cpp\
	RKDevice.cpp\
	RKAndroidDevice.cpp\
	Upgrade.cpp
	

LOCAL_C_INCLUDES := \
	$(prebuilt_stdcxx_PATH)/gnu-libstdc++/include\
	$(prebuilt_stdcxx_PATH)/gnu-libstdc++/libs/armeabi-v7a/include\
	bionic \
	bionic/libstdc++/include \
	external/e2fsprogs/lib \
	$(LOCAL_PATH)/rkrsa


LOCAL_STATIC_LIBRARIES := \
	librkrsa\
	libext2_uuid\
	libgnustl_static\
	libstdc++

LOCAL_MULTILIB := 64 

LOCAL_MODULE := librkupdate

LOCAL_CFLAGS += 
LOCAL_CPPFLAGS += -Wall -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -fexceptions

include $(BUILD_STATIC_LIBRARY)
include $(LOCAL_PATH)/stl/Android.mk\
				$(LOCAL_PATH)/rsa/Android.mk\
				$(LOCAL_PATH)/uuid/Android.mk

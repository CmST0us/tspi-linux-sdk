MYNT_EYE_D_SDK_VERSION = 0d7bcc63ddf51f37aee40b39873fab3e5e34d5fd
MYNT_EYE_D_SDK_SITE = $(call github,slightech,MYNT-EYE-D-SDK,$(MYNT_EYE_D_SDK_VERSION))

include $(sort $(wildcard package/mynt-eye-d-sdk/*/*.mk))

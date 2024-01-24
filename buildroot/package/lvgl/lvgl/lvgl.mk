################################################################################
#
# LittleVGL
#
################################################################################

LVGL_VERSION = 8.3.0
LVGL_SITE = $(call github,lvgl,lvgl,v$(LVGL_VERSION))
LVGL_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_LVGL_DEMO_WIDGETS), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_WIDGETS=1
endif

ifeq ($(BR2_PACKAGE_LVGL_DEMO_KEYPAD_AND_ENCODER), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_KEYPAD_AND_ENCODER=1
endif

ifeq ($(BR2_PACKAGE_LVGL_DEMO_BENCHMARK), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_BENCHMARK=1
endif

ifeq ($(BR2_PACKAGE_LVGL_DEMO_STRESS), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_STRESS=1
endif

ifeq ($(BR2_PACKAGE_LVGL_DEMO_MUSIC), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_MUSIC=1
endif

ifeq ($(BR2_PACKAGE_LVGL_DEMO_CUSTOM), y)
LVGL_CONF_OPTS += -DLV_USE_DEMO_CUSTOM=1
endif

ifeq ($(BR2_PACKAGE_LVGL_USE_SDL), y)
LVGL_CONF_OPTS += -DLV_USE_GPU_SDL=1
endif

$(eval $(cmake-package))

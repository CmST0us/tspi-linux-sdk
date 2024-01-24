################################################################################
#
# vkmark
#
################################################################################

VKMARK_VERSION = d872846e2e7c47010c11227eb713d00ccfdd35c6
VKMARK_SITE = $(call github,vkmark,vkmark,$(VKMARK_VERSION))
VKMARK_LICENSE = LGPL-2.1
VKMARK_LICENSE_FILES = COPYING-LGPL2.1

VKMARK_DEPENDENCIES = assimp glm vulkan-loader libdrm

ifeq ($(BR2_PACKAGE_WAYLAND),y)
VKMARK_DEPENDENCIES += wayland
endif

$(eval $(meson-package))

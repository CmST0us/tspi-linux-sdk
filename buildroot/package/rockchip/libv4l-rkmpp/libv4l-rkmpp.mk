################################################################################
#
# libv4l-rkmpp
#
################################################################################

LIBV4L_RKMPP_SITE = https://github.com/JeffyCN/libv4l-rkmpp.git
LIBV4L_RKMPP_VERSION = fe977181a5ab53bb350706cdc259ba6ccd4528da
LIBV4L_RKMPP_SITE_METHOD = git
LIBV4L_RKMPP_AUTORECONF = YES

LIBV4L_RKMPP_LICENSE = LGPL-2.1
LIBV4L_RKMPP_LICENSE_FILES = COPYING

LIBV4L_RKMPP_DEPENDENCIES = libv4l rockchip-mpp

ifeq ($(BR2_PREFER_ROCKCHIP_RGA),y)
LIBV4L_RKMPP_DEPENDENCIES += rockchip-rga
LIBV4L_RKMPP_CONF_OPTS += -Drga=enabled
else
LIBV4L_RKMPP_CONF_OPTS += -Drga=disabled
endif

$(eval $(meson-package))

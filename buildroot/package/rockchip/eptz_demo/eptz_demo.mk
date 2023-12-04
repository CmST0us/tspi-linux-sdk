################################################################################
#
## eptz_demo
#
#################################################################################


EPTZ_DEMO_SITE = $(TOPDIR)/../app/eptz_demo
EPTZ_DEMO_SITE_METHOD = local
EPTZ_DEMO_INSTALL_STAGING = NO

SMART_DISPLAY_SERVICE_CONF_OPTS += "-DCMAKE_BUILD_TYPE=Debug"

ifeq ($(BR2_PACKAGE_ROCKX),y)
EPTZ_DEMO_DEPENDENCIES += rockx
EPTZ_DEMO_CONF_OPTS += "-DROCKX=ON" "-DROCKX_HEADER_DIR=$(STAGING_DIR)/usr/include/rockx"
endif

EPTZ_DEMO_DEPENDENCIES += rkmedia sdl2 sdl2_gfx sdl2_ttf uvc_app rockchip-rga
EPTZ_DEMO_CONF_OPTS += "-DRKNPU_HEADER_DIR=$(RKNPU_BUILDDIR)/rknn/include"

$(eval $(cmake-package))

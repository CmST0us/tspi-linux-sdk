RKCAMERA_SITE = $(TOPDIR)/../framework/rkcamera
RKCAMERA_SITE_METHOD = local
RKCAMERA_INSTALL_STAGING = YES

# add dependencies
RKCAMERA_DEPENDENCIES = camerahal adk libion cjson

ifeq ($(BR2_PACKAGE_RKCAMERA_TEST),y)
    RKCAMERA_DEPENDENCIES += rockchip-rga rkfb process_units
    RKCAMERA_CONF_OPTS += -DRKCAMERA_TEST=1
endif

RKCAMERA_CONF_OPTS += \
    -DBOARD_VERSION=rv1108-$(RK_TARGET_BOARD_VERSION)

$(eval $(cmake-package))

ifeq ($(BR2_PACKAGE_RKSL), y)
    RKSL_SITE = $(TOPDIR)/../app/rksl
    RKSL_SITE_METHOD = local
    RKSL_INSTALL_STAGING = YES
    RKSL_DEPENDENCIES = libdrm rockchip-rga uvc_app rockface

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP),y)
    RKSL_DEPENDENCIES += camera_engine_rkisp
    RKSL_CONF_OPTS += "-DCAMERA_ENGINE_RKISP=y"
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKAIQ),y)
    RKSL_DEPENDENCIES += camera_engine_rkaiq
    RKSL_CONF_OPTS += "-DCAMERA_ENGINE_RKAIQ=y"
endif

    RKSL_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

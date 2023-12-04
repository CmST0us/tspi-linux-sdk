ifeq ($(BR2_PACKAGE_MULT_UVC_DEMO), y)
    MULT_UVC_DEMO_SITE = $(TOPDIR)/../app/mult_uvc_demo
    MULT_UVC_DEMO_SITE_METHOD = local
    MULT_UVC_DEMO_INSTALL_STAGING = YES
    MULT_UVC_DEMO_DEPENDENCIES = libdrm rockchip-rga rockchip-mpp 

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP),y)
    MULT_UVC_DEMO_DEPENDENCIES += camera_engine_rkisp
    MULT_UVC_DEMO_CONF_OPTS += "-DCAMERA_ENGINE_RKISP=y"
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKAIQ),y)
    MULT_UVC_DEMO_DEPENDENCIES += camera_engine_rkaiq
    MULT_UVC_DEMO_CONF_OPTS += "-DCAMERA_ENGINE_RKAIQ=y"
endif

    MULT_UVC_DEMO_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

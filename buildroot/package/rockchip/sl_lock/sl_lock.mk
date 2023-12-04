ifeq ($(BR2_PACKAGE_SL_LOCK), y)
    SL_LOCK_SITE = $(TOPDIR)/../app/sl_lock
    SL_LOCK_SITE_METHOD = local
    SL_LOCK_INSTALL_STAGING = YES
    SL_LOCK_DEPENDENCIES = libdrm rockchip-rga rkmedia

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKISP),y)
    SL_LOCK_DEPENDENCIES += camera_engine_rkisp
    SL_LOCK_CONF_OPTS += "-DCAMERA_ENGINE_RKISP=y"
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKAIQ),y)
    SL_LOCK_DEPENDENCIES += camera_engine_rkaiq
    SL_LOCK_CONF_OPTS += "-DCAMERA_ENGINE_RKAIQ=y"
endif

    SL_LOCK_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

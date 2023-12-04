ifeq ($(BR2_PACKAGE_FACE_DETECT), y)
    FACE_DETECT_SITE = $(TOPDIR)/../app/face_detect
    FACE_DETECT_SITE_METHOD = local
    FACE_DETECT_INSTALL_STAGING = YES
    FACE_DETECT_DEPENDENCIES = rknpu libdrm rockchip-rga camera_engine_rkisp
    FACE_DETECT_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

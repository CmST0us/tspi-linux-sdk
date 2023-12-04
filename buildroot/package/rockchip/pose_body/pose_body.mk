ifeq ($(BR2_PACKAGE_POSE_BODY), y)
    POSE_BODY_SITE = $(TOPDIR)/../app/pose_body
    POSE_BODY_SITE_METHOD = local
    POSE_BODY_INSTALL_STAGING = YES
    POSE_BODY_DEPENDENCIES = rockx libdrm rockchip-rga tslib minigui jpeg libpng freetype camera_engine_rkisp rockchip-mpp
    POSE_BODY_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

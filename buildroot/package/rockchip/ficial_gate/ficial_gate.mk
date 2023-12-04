ifeq ($(BR2_PACKAGE_FICIAL_GATE), y)
    FICIAL_GATE_SITE = $(TOPDIR)/../app/ficial_gate
    FICIAL_GATE_SITE_METHOD = local
    FICIAL_GATE_INSTALL_STAGING = YES
    FICIAL_GATE_DEPENDENCIES = rkfacial libdrm rockchip-rga tslib minigui jpeg libpng freetype
    FICIAL_GATE_CONF_OPTS += "-DDRM_HEADER_DIR=$(STAGING_DIR)/usr/include/drm"
    $(eval $(cmake-package))
endif

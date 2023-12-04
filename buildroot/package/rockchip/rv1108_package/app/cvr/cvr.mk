CVR_SITE = $(TOPDIR)/../app/cvr
CVR_SITE_METHOD = local
CVR_INSTALL_STAGING = YES

# add dependencies
CVR_DEPENDENCIES = rkcamera messenger process_units libpng rv1108_minigui \
				   hal rknr rockchip-mpp rkmedia adk

CVR_CONF_OPTS += -DUI_RESOLUTION=$(call qstrip,$(RK_UI_RESOLUTION))

ifeq ($(BR2_PACKAGE_CVR_RTSP_STREAM),y)
    CVR_CONF_OPTS += -DUSE_RTSP_STREAM=1
endif

define CVR_INSTALL_INIT_SYSV
	[ -z "$(BR2_PACKAGE_SYSTEM_MANAGER)" ] && $(INSTALL) -m 0755 -D \
			package/rockchip/rv1108_package/app/cvr/S12_cvr $(TARGET_DIR)/etc/init.d || true
	$(INSTALL) -m 0644 -D package/rockchip/rv1108_package/app/cvr/cvr.conf \
			$(TARGET_DIR)/etc/cvr.conf
endef

$(eval $(cmake-package))

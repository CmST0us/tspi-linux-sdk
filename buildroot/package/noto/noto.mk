
ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define NOTO_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 package/noto/70-fonts-noto-cjk.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

include $(sort $(wildcard package/noto/*/*.mk))

ADDRESS_SANITIZER_SITE_METHOD = local
ADDRESS_SANITIZER_SITE = $(TOPDIR)/package/rockchip/address_sanitizer

define ADDRESS_SANITIZER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(STAGING_DIR)/lib/libasan.so.5.0.0 $(TARGET_DIR)/lib/
	(cd $(TARGET_DIR)/lib && ln -srf libasan.so.5.0.0 libasan.so && ln -srf libasan.so.5.0.0 libasan.so.5 )
endef

$(eval $(generic-package))

#############################################################
#
#
#############################################################
ifeq ($(BR2_PACKAGE_RTW_SIMPLE_CONFIG), y)
RE_VERSION:=1.0.0
RTW_SIMPLE_CONFIG_SITE=$(TOPDIR)/package/rockchip/rtw_simple_config/src
RTW_SIMPLE_CONFIG_SITE_METHOD=local

#PM_SUSPEN_API_EXTRA_INCLUDE = `$(PKG_CONFIG_HOST_BINARY) --cflags dbus-1` -I $(@D)
#RTW_SIMPLE_CONFIG_MAKE_ENV = CFLAGS="$(PM_SUSPEN_API_EXTRA_INCLUDE)"

define RTW_SIMPLE_CONFIG_BUILD_CMDS
    $(RTW_SIMPLE_CONFIG_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) AR=$(TARGET_AR) -C $(@D)
endef

define RTW_SIMPLE_CONFIG_CLEAN_CMDS
    $(RTW_SIMPLE_CONFIG_MAKE_ENV) $(MAKE) -C $(@D) clean
	$(RTW_SIMPLE_CONFIG_MAKE_ENV) $(MAKE) -C $(@D)/src clean
endef

define RTW_SIMPLE_CONFIG_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/rtw_simple_config $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
endif

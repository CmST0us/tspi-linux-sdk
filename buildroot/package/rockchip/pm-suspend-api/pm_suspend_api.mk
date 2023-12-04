#############################################################
#
# pm-suspend-api & demo
#
#############################################################
ifeq ($(BR2_PACKAGE_PM_SUSPEND_API), y)
RE_VERSION:=1.0.0
PM_SUSPEND_API_SITE=$(TOPDIR)/package/rockchip/pm-suspend-api/src
PM_SUSPEND_API_SITE_METHOD=local

PM_SUSPEN_API_EXTRA_INCLUDE = `$(PKG_CONFIG_HOST_BINARY) --cflags dbus-1` \
	-I $(@D)

PM_SUSPEND_API_MAKE_ENV = \
    CFLAGS="$(PM_SUSPEN_API_EXTRA_INCLUDE)"

define PM_SUSPEND_API_BUILD_CMDS
    $(PM_SUSPEND_API_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) -C $(@D)
	$(PM_SUSPEND_API_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) -C $(@D)/test
endef

define PM_SUSPEND_API_CLEAN_CMDS
    $(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D) clean
	$(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D)/test clean
endef

define PM_SUSPEND_API_INSTALL_TARGET_CMDS
    $(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D) install
	$(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D)/test install
endef

define PM_SUSPEND_API_UNINSTALL_TARGET_CMDS
    $(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D) uninstall
	$(PM_SUSPEND_API_MAKE_ENV) $(MAKE) -C $(@D)/test uninstall
endef

$(eval $(generic-package))
endif

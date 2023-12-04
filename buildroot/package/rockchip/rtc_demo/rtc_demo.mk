#############################################################
#
# RTC_DEMO
#
#############################################################
ifeq ($(BR2_PACKAGE_RTC_DEMO), y)
RTC_DEMO_VERSION:=1.0.0
RTC_DEMO_SITE=$(TOPDIR)/../external/rtc_demo
RTC_DEMO_SITE_METHOD=local

define RTC_DEMO_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef

define RTC_DEMO_CLEAN_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

define RTC_DEMO_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

define RTC_DEMO_UNINSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))
endif

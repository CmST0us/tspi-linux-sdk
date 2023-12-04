#############################################################
#
# alsa_capture
#
#############################################################
ifeq ($(BR2_PACKAGE_IFLYTEKSDK), y)
IFLYTEKSDK_VERSION:=1.0.0
IFLYTEKSDK_SITE=$(TOPDIR)/../external/iflytekSDK
IFLYTEKSDK_SITE_METHOD=local
ifeq ($(BR2_aarch64),y)
define IFLYTEKSDK_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CAE_PATH=libs_64bit CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef
define IFLYTEKSDK_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CAE_PATH=libs_64bit -C $(@D) install
endef
else
define IFLYTEKSDK_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CAE_PATH=libs CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef
define IFLYTEKSDK_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CAE_PATH=libs -C $(@D) install
endef
endif

define IFLYTEKSDK_CLEAN_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

define IFLYTEKSDK_UNINSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))
endif

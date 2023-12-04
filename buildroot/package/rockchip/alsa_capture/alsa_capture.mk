#############################################################
#
# alsa_capture
#
#############################################################
ifeq ($(BR2_PACKAGE_ALSA_CAPTURE), y)
ALSA_CAPTURE_VERSION:=1.0.0
ALSA_CAPTURE_SITE=$(TOPDIR)/../external/alsa_capture/src
ALSA_CAPTURE_SITE_METHOD=local

define ALSA_CAPTURE_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef

define ALSA_CAPTURE_CLEAN_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

define ALSA_CAPTURE_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

define ALSA_CAPTURE_UNINSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))
endif

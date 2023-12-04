################################################################################
#
# Flutter engine
#
################################################################################

FLUTTER_ENGINE_VERSION = 1e15746bd0d250f5d38640dba19d5765977ce854
FLUTTER_ENGINE_SITE = $(call github,JeffyCN,flutter-engine,$(FLUTTER_ENGINE_VERSION))

ifeq ($(BR2_aarch64),y)
FLUTTER_ENGINE_ARCH = aarch64
else
FLUTTER_ENGINE_ARCH = armhf
endif

FLUTTER_ENGINE_LICENSE_FILES = LICENSE
FLUTTER_ENGINE_INSTALL_STAGING = YES

define FLUTTER_ENGINE_INSTALL_STAGING_CMDS
	cp -rp $(@D)/$(FLUTTER_ENGINE_ARCH)/usr/ $(STAGING_DIR)/
endef

define FLUTTER_ENGINE_INSTALL_TARGET_CMDS
	cp -rp $(@D)/$(FLUTTER_ENGINE_ARCH)/usr/ $(TARGET_DIR)/
endef

$(eval $(generic-package))

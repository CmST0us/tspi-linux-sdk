###############################################
#
#	SecurityAuth
#
###############################################

SECURITYAUTH_SITE_METHOD = local
SECURITYAUTH_VERSION = 1.0
SECURITYAUTH_SITE = $(TOPDIR)/package/rockchip/securityAuth/src


ifeq ($(BR2_PACKAGE_SECURITYAUTH_3128H),y)
CHIP_VENDOR = ROCKCHIP
CHIP_NAME = 3128h
endif

ifeq ($(CHIP_VENDOR), ROCKCHIP)
define SECURITYAUTH_BUILD_CMDS
	mkdir -p $(TARGET_DIR)/lib/optee_armtz
	mkdir -p $(TARGET_DIR)/usr/bin
	mkdir -p $(TARGET_DIR)/usr/lib

	$(INSTALL) -D -m 0666 $(@D)/$(CHIP_NAME)/optee_armtz/* $(TARGET_DIR)/lib/optee_armtz/

	$(INSTALL) -D -m 0755 $(@D)/$(CHIP_NAME)/bin/* $(TARGET_DIR)/usr/bin/

	$(INSTALL) -D -m 0666 $(@D)/$(CHIP_NAME)/lib/* $(TARGET_DIR)/usr/lib/
endef
endif

$(eval $(generic-package))

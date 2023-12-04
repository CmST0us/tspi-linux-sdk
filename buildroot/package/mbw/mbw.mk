################################################################################
#
# mbw
#
################################################################################

MBW_VERSION = master
MBW_SITE = https://github.com/raas/mbw
MBW_SITE_METHOD = git
MBW_LICENSE = MIT
MBW_LICENSE_FILES = LICENSE

define MBW_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define MBW_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/mbw $(TARGET_DIR)/usr/bin/mbw
endef

$(eval $(generic-package))

################################################################################
# 
# testApp
#
################################################################################

MOBILENET_VERSION = 1.0
MOBILENET_SITE = ${TOPDIR}/../app/4G-daemon
MOBILENET_SITE_METHOD = local

MOBILENET_LICENSE = Rongpin
MOBILENET_LICENSE_FILES = NOTICE

define MOBILENET_BUILD_CMDS
        $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)"
endef

define MOBILENET_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 0755 $(@D)/4G-daemon $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/S99_4G-daemon $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))

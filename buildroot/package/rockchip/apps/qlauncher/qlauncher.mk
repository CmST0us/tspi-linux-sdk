################################################################################
#
# qlauncher
#
################################################################################

QLAUNCHER_VERSION = 1.0
QLAUNCHER_SITE = $(TOPDIR)/../app/QLauncher
QLAUNCHER_SITE_METHOD = local

QLAUNCHER_LICENSE = ROCKCHIP
QLAUNCHER_LICENSE_FILES = LICENSE

# TODO: Add install rules in .pro
define QLAUNCHER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/QLauncher $(TARGET_DIR)/usr/bin/QLauncher
endef

define QLAUNCHER_INSTALL_INIT_SYSV
$(INSTALL) -D -m 755 $(QLAUNCHER_PKGDIR)/S50launcher \
	$(TARGET_DIR)/etc/init.d/S50launcher
endef

$(eval $(qmake-package))

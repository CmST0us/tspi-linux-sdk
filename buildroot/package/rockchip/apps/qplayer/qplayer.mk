################################################################################
#
# qplayer
#
################################################################################

QPLAYER_VERSION = 1.0
QPLAYER_SITE = $(TOPDIR)/../app/qplayer
QPLAYER_SITE_METHOD = local

QPLAYER_LICENSE = ROCKCHIP
QPLAYER_LICENSE_FILES = LICENSE

# TODO: Add install rules in .pro
define QPLAYER_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_player.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qplayer $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/qplayer.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(qmake-package))

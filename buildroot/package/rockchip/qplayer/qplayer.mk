################################################################################
#
# qplayer
#
################################################################################

QPLAYER_VERSION = 1.0
QPLAYER_SITE = $(TOPDIR)/../app/qplayer
QPLAYER_SITE_METHOD = local

QPLAYER_LICENSE = Apache V2.0
QPLAYER_LICENSE_FILES = NOTICE

define QPLAYER_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define QPLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QPLAYER_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_player.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qplayer $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/qplayer.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))

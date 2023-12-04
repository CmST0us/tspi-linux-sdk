################################################################################
#
# music
#
################################################################################

MUSIC_VERSION = 1.0
MUSIC_SITE = $(TOPDIR)/../app/music
MUSIC_SITE_METHOD = local

MUSIC_LICENSE = Apache V2.0
MUSIC_LICENSE_FILES = NOTICE

define MUSIC_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define MUSIC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define MUSIC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/local/music $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	cp $(BUILD_DIR)/music-$(MUSIC_VERSION)/conf/* $(TARGET_DIR)/usr/local/music/
	cp $(BUILD_DIR)/music-$(MUSIC_VERSION)/conf/icon_music.png $(TARGET_DIR)/usr/share/icon/
	cp $(BUILD_DIR)/music-$(MUSIC_VERSION)/music.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/musicPlayer $(TARGET_DIR)/usr/bin/musicPlayer
endef

$(eval $(generic-package))

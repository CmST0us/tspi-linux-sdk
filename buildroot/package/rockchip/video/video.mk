################################################################################
#
# video
#
################################################################################

VIDEO_VERSION = 1.0
VIDEO_SITE = $(TOPDIR)/../app/video
VIDEO_SITE_METHOD = local

VIDEO_LICENSE = Apache V2.0
VIDEO_LICENSE_FILES = NOTICE

define VIDEO_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define VIDEO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define VIDEO_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/local/video $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	cp $(BUILD_DIR)/video-$(VIDEO_VERSION)/conf/* $(TARGET_DIR)/usr/local/video/
	cp $(BUILD_DIR)/video-$(VIDEO_VERSION)/conf/icon_video.png $(TARGET_DIR)/usr/share/icon/
	cp $(BUILD_DIR)/video-$(VIDEO_VERSION)/video.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/videoPlayer $(TARGET_DIR)/usr/bin/videoPlayer
endef

$(eval $(generic-package))

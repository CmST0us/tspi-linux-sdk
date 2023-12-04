################################################################################
#
# camera
#
################################################################################

CAMERA_VERSION = 1.0
CAMERA_SITE = $(TOPDIR)/../app/camera
CAMERA_SITE_METHOD = local

CAMERA_LICENSE = Apache V2.0
CAMERA_LICENSE_FILES = NOTICE

define CAMERA_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define CAMERA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CAMERA_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/local/camera $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	cp $(BUILD_DIR)/camera-$(CAMERA_VERSION)/conf/* $(TARGET_DIR)/usr/local/camera/
	cp $(BUILD_DIR)/camera-$(CAMERA_VERSION)/conf/icon_camera.png $(TARGET_DIR)/usr/share/icon/
	cp $(BUILD_DIR)/camera-$(CAMERA_VERSION)/camera.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/cameraView 	$(TARGET_DIR)/usr/bin/cameraView
endef

$(eval $(generic-package))

################################################################################
#
# qcamera
#
################################################################################

QCAMERA_VERSION = 1.0
QCAMERA_SITE = $(TOPDIR)/../app/qcamera
QCAMERA_SITE_METHOD = local

QCAMERA_LICENSE = ROCKCHIP
QCAMERA_LICENSE_FILES = LICENSE

# TODO: Add install rules in .pro
define QCAMERA_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_camera.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qcamera $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/qcamera.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(qmake-package))

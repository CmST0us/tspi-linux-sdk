################################################################################
#
# qcamera
#
################################################################################

QCAMERA_VERSION = 1.0
QCAMERA_SITE = $(TOPDIR)/../app/qcamera
QCAMERA_SITE_METHOD = local

QCAMERA_LICENSE = Apache V2.0
QCAMERA_LICENSE_FILES = NOTICE

define QCAMERA_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define QCAMERA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QCAMERA_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_camera.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qcamera $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/qcamera.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))

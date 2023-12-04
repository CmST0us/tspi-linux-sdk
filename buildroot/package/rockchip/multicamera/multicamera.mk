################################################################################
#
# multicamera
#
################################################################################

MULTICAMERA_VERSION = 1.0
MULTICAMERA_SITE = $(TOPDIR)/../app/multicamera
MULTICAMERA_SITE_METHOD = local

MULTICAMERA_LICENSE = Apache V2.0
MULTICAMERA_LICENSE_FILES = NOTICE

define MULTICAMERA_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define MULTICAMERA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define MULTICAMERA_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_multicamera.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/multicamera $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/multicamera.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))

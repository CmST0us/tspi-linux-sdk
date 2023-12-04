################################################################################
#
# qfm
#
################################################################################

QFM_VERSION = 1.0
QFM_SITE = $(TOPDIR)/../app/qfm
QFM_SITE_METHOD = local

QFM_LICENSE = Apache V2.0
QFM_LICENSE_FILES = NOTICE

define QFM_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define QFM_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QFM_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/image/icon_folder.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qfm	$(TARGET_DIR)/usr/bin/qfm
	$(INSTALL) -D -m 0755 $(@D)/qfm.desktop	$(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/mimeapps.list $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))

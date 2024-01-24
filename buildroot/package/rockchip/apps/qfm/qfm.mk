################################################################################
#
# qfm
#
################################################################################

QFM_VERSION = 1.0
QFM_SITE = $(TOPDIR)/../app/qfm
QFM_SITE_METHOD = local

QFM_LICENSE = ROCKCHIP
QFM_LICENSE_FILES = LICENSE

# TODO: Add install rules in .pro
define QFM_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/image/icon_folder.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qfm	$(TARGET_DIR)/usr/bin/qfm
	$(INSTALL) -D -m 0755 $(@D)/qfm.desktop	$(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/mimeapps.list $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(qmake-package))

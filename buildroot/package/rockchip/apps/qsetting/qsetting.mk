################################################################################
#
# qsetting
#
################################################################################

QSETTING_VERSION = 1.0
QSETTING_SITE = $(TOPDIR)/../app/qsetting
QSETTING_SITE_METHOD = local

QSETTING_LICENSE = ROCKCHIP
QSETTING_LICENSE_FILES = LICENSE
QSETTING_DEPENDENCIES += rkwifibt-app
define QSETTING_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define QSETTING_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CXXFLAGS+="-DRKWIFIBTAPP -I$(TOPDIR)/../external/rkwifibt-app/include" LFLAGS+="-lrkwifibt -lasound" -C $(@D)
endef

define QSETTING_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_qsetting.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/qsetting $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/qsetting.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/S80wifireconnect $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))

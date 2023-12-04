################################################################################
#
# settings
#
################################################################################

SETTINGS_VERSION = 1.0
SETTINGS_SITE = $(TOPDIR)/../app/settings
SETTINGS_SITE_METHOD = local

SETTINGS_LICENSE = Apache V2.0
SETTINGS_LICENSE_FILES = NOTICE

define SETTINGS_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define SETTINGS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define SETTINGS_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/local/settings $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	cp $(BUILD_DIR)/settings-$(SETTINGS_VERSION)/conf/* $(TARGET_DIR)/usr/local/settings/
	cp $(BUILD_DIR)/settings-$(SETTINGS_VERSION)/conf/icon_setting.png $(TARGET_DIR)/usr/share/icon/
	cp $(BUILD_DIR)/settings-$(SETTINGS_VERSION)/setting.desktop $(TARGET_DIR)/usr/share/applications/
	$(INSTALL) -D -m 0755 $(@D)/settings $(TARGET_DIR)/usr/bin/settings
	$(INSTALL) -D -m 0755 $(@D)/S70settings_wifibt $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))

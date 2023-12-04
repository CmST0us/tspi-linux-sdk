################################################################################
#
# multivideoplayer
#
################################################################################

MULTIVIDEOPLAYER_VERSION = 1.0
MULTIVIDEOPLAYER_SITE = $(TOPDIR)/../app/multivideoplayer
MULTIVIDEOPLAYER_SITE_METHOD = local

MULTIVIDEOPLAYER_LICENSE = Apache V2.0
MULTIVIDEOPLAYER_LICENSE_FILES = NOTICE

define MULTIVIDEOPLAYER_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define MULTIVIDEOPLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define MULTIVIDEOPLAYER_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/applications $(TARGET_DIR)/usr/share/icon
	$(INSTALL) -D -m 0644 $(@D)/icon_multivideoplayer.png $(TARGET_DIR)/usr/share/icon/
	$(INSTALL) -D -m 0755 $(@D)/multivideoplayer $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 0755 $(@D)/multivideoplayer.desktop $(TARGET_DIR)/usr/share/applications/
endef

$(eval $(generic-package))

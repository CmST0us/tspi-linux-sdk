MINIGUI_DEMO_VERSION = develop
MINIGUI_DEMO_SITE = $(TOPDIR)/../app/minigui_demo
MINIGUI_DEMO_SITE_METHOD = local

MINIGUI_DEMO_LICENSE = Apache V2.0
MINIGUI_DEMO_LICENSE_FILES = NOTICE

MINIGUI_DEMO_DEPENDENCIES = minigui rockchip-rga jpeg libpng freetype

define MINIGUI_DEMO_BUILD_CMDS
	$(MAKE) -C $(@D) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)"
endef

define MINIGUI_DEMO_INSTALL_TARGET_CMDS
        mkdir -p $(TARGET_DIR)/usr/local/share/minigui $(TARGET_DIR)/usr/local/etc
        cp -r $(@D)/Resources/res $(TARGET_DIR)/usr/local/share/minigui/
        $(INSTALL) -m 0644 -D $(@D)/Resources/MiniGUI.cfg $(TARGET_DIR)/usr/local/etc/MiniGUI.cfg
        $(INSTALL) -m 0755 -D $(@D)/minigui_demo $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

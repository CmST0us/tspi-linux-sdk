# Rockchip's Single Shot Multibox Detector for Linux

RKSSD_SITE = $(TOPDIR)/../external/rkssd
RKSSD_SITE_METHOD = local

define RKSSD_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/rkssd $(TARGET_DIR)/usr/bin/
endef

$(eval $(cmake-package))

# Rockchip's Audio Service for Linux

AUDIOSERVICE_SITE = $(TOPDIR)/../external/audioservice
AUDIOSERVICE_SITE_METHOD = local

define AUDIOSERVICE_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/audioservice $(TARGET_DIR)/usr/bin/
endef

$(eval $(cmake-package))

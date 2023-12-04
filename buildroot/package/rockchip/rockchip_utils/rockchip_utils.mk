################################################################################
#
# rockchip utils
#
################################################################################

ROCKCHIP_UTILS_VERSION = 1.0.0
ROCKCHIP_UTILS_SITE_METHOD = local
ROCKCHIP_UTILS_SITE = $(TOPDIR)/package/rockchip/rockchip_utils/src

define ROCKCHIP_UTILS_BUILD_CMDS
    $(TARGET_CC) -o $(@D)/io/io $(@D)/io/io.c
endef

define ROCKCHIP_UTILS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/io/io $(TARGET_DIR)/usr/bin/io
endef

$(eval $(generic-package))

################################################################################
#
# Rockchip Alsa Tool For Linux
#
################################################################################

ALSATOOL_VERSION = 1.0
ALSATOOL_SITE = $(TOPDIR)/../external/alsatool
ALSATOOL_SITE_METHOD = local

ALSATOOL_LICENSE = Apache V2.0
ALSATOOL_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
ALSATOOL_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) -lasound" \
        PROJECT_DIR="$(@D)"
define ALSATOOL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(ALSATOOL_MAKE_OPTS)
endef

define ALSATOOL_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/AlsaRecord $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 755 $(@D)/AlsaPlay $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

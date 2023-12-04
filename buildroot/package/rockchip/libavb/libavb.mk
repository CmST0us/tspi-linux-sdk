################################################################################
#
# Rockchip libavb For Linux
#
################################################################################

LIBAVB_VERSION = develop
LIBAVB_SITE = $(TOPDIR)/../external/avb/libavb
LIBAVB_SITE_METHOD = local

LIBAVB_INSTALL_STAGING = YES

LIBAVB_LICENSE = Apache V2.0
LIBAVB_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
LIBAVB_BUILD_OPTS=-I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-fPIC \
    -shared

LIBAVB_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(LIBAVB_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define LIBAVB_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(LIBAVB_MAKE_OPTS)
endef

define LIBAVB_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libavb.so $(STAGING_DIR)/usr/lib/
    mkdir -p $(STAGING_DIR)/usr/include/libavb/
    $(INSTALL) -D -m 0644 $(@D)/*.h $(STAGING_DIR)/usr/include/libavb/
endef

define LIBAVB_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libavb.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))

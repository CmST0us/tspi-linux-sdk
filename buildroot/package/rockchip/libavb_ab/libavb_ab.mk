################################################################################
#
# Rockchip libavb_ab For Linux
#
################################################################################

LIBAVB_AB_VERSION = develop
LIBAVB_AB_SITE = $(TOPDIR)/../external/avb/libavb_ab
LIBAVB_AB_SITE_METHOD = local
LIBAVB_AB_INSTALL_STAGING = YES

LIBAVB_AB_LICENSE = Apache V3.0
LIBAVB_AB_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
LIBAVB_AB_BUILD_OPTS=-I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-fPIC \
    -shared

LIBAVB_AB_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(LIBAVB_AB_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define LIBAVB_AB_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(LIBAVB_AB_MAKE_OPTS)
endef

define LIBAVB_AB_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libavb_ab.so $(STAGING_DIR)/usr/lib/
    mkdir -p $(STAGING_DIR)/usr/include/libavb_ab/
    $(INSTALL) -D -m 0644 $(@D)/*.h $(STAGING_DIR)/usr/include/libavb_ab/
endef

define LIBAVB_AB_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libavb_ab.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))

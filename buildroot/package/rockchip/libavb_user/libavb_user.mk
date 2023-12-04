################################################################################
#
# Rockchip libavb_user For Linux
#
################################################################################

LIBAVB_USER_VERSION = develop
LIBAVB_USER_SITE = $(TOPDIR)/../external/avb/libavb_user
LIBAVB_USER_SITE_METHOD = local
LIBAVB_USER_INSTALL_STAGING = YES

LIBAVB_USER_LICENSE = Apache V3.0
LIBAVB_USER_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
LIBAVB_USER_BUILD_OPTS=-I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-fPIC \
    -shared

LIBAVB_USER_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(LIBAVB_USER_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define LIBAVB_USER_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" $(LIBAVB_USER_MAKE_OPTS)
endef

define LIBAVB_USER_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libavb_user.so $(STAGING_DIR)/usr/lib/
    mkdir -p $(STAGING_DIR)/usr/include/libavb_user/
    $(INSTALL) -D -m 0644 $(@D)/*.h $(STAGING_DIR)/usr/include/libavb_user/
endef

define LIBAVB_USER_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 7755 $(@D)/libavb_user.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))

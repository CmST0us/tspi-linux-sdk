################################################################################
#
# Rockchip bootcontrol For Linux
#
################################################################################

BOOTCONTROL_VERSION = develop
BOOTCONTROL_SITE = $(TOPDIR)/../external/avb/boot_control/
BOOTCONTROL_SITE_METHOD = local

BOOTCONTROL_LICENSE = Apache V3.0
BOOTCONTROL_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
BOOTCONTROL_BUILD_OPTS=-I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
    -lavb \
    -lavb_ab \
    -lavb_user

BOOTCONTROL_DEPENDENCIES += libavb libavb_user libavb_ab

BOOTCONTROL_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(BOOTCONTROL_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define BOOTCONTROL_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" $(BOOTCONTROL_MAKE_OPTS)
endef

define BOOTCONTROL_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 765 $(@D)/bootcontrol $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

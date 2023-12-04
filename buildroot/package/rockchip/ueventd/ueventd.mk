################################################################################
#
# ueventd
#
################################################################################

UEVENTD_VERSION = 1.0
UEVENTD_SITE = $(TOPDIR)/../external/ueventd
UEVENTD_SITE_METHOD = local

CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"

UEVENTD_LICENSE_FILES = NOTICE
UEVENTD_LICENSE = Apache V2.0

define UEVENTD_TARGET_CFLAGS
	$(TARGET_CFLAGS) \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include \
	-I$(PROJECT_DIR)/include
endef

UEVENTD_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(UEVENTD_TARGET_CFLAGS)" \
        PROJECT_DIR="$(@D)"

define UEVENTD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(UEVENTD_MAKE_OPTS)
endef

define UEVENTD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/ueventd $(TARGET_DIR)/usr/bin/ueventd
endef

$(eval $(generic-package))

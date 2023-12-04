################################################################################
#
# Rockchip RecoverySystem For Linux
#
################################################################################

RECOVERYSYSTEM_VERSION = develop
RECOVERYSYSTEM_SITE = $(TOPDIR)/../external/recoverySystem
RECOVERYSYSTEM_SITE_METHOD = local

RECOVERYSYSTEM_LICENSE = Apache V2.0
RECOVERYSYSTEM_LICENSE_FILES = NOTICE
#CXX="$(TARGET_CXX)"
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
RECOVERYSYSTEM_BUILD_OPTS=-I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-fPIC

RECOVERYSYSTEM_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(RECOVERYSYSTEM_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define RECOVERYSYSTEM_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(RECOVERYSYSTEM_MAKE_OPTS)
endef

define RECOVERYSYSTEM_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/recoverySystem $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

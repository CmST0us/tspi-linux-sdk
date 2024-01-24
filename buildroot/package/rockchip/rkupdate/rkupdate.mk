################################################################################
#
# Rockchip rkupdate For Linux
#
################################################################################

RKUPDATE_VERSION = develop
RKUPDATE_SITE = $(TOPDIR)/../external/rkupdate
RKUPDATE_SITE_METHOD = local

RKUPDATE_LICENSE = Apache V2.0
RKUPDATE_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"

RKUPDATE_BUILD_OPTS=-I"$(STAGING_DIR)/usr/include/" -I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-fPIC \
	-lpthread -luuid

#RKUPDATE_BUILD_OPTS=-I"$(STAGING_DIR)/usr/include/" -I$(PROJECT_DIR) \
	--sysroot=$(STAGING_DIR) \
	-shared -nostdlib

ifeq ($(BR2_PACKAGE_RKUPDATE_SINGNATURE_FW),y)
	TARGET_CFLAGS += -DUSE_SIGNATURE_FW=ON
endif

ifeq ($(BR2_PACKAGE_RKUPDATE_SIMULATE_ABNORMAL_POWER_OFF),y)
	TARGET_CFLAGS += -DUSE_SIMULATE_POWER_OFF=ON
endif

RKUPDATE_MAKE_OPTS = \
	CFLAGS="$(TARGET_CFLAGS) $(RKUPDATE_BUILD_OPTS)" \
	PROJECT_DIR="$(@D)"



define RKUPDATE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(RKUPDATE_MAKE_OPTS)
endef

define RKUPDATE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/rkupdate $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

################################################################################
#
# Rockchip Linux A/B For Linux
#
################################################################################

LINUXAB_VERSION = develop
LINUXAB_SITE = $(TOPDIR)/../external/update_engine
LINUXAB_SITE_METHOD = local

LINUXAB_LICENSE = Apache V2.0
LINUXAB_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"
LINUXAB_BUILD_OPTS=-I$(PROJECT_DIR) \
    --sysroot=$(STAGING_DIR) \
    -fPIC \
    -lssl \
    -lcrypto \
    -lcurl \
    -lpthread
ifeq ($(BR2_PACKAGE_LINUXAB_SUCCESSFUL_BOOT),y)
    TARGET_CFLAGS += -DSUCCESSFUL_BOOT=ON
endif

ifeq ($(BR2_PACKAGE_LINUXAB_RETRY),y)
    TARGET_CFLAGS += -DRETRY_BOOT=ON
endif

LINUXAB_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(LINUXAB_BUILD_OPTS) $(LINUXAB_CONF_OPTS)" \
        PROJECT_DIR="$(@D)"

define LINUXAB_INSTALL_STAGING_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libupdateengine.so $(STAGING_DIR)/usr/lib/
    mkdir -p $(STAGING_DIR)/usr/include/libupdateengine/
    $(INSTALL) -D -m 0644 $(@D)/update.h $(STAGING_DIR)/usr/include/libupdateengine/
endef

define LINUXAB_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(LINUXAB_MAKE_OPTS)
endef

define LINUXAB_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 755 $(@D)/libupdateengine.so $(TARGET_DIR)/usr/lib/
    $(INSTALL) -D -m 755 $(@D)/rkboot_control $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 755 $(@D)/update_engine $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 755 $(@D)/S99_bootcontrol $(TARGET_DIR)/etc/init.d/
endef
$(eval $(generic-package))

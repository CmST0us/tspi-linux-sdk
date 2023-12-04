################################################################################
#
# Rockchip Power Manager For Linux
#
################################################################################

POWERMANAGER_VERSION = 1.0
POWERMANAGER_SITE = $(TOPDIR)/../external/powermanager
POWERMANAGER_SITE_METHOD = local

POWERMANAGER_LICENSE = Apache V2.0
POWERMANAGER_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
POWERMANAGER_BUILD_OPTS=-I$(PROJECT_DIR) -I$(PROJECT_DIR)/include -I$(PROJECT_DIR)/include/libxml \
	-D_GNU_SOURCE -rdynamic \
	-fPIC -funwind-tables \
	-lxml2 -lpthread

POWERMANAGER_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(POWERMANAGER_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define POWERMANAGER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(POWERMANAGER_MAKE_OPTS)
endef

define POWERMANAGER_INSTALL_TARGET_CMDS
	cp $(BUILD_DIR)/powermanager-$(POWERMANAGER_VERSION)/thermal_sensor_config.xml $(TARGET_DIR)/etc/ && \
	cp $(BUILD_DIR)/powermanager-$(POWERMANAGER_VERSION)/thermal_throttle_config.xml $(TARGET_DIR)/etc/ && \
        $(INSTALL) -D -m 755 $(@D)/power_manager_service $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 0755 $(@D)/S45PM_init $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))

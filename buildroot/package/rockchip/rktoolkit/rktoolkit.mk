################################################################################
#
# rktoolkit
#
################################################################################

RKTOOLKIT_VERSION = master
RKTOOLKIT_SITE = $(TOPDIR)/../external/rktoolkit
RKTOOLKIT_SITE_METHOD = local
RKTOOLKIT_LICENSE_FILES = LICENSE
RKTOOLKIT_LICENSE = Apache V2.0

ifeq ($(BR2_PACKAGE_VENDOR_STORAGE_LIBRARY), y)
RKTOOLKIT_VENDOR_STORAGE_LIB_NAME = libvendor_storage.so
RKTOOLKIT_VENDOR_STORAGE_INCLUDE_NAME = vendor_storage.h
define RKTOOLKIT_BUILD_VENDOR_STORAGE_LIB
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(@D)/vendor_storage.c -DBUILD_LIB_VENDOR_STORAGE -shared -fPIC -o $(@D)/$(RKTOOLKIT_VENDOR_STORAGE_LIB_NAME)
endef
RKTOOLKIT_POST_BUILD_HOOKS += RKTOOLKIT_BUILD_VENDOR_STORAGE_LIB

define RKTOOLKIT_INSTALL_LIB_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/$(RKTOOLKIT_VENDOR_STORAGE_LIB_NAME) $(TARGET_DIR)/usr/lib || true
	$(INSTALL) -D -m 755 $(@D)/$(RKTOOLKIT_VENDOR_STORAGE_INCLUDE_NAME) $(STAGING_DIR)/usr/include || true
	$(INSTALL) -D -m 755 $(@D)/$(RKTOOLKIT_VENDOR_STORAGE_LIB_NAME) $(STAGING_DIR)/usr/lib/ || true
endef
RKTOOLKIT_POST_INSTALL_TARGET_HOOKS += RKTOOLKIT_INSTALL_LIB_TARGET_CMDS
endif

define RKTOOLKIT_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(@D)/io.c -o $(@D)/io
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(@D)/update.c $(@D)/update_recv/update_recv.c -I$(@D)/update_recv/ -o $(@D)/update
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(@D)/vendor_storage.c -o $(@D)/vendor_storage
endef

define RKTOOLKIT_INSTALL_TARGET_CMDS
	[ -n "$(BR2_PACKAGE_IO)" ] && $(INSTALL) -D -m 755 $(@D)/io $(TARGET_DIR)/usr/bin/io || true
	[ -n "$(BR2_PACKAGE_UPDATE)" ] && $(INSTALL) -D -m 755 $(@D)/update $(TARGET_DIR)/usr/bin/update || true
	[ -n "$(BR2_PACKAGE_VENDOR_STORAGE)" ] && $(INSTALL) -D -m 755 $(@D)/vendor_storage $(TARGET_DIR)/usr/bin/vendor_storage || true
endef

$(eval $(generic-package))

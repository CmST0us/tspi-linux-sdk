################################################################################
#
# Rockchip Recovery For Linux
#
################################################################################

RECOVERY_VERSION = develop
RECOVERY_SITE = $(TOPDIR)/../external/recovery
RECOVERY_SITE_METHOD = local

RECOVERY_LICENSE = Apache V2.0
RECOVERY_LICENSE_FILES = NOTICE
CC="$(TARGET_CC)"
PROJECT_DIR="$(@D)"
RECOVERY_BUILD_OPTS+=-I$(PROJECT_DIR) -I$(STAGING_DIR)/usr/include/libdrm \
	--sysroot=$(STAGING_DIR) \
	-fPIC \
	-lpthread \
	-lcurl \
	-lssl \
	-lcrypto

ifeq ($(BR2_PACKAGE_RECOVERY_NO_UI),y)
	TARGET_MAKE_ENV += RecoveryNoUi=true
else
	RECOVERY_BUILD_OPTS += -lz -lpng -ldrm
	RECOVERY_DEPENDENCIES += libzlib libpng libdrm
endif

ifeq ($(BR2_PACKAGE_RECOVERY_USE_RKUPDATE),y)
	TARGET_CFLAGS += -DUSE_RKUPDATE=ON
endif

ifeq ($(BR2_PACKAGE_RECOVERY_USE_UPDATEENGINE),y)
	TARGET_CFLAGS += -DUSE_UPDATEENGINE=ON
endif

ifeq ($(BR2_PACKAGE_RECOVERY_SUCCESSFUL_BOOT),y)
	TARGET_CFLAGS += -DSUCCESSFUL_BOOT=ON
endif

ifeq ($(BR2_PACKAGE_RECOVERY_RETRY),y)
	TARGET_CFLAGS += -DRETRY_BOOT=ON
endif

RECOVERY_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(RECOVERY_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define RECOVERY_IMAGE_COPY
        mkdir -p $(TARGET_DIR)/res/images
        cp $(BUILD_DIR)/recovery-$(RECOVERY_VERSION)/res/images/* $(TARGET_DIR)/res/images/
endef

define RECOVERY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CC="$(TARGET_CC)" $(RECOVERY_MAKE_OPTS)
endef

ifeq ($(BR2_PACKAGE_RECOVERY_RECOVERYBIN),y)
define RECOVERYBIN_INSTALL_TARGET
	$(INSTALL) -D -m 755 $(@D)/recovery $(TARGET_DIR)/usr/bin/ && $(RECOVERY_IMAGE_COPY)
endef
endif

ifeq ($(BR2_PACKAGE_RECOVERY_BOOTCONTROL), y)
define BOOTCONTROLBIN_INSTALL_TARGET
	$(INSTALL) -D -m 755 $(@D)/update_engine/S99_bootcontrol $(TARGET_DIR)/etc/init.d/
endef
endif

ifeq ($(BR2_PACKAGE_RECOVERY_UPDATEENGINEBIN),y)
define UPDATEENGINEBIN_INSTALL_TARGET
	$(INSTALL) -D -m 755 $(@D)/updateEngine $(TARGET_DIR)/usr/bin/
endef
endif

define RECOVERY_INSTALL_TARGET_CMDS
	$(RECOVERYBIN_INSTALL_TARGET)
	$(UPDATEENGINEBIN_INSTALL_TARGET)
	$(BOOTCONTROLBIN_INSTALL_TARGET)
endef

ifeq ($(BR2_PACKAGE_RECOVERY_RECOVERYBIN),y)
define RECOVERY_IMAGE_PACK
	mkdir -p $(TARGET_DIR)/res/images
	cp $(BUILD_DIR)/recovery-$(RECOVERY_VERSION)/res/images/* $(TARGET_DIR)/res/images/
	$(HOST_DIR)/bin/mkbootfs $(TARGET_DIR) | $(HOST_DIR)/bin/minigzip > $(BINARIES_DIR)/ramdisk-recovery.img
	$(RECOVERY_MKBOOTIMG) --kernel $(RECOVERY_MK_KERNEL_IMAGE) --ramdisk $(BINARIES_DIR)/ramdisk-recovery.img --second $(RECOVERY_RESOURCEIMG) --os_version $(RECOVERY_OS_VERSION) --os_patch_level $(RECOVERY_OS_PATCH_LEVEL) --cmdline $(RECOVERY_CMDLINNE) --output $(BINARIES_DIR)/recovery.img
	$(RECOVERY_MK_KERNEL_IMAGE) $(BINARIES_DIR)/ramdisk-recovery.img $(BINARIES_DIR)/recovery.img
endef

#TARGET_FINALIZE_HOOKS += RECOVERY_IMAGE_PACK
endif

$(eval $(generic-package))

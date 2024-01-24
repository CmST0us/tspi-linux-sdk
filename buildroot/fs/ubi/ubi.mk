################################################################################
#
# Embed the ubifs image into an ubi image
#
################################################################################

UBI_UBINIZE_OPTS = -m $(BR2_TARGET_ROOTFS_UBI_MINIOSIZE)
UBI_UBINIZE_OPTS += -p $(BR2_TARGET_ROOTFS_UBI_PEBSIZE)
ifneq ($(BR2_TARGET_ROOTFS_UBI_SUBSIZE),0)
UBI_UBINIZE_OPTS += -s $(BR2_TARGET_ROOTFS_UBI_SUBSIZE)
endif

UBI_UBINIZE_OPTS += $(call qstrip,$(BR2_TARGET_ROOTFS_UBI_OPTS))

ifeq ($(BR2_TARGET_ROOTFS_UBI_UBIFS),y)
ROOTFS_UBI_DEPENDENCIES = rootfs-ubifs
UBI_ROOTFS_NAME=$(ROOTFS_UBIFS_FINAL_IMAGE_NAME)
UBI_VOL_TYPE=dynamic
else
ROOTFS_UBI_DEPENDENCIES = rootfs-squashfs host-mtd
UBI_ROOTFS_NAME=$(ROOTFS_SQUASHFS_FINAL_IMAGE_NAME)
UBI_VOL_TYPE=static
endif

ifeq ($(BR2_TARGET_ROOTFS_UBI_USE_CUSTOM_CONFIG),y)
UBI_UBINIZE_CONFIG_FILE_PATH = $(call qstrip,$(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE))
else
UBI_UBINIZE_CONFIG_FILE_PATH = fs/ubi/ubinize.cfg
endif

# don't use sed -i as it misbehaves on systems with SELinux enabled when this is
# executed through fakeroot (see #9386)
define ROOTFS_UBI_CMD
	sed -e 's;BR2_ROOTFS_UBI_PATH;$(BINARIES_DIR)/$(UBI_ROOTFS_NAME);' \
		-e 's;BINARIES_DIR;$(BINARIES_DIR);' \
		-e 's;UBI_VOL_TYPE;$(UBI_VOL_TYPE);' \
		$(UBI_UBINIZE_CONFIG_FILE_PATH) > $(BUILD_DIR)/ubinize.cfg
	$(HOST_DIR)/sbin/ubinize -o $@ $(UBI_UBINIZE_OPTS) $(BUILD_DIR)/ubinize.cfg
	rm $(BUILD_DIR)/ubinize.cfg
endef

$(eval $(rootfs))

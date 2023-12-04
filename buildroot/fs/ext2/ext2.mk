################################################################################
#
# Build the ext2 root filesystem image
#
################################################################################

EXT2_SIZE = $(call qstrip,$(BR2_TARGET_ROOTFS_EXT2_SIZE))
ifeq ($(BR2_TARGET_ROOTFS_EXT2)-$(EXT2_SIZE),y-)
$(error BR2_TARGET_ROOTFS_EXT2_SIZE cannot be empty)
endif

# If SIZE is AUTO, which mean shrink the filesystem to the minimum size.
# Suppose 2G is big enough for buildroot ext filesystem, we then shrink
# it after rootfs img packed.
ifeq ($(EXT2_SIZE),AUTO)
EXT2_SIZE = 2G
define ROOTFS_EXT2_SHRINK
	$(HOST_DIR)/sbin/resize2fs -M $(BINARIES_DIR)/rootfs.ext2
	$(HOST_DIR)/sbin/e2fsck -fy $(BINARIES_DIR)/rootfs.ext2
	$(HOST_DIR)/sbin/tune2fs -m $(BR2_TARGET_ROOTFS_EXT2_RESBLKS) $(BINARIES_DIR)/rootfs.ext2
	$(HOST_DIR)/sbin/resize2fs -M $(BINARIES_DIR)/rootfs.ext2
endef
endif

EXT2_MKFS_OPTS = $(call qstrip,$(BR2_TARGET_ROOTFS_EXT2_MKFS_OPTIONS))

# qstrip results in stripping consecutive spaces into a single one. So the
# variable is not qstrip-ed to preserve the integrity of the string value.
EXT2_LABEL := $(subst ",,$(BR2_TARGET_ROOTFS_EXT2_LABEL))
#" Syntax highlighting... :-/ )

EXT2_OPTS = \
	-d $(TARGET_DIR) \
	-r $(BR2_TARGET_ROOTFS_EXT2_REV) \
	-N $(BR2_TARGET_ROOTFS_EXT2_INODES) \
	-m $(BR2_TARGET_ROOTFS_EXT2_RESBLKS) \
	-L "$(EXT2_LABEL)" \
	$(EXT2_MKFS_OPTS)

ROOTFS_EXT2_DEPENDENCIES = host-e2fsprogs

define ROOTFS_EXT2_CMD
	rm -f $@
	$(HOST_DIR)/sbin/mkfs.ext$(BR2_TARGET_ROOTFS_EXT2_GEN) $(EXT2_OPTS) $@ \
		"$(EXT2_SIZE)" \
	|| { ret=$$?; \
	     echo "*** Maybe you need to increase the filesystem size (BR2_TARGET_ROOTFS_EXT2_SIZE)" 1>&2; \
	     exit $$ret; \
	}
endef

ifneq ($(BR2_TARGET_ROOTFS_EXT2_GEN),2)
define ROOTFS_EXT2_SYMLINK
	ln -sf rootfs.ext2$(ROOTFS_EXT2_COMPRESS_EXT) $(BINARIES_DIR)/rootfs.ext$(BR2_TARGET_ROOTFS_EXT2_GEN)$(ROOTFS_EXT2_COMPRESS_EXT)
endef
ROOTFS_EXT2_POST_GEN_HOOKS += ROOTFS_EXT2_SYMLINK ROOTFS_EXT2_SHRINK
endif

$(eval $(rootfs))

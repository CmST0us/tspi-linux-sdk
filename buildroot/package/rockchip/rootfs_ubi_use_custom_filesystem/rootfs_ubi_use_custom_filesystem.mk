################################################################################
#
## ubi_custom_filesystem
#
#################################################################################

CUSTOM_FILESYSTEM=$(call qstrip,$(BR2_PACKAGE_ROOTFS_UBI_CUSTOM_FILESYSTEM))

define ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_PRE_BUILD_HOOKS_CMDS
    sed 's;UBIFS_INPUT_IMAGE_PATH;$(BINARIES_DIR)/rootfs.$(CUSTOM_FILESYSTEM);' \
            $(TOPDIR)/package/rockchip/rootfs_ubi_use_custom_filesystem/ubinize.template > \
            $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE).tmp

    if [ "$(CUSTOM_FILESYSTEM)" = "squashfs" ]; then \
        sed 's;vol_type=dynamic;vol_type=static;' \
                $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE).tmp > \
                $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE); \
        else cat $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE).tmp > $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE) ; \
    fi

    rm -f $(BR2_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE).tmp

endef

# reflash itself to make sure it will be built during every build
define ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_POST_INSTALL_TARGET_HOOKS_CMDS
    rm  `dirname $@`/.stamp_downloaded
endef

ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_PRE_BUILD_HOOKS += ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_PRE_BUILD_HOOKS_CMDS
ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_POST_INSTALL_TARGET_HOOKS += ROOTFS_UBI_USE_CUSTOM_FILESYSTEM_POST_INSTALL_TARGET_HOOKS_CMDS

$(eval $(generic-package))

# Auto mount for mdev
# Author : Hans Yang <yhx@rock-chips.com>

define MDEV_MOUNT_INSTALL_TARGET_CMDS
$(INSTALL) -D -m 0755 package/rockchip/mdev_mount/sdcard_mount \
		$(TARGET_DIR)/usr/bin/sdcard_mount
$(INSTALL) -D -m 0755 package/rockchip/mdev_mount/udisk_mount \
        $(TARGET_DIR)/usr/bin/udisk_mount
$(INSTALL) -D -m 0755 package/rockchip/mdev_mount/mdev_debug \
        $(TARGET_DIR)/usr/bin/mdev_debug
        
mkdir -p $(TARGET_DIR)/mnt/sdcard
mkdir -p $(TARGET_DIR)/mnt/udisk
endef

$(eval $(generic-package))

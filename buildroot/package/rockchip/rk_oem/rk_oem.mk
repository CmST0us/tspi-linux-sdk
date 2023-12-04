################################################################################
#
# rk_oem
#
################################################################################

ifeq ($(BR2_PACKAGE_RK_OEM), y)
RK_OEM_SITE_METHOD = local
RK_OEM_IMAGE_OUTPUT = $(BASE_DIR)/images/oem.img
RK_OEM_FAKEROOT_SCRIPT = oem_fakeroot.fs
RK_OEM_FILESYSTEM_TYPE = $(BR2_PACKAGE_RK_OEM_IMAGE_FILESYSTEM_TYPE)
RK_OEM_PARTITION_SIZE = $(BR2_PACKAGE_RK_OEM_IMAGE_PARTITION_SIZE)
RK_OEM_UBI_PAGE_SIZE = $(BR2_TARGET_ROOTFS_UBI_SUBSIZE)
RK_OEM_UBI_BLOCK_SIZE = $(BR2_TARGET_ROOTFS_UBI_PEBSIZE)

ifeq ($(RK_OEM_FILESYSTEM_TYPE),"")
RK_OEM_FILESYSTEM_TYPE = ext4
endif

ifeq ($(BR2_PACKAGE_RK_OEM_RESOURCE_DIR),"")
RK_OEM_SITE = $(TOPDIR)/package/rockchip/rk_oem/src

define RK_OEM_INSTALL_INIT_SYSV
$(INSTALL) -D -m 0755 $(@D)/S98_lunch_init \
		$(TARGET_DIR)/etc/init.d/S98_lunch_init
endef

else
RK_OEM_SITE = $(BR2_PACKAGE_RK_OEM_RESOURCE_DIR)
endif

RK_OEM_REDIRECT_DBUS4OEM_CONF = package/rockchip/rk_oem/redirect_dbus4oem.conf
RK_OEM_INSTALL_TARGET_DIR = $(BR2_PACKAGE_RK_OEM_INSTALL_TARGET_DIR)

define RK_OEM_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/dbus-1/system.d
	$(INSTALL) -D -m 0644 $(RK_OEM_REDIRECT_DBUS4OEM_CONF) \
		$(TARGET_DIR)/usr/share/dbus-1/system.d
	mkdir -p $(RK_OEM_INSTALL_TARGET_DIR)
	cp -rfp $(@D)/* $(RK_OEM_INSTALL_TARGET_DIR) | true
	rm -fv $(RK_OEM_INSTALL_TARGET_DIR)/rk_oem.tar
	rm -f $(RK_OEM_INSTALL_TARGET_DIR)/$(RK_OEM_FAKEROOT_SCRIPT)
endef

define RK_OEM_TARGET_POST_CLEAN_HOOK_CMDS
	rm -rf $(RK_OEM_INSTALL_TARGET_DIR)/usr/include $(RK_OEM_INSTALL_TARGET_DIR)/usr/share/aclocal \
		$(RK_OEM_INSTALL_TARGET_DIR)/usr/lib/pkgconfig $(RK_OEM_INSTALL_TARGET_DIR)/usr/share/pkgconfig \
		$(RK_OEM_INSTALL_TARGET_DIR)/usr/lib/cmake $(RK_OEM_INSTALL_TARGET_DIR)/usr/share/cmake
	find $(RK_OEM_INSTALL_TARGET_DIR)/usr/{lib,share}/ -name '*.cmake' -print0 | xargs -0 rm -f
	find $(RK_OEM_INSTALL_TARGET_DIR)/lib/ $(RK_OEM_INSTALL_TARGET_DIR)/usr/lib/ $(RK_OEM_INSTALL_TARGET_DIR)/usr/libexec/ \
		\( -name '*.a' -o -name '*.la' \) -print0 | xargs -0 rm -f
endef

define RK_OEM_TARGET_POST_MKIMAGE_HOOK_CMDS
	echo "#!/bin/sh" > $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	echo "set -e" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	echo "export TARGET_OUTPUT_DIR=$(BASE_DIR)" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	echo "[ -d $(RK_OEM_INSTALL_TARGET_DIR)/www ] && chown -R www-data:www-data $(RK_OEM_INSTALL_TARGET_DIR)/www" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	echo "[ -d $(RK_OEM_INSTALL_TARGET_DIR)/usr/www ] && chown -R www-data:www-data $(RK_OEM_INSTALL_TARGET_DIR)/usr/www" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	echo "mkdir -p $$(dirname $(RK_OEM_IMAGE_OUTPUT))" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
	if [ $(RK_OEM_FILESYSTEM_TYPE) = "ubi" ];then \
		echo "$(TOPDIR)/../device/rockchip/common/mk-image.sh $(RK_OEM_INSTALL_TARGET_DIR) $(RK_OEM_IMAGE_OUTPUT) $(RK_OEM_FILESYSTEM_TYPE) $(RK_OEM_PARTITION_SIZE) oem $(RK_OEM_UBI_PAGE_SIZE) $(RK_OEM_UBI_BLOCK_SIZE)" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT); \
	else \
		echo "$(TOPDIR)/../device/rockchip/common/mk-image.sh $(RK_OEM_INSTALL_TARGET_DIR) $(RK_OEM_IMAGE_OUTPUT) $(RK_OEM_FILESYSTEM_TYPE) $(RK_OEM_PARTITION_SIZE) oem" >> $(@D)/$(RK_OEM_FAKEROOT_SCRIPT); \
	fi
	chmod a+x $(@D)/$(RK_OEM_FAKEROOT_SCRIPT)
endef

define RK_OEM_TARGET_FINALIZE_STRIP_HOOK_CMDS
	find $(RK_OEM_INSTALL_TARGET_DIR) -type f \( -perm /111 -o -name '*.so*' \) \
		-not \( -name 'libpthread*.so*' -o -name 'ld-*.so*' -o -name '*.ko' \) -print0 | \
		xargs -0 $(STRIPCMD) 2>/dev/null || true
	touch $(TARGET_DIR)/.auto_mkfs # format userdata partition default if mount error.
endef

ifneq ($(BR2_ENABLE_DEBUG),y)
ifneq ($(BR2_PACKAGE_RK_OEM_ENABLE_DEBUG),y)
RK_OEM_TARGET_FINALIZE_HOOKS += RK_OEM_TARGET_FINALIZE_STRIP_HOOK_CMDS
endif
endif
RK_OEM_POST_INSTALL_TARGET_HOOKS += RK_OEM_TARGET_POST_CLEAN_HOOK_CMDS

ifneq ($(RK_OEM_FILESYSTEM_TYPE),"NONE")
RK_OEM_POST_INSTALL_TARGET_HOOKS += RK_OEM_TARGET_POST_MKIMAGE_HOOK_CMDS

define RK_OEM_TARGET_FINALIZE_MKIMAGE_HOOK_CMDS
	$(HOST_DIR)/bin/fakeroot -- $(BUILD_DIR)/rk_oem/$(RK_OEM_FAKEROOT_SCRIPT)
endef
RK_OEM_TARGET_FINALIZE_HOOKS += RK_OEM_TARGET_FINALIZE_MKIMAGE_HOOK_CMDS
endif

endif

$(eval $(generic-package))

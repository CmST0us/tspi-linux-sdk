################################################################################
#
# rockchip modules
#
################################################################################

ROCKCHIP_MODULES_VERSION = 1.0.0
ROCKCHIP_MODULES_SITE_METHOD = local
ROCKCHIP_MODULES_SITE = $(TOPDIR)/package/rockchip/rockchip_modules/src

ROCKCHIP_MODULES_EXT4 = ext4.ko
ROCKCHIP_MODULES_JBD2 = jbd2.ko
ROCKCHIP_MODULES_MBCACHE2 = mbcache2.ko
ROCKCHIP_MODULES_FAT = fat.ko
ROCKCHIP_MODULES_VFAT = vfat.ko
ROCKCHIP_MODULES_NTFS = ntfs.ko
ROCKCHIP_MODULES_GSLX680_D708 = gslx680_d708.ko

ROCKCHIP_MODULES_PATH = /system/lib/modules

define ROCKCHIP_MODULES_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)$(ROCKCHIP_MODULES_PATH)
	$(INSTALL) -D -m 0755 $(@D)/S100load_modules $(TARGET_DIR)/etc/init.d
endef

copy_file_from_kernel = find $(TOPDIR)/../kernel/* -name "$(1)" | \
	xargs -n1 -i cp {} $(TARGET_DIR)$(ROCKCHIP_MODULES_PATH)
fix_script = $(SED) "/load modules/a\\	\	insmod $(ROCKCHIP_MODULES_PATH)\/$(1)" \
	$(TARGET_DIR)/etc/init.d/S100load_modules
add_new_module = $(call copy_file_from_kernel,$(1)) &&\
	$(call fix_script,$(1))

ifeq ($(BR2_PACKAGE_ROCKCHIP_MODULES_EXT4),y)
define ROCKCHIP_MODULES_INSTALL_EXT4
	$(call add_new_module,$(ROCKCHIP_MODULES_EXT4))
	$(call add_new_module,$(ROCKCHIP_MODULES_JBD2))
	$(call add_new_module,$(ROCKCHIP_MODULES_MBCACHE2))
endef
ROCKCHIP_MODULES_POST_INSTALL_TARGET_HOOKS += ROCKCHIP_MODULES_INSTALL_EXT4
endif

ifeq ($(BR2_PACKAGE_ROCKCHIP_MODULES_FAT),y)
define ROCKCHIP_MODULES_INSTALL_FAT
	$(call add_new_module,$(ROCKCHIP_MODULES_FAT))
	$(call add_new_module,$(ROCKCHIP_MODULES_VFAT))
endef
ROCKCHIP_MODULES_POST_INSTALL_TARGET_HOOKS += ROCKCHIP_MODULES_INSTALL_FAT
endif

ifeq ($(BR2_PACKAGE_ROCKCHIP_MODULES_NTFS),y)
define ROCKCHIP_MODULES_INSTALL_NTFS
	$(call add_new_module,$(ROCKCHIP_MODULES_NTFS))
endef
ROCKCHIP_MODULES_POST_INSTALL_TARGET_HOOKS += ROCKCHIP_MODULES_INSTALL_NTFS
endif

ifeq ($(BR2_PACKAGE_ROCKCHIP_MODULES_GSLX680_D708),y)
define ROCKCHIP_MODULES_INSTALL_GSLX680_D708
	$(call add_new_module,$(ROCKCHIP_MODULES_GSLX680_D708))
endef
ROCKCHIP_MODULES_POST_INSTALL_TARGET_HOOKS += ROCKCHIP_MODULES_INSTALL_GSLX680_D708
endif

$(eval $(generic-package))

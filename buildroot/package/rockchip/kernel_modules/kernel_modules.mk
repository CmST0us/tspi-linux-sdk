################################################################################
#
# kernel_modules
#
################################################################################

# This package is used to build rockchip kernel modules and install into rootfs

KERNEL_MODULES_LICENSE = GPL-2.0
KERNEL_MODULES_LICENSE_FILES = COPYING

KERNEL_MODULES_SITE = $(TOPDIR)/../kernel
KERNEL_MODULES_SITE_METHOD = local

KERNEL_MODULES_KCONFIG_DEFCONFIG = $(call qstrip,$(BR2_PACKAGE_KERNEL_DEFCONFIG))_defconfig

KERNEL_MODULES_MAKE_FLAGS = \
	HOSTCC="$(HOSTCC)" \
	HOSTCFLAGS="$(HOSTCFLAGS)" \
        ARCH=$(KERNEL_ARCH) \
        CROSS_COMPILE="$(TARGET_CROSS)" \
	INSTALL_MOD_STRIP=1 \
	INSTALL_MOD_PATH="$(TARGET_DIR)"

define KERNEL_MODULES_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(KERNEL_MODULES_MAKE_FLAGS) -C $(@D) $(KERNEL_MODULES_KCONFIG_DEFCONFIG)
	$(TARGET_MAKE_ENV) $(MAKE) $(KERNEL_MODULES_MAKE_FLAGS) -C $(@D) vmlinux
	$(TARGET_MAKE_ENV) $(MAKE) $(KERNEL_MODULES_MAKE_FLAGS) -C $(@D) modules
endef

KERNEL_MODULES_VERSION_PROBED = `$(MAKE) $(KERNEL_MODULES_MAKE_FLAGS) -C $(@D) --no-print-directory -s kernelrelease 2>/dev/null`

define KERNEL_MODULES_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(KERNEL_MODULES_MAKE_FLAGS) -C $(@D) modules_install
	rm -f $(TARGET_DIR)/lib/modules/$(KERNEL_MODULES_VERSION_PROBED)/build
	rm -f $(TARGET_DIR)/lib/modules/$(KERNEL_MODULES_VERSION_PROBED)/source
endef

$(eval $(kconfig-package))

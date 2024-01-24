# add test tool for rockchip platform

ROCKCHIP_TEST_SITE = $(TOPDIR)/../external/rockchip-test
ROCKCHIP_TEST_VERSION = master
ROCKCHIP_TEST_SITE_METHOD = local
ROCKCHIP_TEST_LICENSE = ROCKCHIP
ROCKCHIP_TEST_LICENSE_FILES = LICENSE

define ROCKCHIP_TEST_INSTALL_TARGET_CMDS
	mkdir -p ${TARGET_DIR}/rockchip-test
	cp -rf  $(@D)/*  ${TARGET_DIR}/rockchip-test/
endef

define ROCKCHIP_TEST_INSTALL_INIT_SYSV
        $(INSTALL) -D -m 0755 $(@D)/auto_reboot/S99-auto-reboot \
		$(TARGET_DIR)/etc/init.d/
endef

define ROCKCHIP_TEST_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(@D)/auto_reboot/autoreboot.service \
		$(TARGET_DIR)/usr/lib/systemd/system/
endef

$(eval $(generic-package))

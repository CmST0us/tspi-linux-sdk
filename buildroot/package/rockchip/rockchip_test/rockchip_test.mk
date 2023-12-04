# add test tool for rockchip platform
# Author : Hans Yang <yhx@rock-chips.com>

ROCKCHIP_TEST_VERSION = 20220105
ROCKCHIP_TEST_SITE_METHOD = local
ROCKCHIP_TEST_SITE = $(TOPDIR)/package/rockchip/rockchip_test/src
ROCKCHIP_TEST_LICENSE = Apache V2.0
ROCKCHIP_TEST_LICENSE_FILES = NOTICE

ifeq ($(BR2_PACKAGE_RK356X),y)
ROCKCHIP_TEST_NPU_SOURCE = RK356X/npu2_${ARCH}
ROCKCHIP_TEST_NPU_TARGET = npu2
else ifeq ($(BR2_PACKAGE_RK3588),y)
ROCKCHIP_TEST_NPU_SOURCE = RK3588/npu2_${ARCH}
ROCKCHIP_TEST_NPU_TARGET = npu2
else
ROCKCHIP_TEST_NPU_SOURCE = npu_${ARCH}
ROCKCHIP_TEST_NPU_TARGET = npu
endif

ifeq ($(BR2_PACKAGE_RKNPU)$(BR2_PACKAGE_RKNPU2),y)
define ROCKCHIP_TEST_INSTALL_NPU_TARGET_CMDS
	rm -rf ${TARGET_DIR}/rockchip_test/npu
	cp -rf $(@D)/$(ROCKCHIP_TEST_NPU_SOURCE) ${TARGET_DIR}/rockchip_test/$(ROCKCHIP_TEST_NPU_TARGET)
endef
ROCKCHIP_TEST_POST_INSTALL_TARGET_HOOKS = ROCKCHIP_TEST_INSTALL_NPU_TARGET_CMDS
endif

define ROCKCHIP_TEST_INSTALL_TARGET_CMDS
	cp -rf  $(@D)/rockchip_test  ${TARGET_DIR}/
	cp -rf $(@D)/rockchip_test_${ARCH}/* ${TARGET_DIR}/rockchip_test/ || true
	$(INSTALL) -D -m 0755 $(@D)/rockchip_test/auto_reboot/S99_auto_reboot $(TARGET_DIR)/etc/init.d/
	test "${ROCKCHIP_TEST_NPU_TARGET}" = "npu2" && \
		$(INSTALL) -D -m 0755 $(@D)/npu2_aarch64/rknn_mobilenet_demo $(TARGET_DIR)/rockchip_test/npu2/ || true
endef

$(eval $(generic-package))

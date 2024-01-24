################################################################################
#
# rknpu
#
################################################################################
RKNPU_VERSION = 1.5.0
RKNPU_SITE_METHOD = local
RKNPU_SITE = $(TOPDIR)/../external/rknpu
NPU_TEST_FILE = $(@D)/test
RKNPU_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_RKNPU_PCIE),y)
NPU_KO_FILE = galcore_rk3399pro-npu-pcie.ko
else ifeq ($(BR2_PACKAGE_RK3399PRO_NPU),y)
NPU_KO_FILE = galcore_rk3399pro-npu.ko
else ifeq ($(BR2_PACKAGE_RK1806),y)
NPU_KO_FILE = galcore_rk1806.ko
else ifeq ($(BR2_PACKAGE_RV1126_RV1109),y)
ifeq ($(BR2_PACKAGE_RKNPU_USE_MINI_DRIVER),y)
NPU_KO_FILE = galcore_puma_tb.ko
else
NPU_KO_FILE = galcore_puma.ko
endif
else
NPU_KO_FILE = galcore.ko
endif

ifeq ($(BR2_arm),y)
NPU_PLATFORM_ARCH := linux-armhf
else
NPU_PLATFORM_ARCH := linux-aarch64
endif

ifeq ($(BR2_PACKAGE_RV1126_RV1109),y)
NPU_PLATFORM_ARCH := linux-armhf-puma
endif

ifeq ($(BR2_PACKAGE_RKNPU_USE_MINI_DRIVER), y)
NPU_PLATFORM := $(NPU_PLATFORM_ARCH)-mini
BUILD_NOT_START_RKNN_SCRIPT=y
else
NPU_PLATFORM := $(NPU_PLATFORM_ARCH)
endif

ifeq ($(BR2_PACKAGE_PYTHON_RKNN), y)
BUILD_PYTHON_RKNN=y
endif

ifeq ($(BR2_PACKAGE_RKNPU_USE_RKNN_API), y)
BUILD_RKNN_API=y
endif

define RKNPU_INSTALL_STAGING_CMDS
    mkdir -p $(STAGING_DIR)/usr/include/rknn
    $(INSTALL) -D -m 0644 $(@D)/rknn/include/rknn_runtime.h $(STAGING_DIR)/usr/include/rknn/rknn_runtime.h
    if [ ${BUILD_RKNN_API} = "y" ]; then \
        $(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api/include/rknn_api.h $(STAGING_DIR)/usr/include/rknn/rknn_api.h; \
    fi
endef

define RKNPU_INSTALL_TARGET_CMDS
    mkdir -p $(TARGET_DIR)/lib/modules/
    mkdir -p $(TARGET_DIR)/usr/share/npu/
    $(INSTALL) -D -m 0644 $(@D)/drivers/npu_ko/$(NPU_KO_FILE) $(TARGET_DIR)/lib/modules/galcore.ko
    cp -r $(@D)/drivers/common/* $(TARGET_DIR)/
    cp -r $(@D)/drivers/common/* $(STAGING_DIR)/

    if [ x${BUILD_NOT_START_RKNN_SCRIPT} != x ]; then \
        rm $(TARGET_DIR)/etc/init.d/S60NPU_init; \
    else \
        rm $(TARGET_DIR)/etc/init.d/S05NPU_init; \
    fi

    cp -r $(@D)/drivers/$(NPU_PLATFORM)/* $(TARGET_DIR)/
    cp -r $(@D)/drivers/$(NPU_PLATFORM)/* $(STAGING_DIR)/

    if [ -e "$(@D)/test" ]; then \
        cp -r $(@D)/test $(TARGET_DIR)/usr/share/npu; \
    fi

    if [ x${BUILD_PYTHON_RKNN} != x ]; then \
        cp -r $(@D)/rknn/python/rknn $(TARGET_DIR)/usr/lib/python3.6/site-packages/; \
    fi

    if [ ${BUILD_RKNN_API} = "y" ]; then \
        if [ ${NPU_PLATFORM_ARCH} = "linux-aarch64" ]; then \
            $(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api/lib64/librknn_api.so $(TARGET_DIR)/usr/lib/; \
            $(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api/lib64/librknn_api.so $(STAGING_DIR)/usr/lib; \
        else \
            $(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api/lib/librknn_api.so $(TARGET_DIR)/usr/lib/; \
            $(INSTALL) -D -m 0644 $(@D)/rknn/rknn_api/librknn_api/lib/librknn_api.so $(STAGING_DIR)/usr/lib; \
        fi \
    fi
endef

$(eval $(generic-package))

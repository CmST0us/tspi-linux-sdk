################################################################################
#
# rockchip-mpp
#
################################################################################

ROCKCHIP_MPP_SITE = $(TOPDIR)/../external/mpp
ROCKCHIP_MPP_VERSION = develop
ROCKCHIP_MPP_SITE_METHOD = local

ROCKCHIP_MPP_LICENSE = Apache-2.0
ROCKCHIP_MPP_LICENSE_FILES = LICENSE.md

ROCKCHIP_MPP_CONF_OPTS = "-DRKPLATFORM=ON"
ROCKCHIP_MPP_DEPENDENCIES += libdrm

ROCKCHIP_MPP_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_ROCKCHIP_MPP_ALLOCATOR_DRM),y)
ROCKCHIP_MPP_CONF_OPTS += "-DHAVE_DRM=ON"
endif

ifeq ($(BR2_PACKAGE_ROCKCHIP_MPP_TESTS),y)
ROCKCHIP_MPP_CONF_OPTS += "-DBUILD_TEST=ON"
endif

define ROCKCHIP_MPP_LINK_GIT
	rm -rf $(@D)/.git
	ln -s $(SRCDIR)/.git $(@D)/
endef
ROCKCHIP_MPP_POST_RSYNC_HOOKS += ROCKCHIP_MPP_LINK_GIT

ifeq ($(BR2_PACKAGE_RK3328),y)
define ROCKCHIP_MPP_H265_SUPPORTED_FIRMWARE
	mkdir -p $(TARGET_DIR)/lib/firmware/

	if test -e $(ROCKCHIP_MPP_SITE)/../rktoolkit/monet.bin ; then \
		$(INSTALL) -m 0644 -D $(ROCKCHIP_MPP_SITE)/../rktoolkit/monet.bin \
			$(TARGET_DIR)/lib/firmware/ ; \
	else \
		$(INSTALL) -m 0644 -D package/rockchip/rockchip-mpp/monet.bin \
			$(TARGET_DIR)/lib/firmware/ ; \
	fi
endef
ROCKCHIP_MPP_POST_INSTALL_TARGET_HOOKS += ROCKCHIP_MPP_H265_SUPPORTED_FIRMWARE
endif

ifeq ($(BR2_PACKAGE_RK_OEM), y)
ifneq ($(BR2_PACKAGE_THUNDERBOOT), y)
ROCKCHIP_MPP_INSTALL_TARGET_OPTS = DESTDIR=$(BR2_PACKAGE_RK_OEM_INSTALL_TARGET_DIR) install/fast
endif
endif

define ROCKCHIP_MPP_REMOVE_NOISY_LOGS
	sed -i -e "/pp_enable %d/d" \
		$(@D)/mpp/hal/vpu/jpegd/hal_jpegd_vdpu2.c || true
	sed -i -e "/reg size mismatch wr/i    if (0)" \
		$(@D)/osal/driver/vcodec_service.c || true
endef
ROCKCHIP_MPP_POST_RSYNC_HOOKS += ROCKCHIP_MPP_REMOVE_NOISY_LOGS

$(eval $(cmake-package))

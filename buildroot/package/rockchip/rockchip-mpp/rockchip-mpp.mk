################################################################################
#
# rockchip-mpp
#
################################################################################

ROCKCHIP_MPP_SITE = $(TOPDIR)/../external/mpp
ROCKCHIP_MPP_VERSION = develop
ROCKCHIP_MPP_SITE_METHOD = local

ROCKCHIP_MPP_LICENSE = Apache-2.0, MIT
ROCKCHIP_MPP_LICENSE_FILES = LICENSES/Apache-2.0 LICENSES/MIT

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

define ROCKCHIP_MPP_REMOVE_NOISY_LOGS
	sed -i -e "/pp_enable %d/d" \
		$(@D)/mpp/hal/vpu/jpegd/hal_jpegd_vdpu2.c || true
	sed -i -e "/reg size mismatch wr/i    if (0)" \
		$(@D)/osal/driver/vcodec_service.c || true
endef
ROCKCHIP_MPP_POST_RSYNC_HOOKS += ROCKCHIP_MPP_REMOVE_NOISY_LOGS

$(eval $(cmake-package))

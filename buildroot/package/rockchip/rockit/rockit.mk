################################################################################
#
# rockit project
#
################################################################################

ROCKIT_SITE = $(TOPDIR)/../external/rockit

ROCKIT_SITE_METHOD = local

ROCKIT_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_ROCKCHIP_MPP),y)
ROCKIT_DEPENDENCIES += rockchip-mpp
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKAIQ), y)
ROCKIT_DEPENDENCIES += camera_engine_rkaiq
ROCKIT_CONF_OPTS += -DUSE_RKAIQ=ON
endif

ifeq ($(BR2_PACKAGE_AISERVER_USE_STASTERIA), y)
ROCKIT_CONF_OPTS += -DUSE_STASTERIA=ON
endif

ifeq ($(BR2_PACKAGE_ROCKIT_TGI),y)
ROCKIT_CONF_OPTS += -DUSE_ROCKIT_TGI=ON
endif

ifeq ($(BR2_PACKAGE_ROCKIT_MPI),y)
ROCKIT_CONF_OPTS += -DUSE_ROCKIT_MPI=ON
endif

ifeq ($(BR2_PACKAGE_AISERVER_USE_ROCKX), y)
ROCKIT_CONF_OPTS += -DUSE_ROCKX=ON
else
ROCKIT_CONF_OPTS += -DUSE_ROCKX=OFF
endif

$(eval $(cmake-package))

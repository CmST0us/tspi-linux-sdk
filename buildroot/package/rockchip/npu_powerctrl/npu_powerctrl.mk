################################################################################
#
# npu_powerctrl
#
################################################################################

NPU_POWERCTRL_LICENSE_FILES = NOTICE
NPU_POWERCTRL_LICENSE = Apache V2.0
NPU_POWERCTRL_SITE = $(TOPDIR)/package/rockchip/npu_powerctrl
NPU_POWERCTRL_SITE_METHOD = local
NPU_POWERCTRL_LICENSE = Apache V2.0
NPU_POWERCTRL_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"
NPU_POWERCTRL_BUILD_OPTS=-I$(PROJECT_DIR) -fPIC \
	--sysroot=$(STAGING_DIR) \
	-ldl -lpthread

NPU_POWERCTRL_MAKE_OPTS = \
        CXXFLAGS="$(TARGET_CPPFLAGS) $(NPU_POWERCTRL_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define NPU_POWERCTRL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(NPU_POWERCTRL_MAKE_OPTS)
endef

define NPU_POWERCTRL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/npu_powerctrl $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))

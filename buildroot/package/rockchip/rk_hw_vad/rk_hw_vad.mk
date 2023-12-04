################################################################################
#
# Rockchip CAE APP USING VAD
#
################################################################################

RK_HW_VAD_VERSION = 1.0
RK_HW_VAD_SITE = $(TOPDIR)/../external/rk_hw_vad
RK_HW_VAD_SITE_METHOD = local

RK_HW_VAD_LICENSE = Apache V2.0
RK_HW_VAD_LICENSE_FILES = NOTICE

define RK_HW_VAD_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef
define RK_HW_VAD_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

define RK_HW_VAD_CLEAN_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

define RK_HW_VAD_UNINSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))

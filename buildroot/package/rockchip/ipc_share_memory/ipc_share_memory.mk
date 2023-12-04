################################################################################
#
# Rockchip CAE APP USING VAD
#
################################################################################

IPC_SHARE_MEMORY_VERSION = 1.0
IPC_SHARE_MEMORY_SITE = $(TOPDIR)/../external/ipc_share_memory
IPC_SHARE_MEMORY_SITE_METHOD = local

IPC_SHARE_MEMORY_LICENSE = Apache V2.0
IPC_SHARE_MEMORY_LICENSE_FILES = NOTICE

define IPC_SHARE_MEMORY_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) CXX=$(TARGET_CXX) -C $(@D)
endef
define IPC_SHARE_MEMORY_INSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
endef

define IPC_SHARE_MEMORY_CLEAN_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

define IPC_SHARE_MEMORY_UNINSTALL_TARGET_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) uninstall
endef

$(eval $(generic-package))

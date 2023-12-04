SYSTEM_MANAGER_SITE = $(TOPDIR)/../app/system_manager
SYSTEM_MANAGER_SITE_METHOD = local
SYSTEM_MANAGER_INSTALL_STAGING = YES

# add dependencies
SYSTEM_MANAGER_DEPENDENCIES = adk messenger

define SYSTEM_MANAGER_INSTALL_INIT_SYSV
    $(INSTALL) -m 0755 -D package/rockchip/rv1108_package/app/system_manager/S10_system_manager \
		    $(TARGET_DIR)/etc/init.d/
    $(INSTALL) -m 0644 -D package/rockchip/rv1108_package/app/system_manager/system_manager.conf \
                    $(TARGET_DIR)/etc/system_manager.conf
endef

$(eval $(cmake-package))

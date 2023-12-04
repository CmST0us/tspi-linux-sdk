################################################################################
#
# shm-tools
#
################################################################################

SHM_TOOLS_VERSION = f1dd16803e50d97e66d4ac1143a762977f19c8cf
SHM_TOOLS_SITE = http://github.com/mikewei/shm_container.git
SHM_TOOLS_SITE_METHOD = git
SHM_TOOLS_INSTALL_STAGING = YES
SHM_TOOLS_LICENSE = BSD-like

define SHM_TOOLS_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/include/shmc
	$(INSTALL) -D -m 0644 $(@D)/src/shmc/*.h $(STAGING_DIR)/usr/include/shmc
endef

$(eval $(generic-package))

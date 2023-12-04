################################################################################
#
# Rockchip EQ DRC PROCESS
#
################################################################################

EQ_DRC_PROCESS_VERSION = 1.1
EQ_DRC_PROCESS_SITE = $(TOPDIR)/../external/eq_drc_process
EQ_DRC_PROCESS_SITE_METHOD = local

EQ_DRC_PROCESS_LICENSE = Apache V2.0
EQ_DRC_PROCESS_LICENSE_FILES = NOTICE
CXX="$(TARGET_CXX)"
PROJECT_DIR="$(@D)"
EQ_DRC_PROCESS_BUILD_OPTS=-I$(PROJECT_DIR) -fPIC -O3 -I$(PROJECT_DIR)/include \
	--sysroot=$(STAGING_DIR) \
	-ldl -lpthread -lasound

EQ_DRC_PROCESS_MAKE_OPTS = \
        CPPFLAGS="$(TARGET_CPPFLAGS) $(EQ_DRC_PROCESS_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)"

define EQ_DRC_PROCESS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) CXX="$(TARGET_CXX)" $(EQ_DRC_PROCESS_MAKE_OPTS)
endef

define EQ_DRC_PROCESS_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/eq_drc_process $(TARGET_DIR)/usr/bin/
	$(INSTALL) -D -m 755 $(@D)/S97_EQ_init $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))

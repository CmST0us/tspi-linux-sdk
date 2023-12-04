################################################################################
#
# carmachine
#
################################################################################

CARMACHINE_VERSION = 1.0
CARMACHINE_SITE = $(TOPDIR)/../app/carmachine
CARMACHINE_SITE_METHOD = local

CARMACHINE_LICENSE = Apache V2.0
CARMACHINE_LICENSE_FILES = NOTICE

define CARMACHINE_CONFIGURE_CMDS
	cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake
endef

define CARMACHINE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CARMACHINE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/Carmachine \
		$(TARGET_DIR)/usr/bin/Carmachine
endef

$(eval $(generic-package))

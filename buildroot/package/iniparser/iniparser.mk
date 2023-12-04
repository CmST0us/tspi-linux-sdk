################################################################################
#
# iniparser
#
################################################################################

INIPARSER_VERSION = 4.1
INIPARSER_SITE = $(call github,ndevilla,iniparser,v$(INIPARSER_VERSION))
INIPARSER_INSTALL_STAGING = YES
INIPARSER_LICENSE = MIT
INIPARSER_LICENSE_FILES = LICENSE

define INIPARSER_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) SO_TARGET=libiniparser.so -C $(@D)
endef

define INIPARSER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libiniparser.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -D -m 0644 $(@D)/src/*.h $(TARGET_DIR)/usr/include
endef

define INIPARSER_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libiniparser.so $(STAGING_DIR)/usr/lib/
	$(INSTALL) -D -m 0644 $(@D)/src/*.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))
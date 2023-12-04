################################################################################
#
# Rockchip Rkaiq Tool Server For Linux
#
################################################################################

ifeq ($(BR2_PACKAGE_RKAIQ_TOOL_SERVER), y)
RKAIQ_TOOL_SERVER_VERSION = 1.0
RKAIQ_TOOL_SERVER_SITE = $(TOPDIR)/../app/rkaiq_tool_server
RKAIQ_TOOL_SERVER_SITE_METHOD = local
RKAIQ_TOOL_SERVER_INSTALL_STAGING = YES

RKAIQ_TOOL_SERVER_LICENSE = Apache V2.0
RKAIQ_TOOL_SERVER_LICENSE_FILES = NOTICE

RKAIQ_TOOL_SERVER_TARGET_INSTALL_DIR = $(TARGET_DIR)

RKAIQ_TOOL_SERVER_CONF_OPTS = -DBUILDROOT_BUILD_PROJECT=TRUE -DARCH=$(BR2_ARCH)


ifeq ($(BR2_PACKAGE_RV1126_RV1109),y)
RKAIQ_TOOL_SERVER_CONF_OPTS += -DISP_HW_VERSION=-DISP_HW_V20
else ifeq ($(BR2_PACKAGE_RK356X),y)
RKAIQ_TOOL_SERVER_CONF_OPTS += -DISP_HW_VERSION=-DISP_HW_V21
endif

define RKAIQ_TOOL_SERVER_INSTALL_TARGET_CMDS
	mkdir -p $(RKAIQ_TOOL_SERVER_TARGET_INSTALL_DIR)/usr/bin/
	mkdir -p $(RKAIQ_TOOL_SERVER_TARGET_INSTALL_DIR)/usr/lib/
	$(INSTALL) -D -m  644 $(@D)/rkmedia/librkmedia/librkmedia.so $(RKAIQ_TOOL_SERVER_TARGET_INSTALL_DIR)/usr/lib/
	$(INSTALL) -D -m  755 $(@D)/rkaiq_tool_server $(RKAIQ_TOOL_SERVER_TARGET_INSTALL_DIR)/usr/bin/
endef

$(eval $(cmake-package))
$(eval $(host-generic-package))

endif

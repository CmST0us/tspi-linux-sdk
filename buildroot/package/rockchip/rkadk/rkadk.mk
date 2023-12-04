RKADK_SITE = $(TOPDIR)/../external/rkadk
RKADK_SITE_METHOD = local
RKADK_INSTALL_STAGING = YES

RKADK_DEPENDENCIES += rkmedia rkfsmk iniparser cjson

ifeq ($(BR2_PACKAGE_RKADK_ROCKIT), y)
RKADK_DEPENDENCIES += rockit
RKADK_CONF_OPTS += -DUSE_ROCKIT=ON \
	-DROCKIT_HEADER_DIR=$(STAGING_DIR)/usr/include
endif

ifeq ($(BR2_PACKAGE_CAMERA_ENGINE_RKAIQ), y)
RKADK_DEPENDENCIES += camera_engine_rkaiq
RKADK_CONF_OPTS += -DUSE_RKAIQ=ON
endif

RKADK_CONF_OPTS += -DRKMEDIA_HEADER_DIR=$(TOPDIR)/../external/rkmedia/include/rkmedia

RKADK_CONF_OPTS += -DCMAKE_INSTALL_STAGING=$(STAGING_DIR)

define RKADK_CP_DEF_SETTING_FILE
	mkdir -p $(TARGET_DIR)/etc/rkadk
	cp -rfp $(@D)/inicfg/rkadk_defsetting.ini $(TARGET_DIR)/etc/rkadk/ | true
	cp -rfp $(@D)/inicfg/rkadk_defsetting_sensor_0.ini $(TARGET_DIR)/etc/rkadk/ | true
	cp -rfp $(@D)/inicfg/rkadk_defsetting_sensor_1.ini $(TARGET_DIR)/etc/rkadk/ | true
endef

RKADK_POST_INSTALL_TARGET_HOOKS += RKADK_CP_DEF_SETTING_FILE

$(eval $(cmake-package))

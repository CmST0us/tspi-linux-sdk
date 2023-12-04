ifeq ($(BR2_PACKAGE_UAC_APP), y)
    UAC_APP_SITE = $(TOPDIR)/../external/uac_app
    UAC_APP_SITE_METHOD = local
    UAC_APP_INSTALL_STAGING = YES
    UAC_APP_DEPENDENCIES = rockit
    $(eval $(cmake-package))
endif

ifeq ($(BR2_PACKAGE_RK_OEM), y)
UAC_APP_INSTALL_TARGET_OPTS = DESTDIR=$(BR2_PACKAGE_RK_OEM_INSTALL_TARGET_DIR) install/fast
UAC_APP_DEPENDENCIES += rk_oem
UAC_APP_CONF_OPTS += -DUAC_APP_CONF_PREFIX="\"/oem\""
endif

ifeq ($(BR2_PACKAGE_UAC_APP_ALSA_PLAYBACK), y)
UAC_APP_DEPENDENCIES += libasound
endif

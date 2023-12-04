###############################################################################
#
# Source Han Sans CN
#
###############################################################################

SOURCE_HAN_SANS_CN_VERSION = $(SOURCE_HAN_SANS_VERSION)
SOURCE_HAN_SANS_CN_SOURCE = SourceHanSansCN.zip
SOURCE_HAN_SANS_CN_SITE = $(SOURCE_HAN_SANS_SITE)
SOURCE_HAN_SANS_CN_LICENSE = LICENSE.txt
SOURCE_HAN_SANS_CN_DEPENDENCIES = host-zip

define SOURCE_HAN_SANS_CN_EXTRACT_CMDS
	unzip $(DL_DIR)/$(SOURCE_HAN_SANS_CN_SOURCE) -d $(@D)/
endef

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define SOURCE_HAN_SANS_CN_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 \
		$(SOURCE_HAN_SANS_CN_PKGDIR)/44-source-han-sans-cn.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

define SOURCE_HAN_SANS_CN_INSTALL_TARGET_CMDS
	cp -r $(@D)/SourceHanSansCN $(TARGET_DIR)/usr/share/fonts/source-han-sans-cn
	$(SOURCE_HAN_SANS_CN_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

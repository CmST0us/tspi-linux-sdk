###############################################################################
#
# Source Han Sans JP
#
###############################################################################

SOURCE_HAN_SANS_JP_VERSION = $(SOURCE_HAN_SANS_VERSION)
SOURCE_HAN_SANS_JP_SOURCE = SourceHanSansJP.zip
SOURCE_HAN_SANS_JP_SITE = $(SOURCE_HAN_SANS_SITE)
SOURCE_HAN_SANS_JP_LICENSE = LICENSE.txt
SOURCE_HAN_SANS_JP_DEPENDENCIES = host-zip

define SOURCE_HAN_SANS_JP_EXTRACT_CMDS
	unzip $(DL_DIR)/$(SOURCE_HAN_SANS_JP_SOURCE) -d $(@D)/
endef

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define SOURCE_HAN_SANS_JP_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 \
		$(SOURCE_HAN_SANS_JP_PKGDIR)/44-source-han-sans-jp.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

define SOURCE_HAN_SANS_JP_INSTALL_TARGET_CMDS
	cp -r $(@D)/SourceHanSansJP $(TARGET_DIR)/usr/share/fonts/source-han-sans-jp
	$(SOURCE_HAN_SANS_JP_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

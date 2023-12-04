###############################################################################
#
# Source Han Sans TW
#
###############################################################################

SOURCE_HAN_SANS_TW_VERSION = $(SOURCE_HAN_SANS_VERSION)
SOURCE_HAN_SANS_TW_SOURCE = SourceHanSansTW.zip
SOURCE_HAN_SANS_TW_SITE = $(SOURCE_HAN_SANS_SITE)
SOURCE_HAN_SANS_TW_LICENSE = LICENSE.txt
SOURCE_HAN_SANS_TW_DEPENDENCIES = host-zip

define SOURCE_HAN_SANS_TW_EXTRACT_CMDS
	unzip $(DL_DIR)/$(SOURCE_HAN_SANS_TW_SOURCE) -d $(@D)/
endef

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define SOURCE_HAN_SANS_TW_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 \
		$(SOURCE_HAN_SANS_TW_PKGDIR)/44-source-han-sans-tw.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

define SOURCE_HAN_SANS_TW_INSTALL_TARGET_CMDS
	cp -r $(@D)/SourceHanSansTW $(TARGET_DIR)/usr/share/fonts/source-han-sans-tw
	$(SOURCE_HAN_SANS_TW_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

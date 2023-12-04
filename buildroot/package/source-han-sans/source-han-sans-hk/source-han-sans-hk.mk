###############################################################################
#
# Source Han Sans HK
#
###############################################################################

SOURCE_HAN_SANS_HK_VERSION = $(SOURCE_HAN_SANS_VERSION)
SOURCE_HAN_SANS_HK_SOURCE = SourceHanSansHK.zip
SOURCE_HAN_SANS_HK_SITE = $(SOURCE_HAN_SANS_SITE)
SOURCE_HAN_SANS_HK_LICENSE = LICENSE.txt
SOURCE_HAN_SANS_HK_DEPENDENCIES = host-zip

define SOURCE_HAN_SANS_HK_EXTRACT_CMDS
	unzip $(DL_DIR)/$(SOURCE_HAN_SANS_HK_SOURCE) -d $(@D)/
endef

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define SOURCE_HAN_SANS_HK_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 \
		$(SOURCE_HAN_SANS_HK_PKGDIR)/44-source-han-sans-hk.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

define SOURCE_HAN_SANS_HK_INSTALL_TARGET_CMDS
	cp -r $(@D)/SourceHanSansHK $(TARGET_DIR)/usr/share/fonts/source-han-sans-hk
	$(SOURCE_HAN_SANS_HK_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

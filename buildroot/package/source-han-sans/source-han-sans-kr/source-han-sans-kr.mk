###############################################################################
#
# Source Han Sans KR
#
###############################################################################

SOURCE_HAN_SANS_KR_VERSION = $(SOURCE_HAN_SANS_VERSION)
SOURCE_HAN_SANS_KR_SOURCE = SourceHanSansKR.zip
SOURCE_HAN_SANS_KR_SITE = $(SOURCE_HAN_SANS_SITE)
SOURCE_HAN_SANS_KR_LICENSE = LICENSE.txt
SOURCE_HAN_SANS_KR_DEPENDENCIES = host-zip

define SOURCE_HAN_SANS_KR_EXTRACT_CMDS
	unzip $(DL_DIR)/$(SOURCE_HAN_SANS_KR_SOURCE) -d $(@D)/
endef

ifeq ($(BR2_PACKAGE_FONTCONFIG),y)
define SOURCE_HAN_SANS_KR_INSTALL_FONTCONFIG_CONF
	$(INSTALL) -D -m 0644 \
		$(SOURCE_HAN_SANS_KR_PKGDIR)/44-source-han-sans-kr.conf \
		$(TARGET_DIR)/usr/share/fontconfig/conf.avail/
endef
endif

define SOURCE_HAN_SANS_KR_INSTALL_TARGET_CMDS
	cp -r $(@D)/SourceHanSansKR $(TARGET_DIR)/usr/share/fonts/source-han-sans-kr
	$(SOURCE_HAN_SANS_KR_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

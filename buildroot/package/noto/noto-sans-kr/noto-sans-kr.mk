###############################################################################
#
# Noto Sans SC
#
###############################################################################

NOTO_SANS_KR_VERSION = 1.1
NOTO_SANS_KR_SOURCE = NotoSansKR.zip
NOTO_SANS_KR_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SANS_KR_LICENSE = LICENSE_OFL.txt
NOTO_SANS_KR_DEPENDENCIES = host-zip

define NOTO_SANS_KR_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SANS_KR_SOURCE) -d $(@D)/
endef

define NOTO_SANS_KR_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-sans-kr/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-sans-kr/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

###############################################################################
#
# Noto Serif SC
#
###############################################################################

NOTO_SERIF_KR_VERSION = 1.1
NOTO_SERIF_KR_SOURCE = NotoSerifKR.zip
NOTO_SERIF_KR_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SERIF_KR_LICENSE = LICENSE_OFL.txt
NOTO_SERIF_KR_DEPENDENCIES = host-zip

define NOTO_SERIF_KR_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SERIF_KR_SOURCE) -d $(@D)/
endef

define NOTO_SERIF_KR_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-serif-kr/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-serif-kr/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

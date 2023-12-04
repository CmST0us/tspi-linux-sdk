###############################################################################
#
# Noto Sans SC
#
###############################################################################

NOTO_SANS_JP_VERSION = 1.1
NOTO_SANS_JP_SOURCE = NotoSansJP.zip
NOTO_SANS_JP_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SANS_JP_LICENSE = LICENSE_OFL.txt
NOTO_SANS_JP_DEPENDENCIES = host-zip

define NOTO_SANS_JP_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SANS_JP_SOURCE) -d $(@D)/
endef

define NOTO_SANS_JP_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-sans-jp/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-sans-jp/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

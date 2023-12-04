###############################################################################
#
# Noto Serif SC
#
###############################################################################

NOTO_SERIF_JP_VERSION = 1.1
NOTO_SERIF_JP_SOURCE = NotoSerifJP.zip
NOTO_SERIF_JP_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SERIF_JP_LICENSE = LICENSE_OFL.txt
NOTO_SERIF_JP_DEPENDENCIES = host-zip

define NOTO_SERIF_JP_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SERIF_JP_SOURCE) -d $(@D)/
endef

define NOTO_SERIF_JP_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-serif-jp/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-serif-jp/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

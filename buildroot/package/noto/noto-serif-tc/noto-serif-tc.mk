###############################################################################
#
# Noto Serif SC
#
###############################################################################

NOTO_SERIF_TC_VERSION = 1.1
NOTO_SERIF_TC_SOURCE = NotoSerifTC.zip
NOTO_SERIF_TC_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SERIF_TC_LICENSE = LICENSE_OFL.txt
NOTO_SERIF_TC_DEPENDENCIES = host-zip

define NOTO_SERIF_TC_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SERIF_TC_SOURCE) -d $(@D)/
endef

define NOTO_SERIF_TC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-serif-tc/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-serif-tc/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

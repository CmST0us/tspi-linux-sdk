###############################################################################
#
# Noto Serif SC
#
###############################################################################

NOTO_SERIF_SC_VERSION = 1.1
NOTO_SERIF_SC_SOURCE = NotoSerifSC.zip
NOTO_SERIF_SC_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SERIF_SC_LICENSE = LICENSE_OFL.txt
NOTO_SERIF_SC_DEPENDENCIES = host-zip

define NOTO_SERIF_SC_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SERIF_SC_SOURCE) -d $(@D)/
endef

define NOTO_SERIF_SC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-serif-sc/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-serif-sc/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

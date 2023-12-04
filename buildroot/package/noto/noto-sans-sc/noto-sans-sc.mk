###############################################################################
#
# Noto Sans SC
#
###############################################################################

NOTO_SANS_SC_VERSION = 1.1
NOTO_SANS_SC_SOURCE = NotoSansSC.zip
NOTO_SANS_SC_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SANS_SC_LICENSE = LICENSE_OFL.txt
NOTO_SANS_SC_DEPENDENCIES = host-zip

define NOTO_SANS_SC_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SANS_SC_SOURCE) -d $(@D)/
endef

define NOTO_SANS_SC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-sans-sc/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-sans-sc/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

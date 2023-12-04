###############################################################################
#
# Noto Sans SC
#
###############################################################################

NOTO_SANS_TC_VERSION = 1.1
NOTO_SANS_TC_SOURCE = NotoSansTC.zip
NOTO_SANS_TC_SITE = https://noto-website-2.storage.googleapis.com/pkgs
NOTO_SANS_TC_LICENSE = LICENSE_OFL.txt
NOTO_SANS_TC_DEPENDENCIES = host-zip

define NOTO_SANS_TC_EXTRACT_CMDS
	unzip $(DL_DIR)/$(NOTO_SANS_TC_SOURCE) -d $(@D)/
endef

define NOTO_SANS_TC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/fonts/noto-sans-tc/
	cp -r $(@D)/*.otf $(TARGET_DIR)/usr/share/fonts/noto-sans-tc/
	$(NOTO_INSTALL_FONTCONFIG_CONF)
endef

$(eval $(generic-package))

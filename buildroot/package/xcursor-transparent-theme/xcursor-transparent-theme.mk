################################################################################
#
# xcursor-transparent-theme
#
################################################################################

XCURSOR_TRANSPARENT_THEME_VERSION = 0.1.1
XCURSOR_TRANSPARENT_THEME_SITE = http://downloads.yoctoproject.org/releases/matchbox/utils
XCURSOR_TRANSPARENT_THEME_LICENSE = GPL-2.0
XCURSOR_TRANSPARENT_THEME_LICENSE_FILES = COPYING

XCURSOR_TRANSPARENT_THEME_DEPENDENCIES = host-xapp_xcursorgen

define XCURSOR_TRANSPARENT_THEME_ICONS_DEFAULT_CONFIG_INSTALL
	$(INSTALL) -m 0755 -D $(XCURSOR_TRANSPARENT_THEME_PKGDIR)/index.theme \
		$(TARGET_DIR)/usr/share/icons/default/index.theme
endef

XCURSOR_TRANSPARENT_THEME_POST_INSTALL_TARGET_HOOKS += \
	XCURSOR_TRANSPARENT_THEME_ICONS_DEFAULT_CONFIG_INSTALL

$(eval $(autotools-package))

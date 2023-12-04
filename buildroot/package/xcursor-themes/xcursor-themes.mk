################################################################################
#
# xcursor-themes
#
################################################################################

XCURSOR_THEMES_VERSION = 1.0.6
XCURSOR_THEMES_SOURCE = xcursor-themes-$(XCURSOR_THEMES_VERSION).tar.bz2
XCURSOR_THEMES_SITE = http://xorg.freedesktop.org/releases/individual/data
XCURSOR_THEMES_LICENSE = MIT
XCURSOR_THEMES_LICENSE_FILES = COPYING

XCURSOR_THEMES_AUTORECONF = YES

XCURSOR_THEMES_DEPENDENCIES = host-xapp_xcursorgen

ifneq ($(BR2_PACKAGE_XORG7),)
XCURSOR_THEMES_DEPENDENCIES += xlib_libXcursor
else
XCURSOR_THEMES_CONF_OPTS += --with-cursordir=/usr/share/icons/
endif

$(eval $(autotools-package))

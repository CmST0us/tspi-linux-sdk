################################################################################
#
# jwm
#
################################################################################

JWM_VERSION = 71c1549d008c783fbb47e867584d4bf7c5e90b90
JWM_SITE = $(call github,joewing,jwm,$(JWM_VERSION))
JWM_LICENSE = MIT
JWM_LICENSE_FILES = COPYING
JWM_DEPENDENCIES = \
	libfribidi libjpeg libpng librsvg \
	xlib_libX11 xlib_libXext xlib_libXft xlib_libXinerama \
	xlib_libXmu xlib_libXpm xlib_libXrender

ifeq ($(BR2_PACKAGE_CAIRO),y)
JWM_CONF_OPTS += --enable-cairo
JWM_DEPENDENCIES += cairo
else
JWM_CONF_OPTS += --disable-cairo
endif

ifeq ($(BR2_PACKAGE_LIBFRIBIDI),y)
JWM_CONF_OPTS += --enable-fribidi
JWM_DEPENDENCIES += libfribidi
else
JWM_CONF_OPTS += --disable-fribidi
endif

ifeq ($(BR2_PACKAGE_LIBJPEG),y)
JWM_CONF_OPTS += --enable-jpeg
JWM_DEPENDENCIES += libjpeg
else
JWM_CONF_OPTS += --disable-jpeg
endif

ifeq ($(BR2_PACKAGE_LIBPNG),y)
JWM_CONF_OPTS += --enable-png
JWM_DEPENDENCIES += libpng
else
JWM_CONF_OPTS += --disable-png
endif

ifeq ($(BR2_PACKAGE_LIBRSVG),y)
JWM_CONF_OPTS += --enable-rsvg
JWM_DEPENDENCIES += librsvg
else
JWM_CONF_OPTS += --disable-rsvg
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXEXT),y)
JWM_CONF_OPTS += --enable-shape
JWM_DEPENDENCIES += xlib_libXext
else
JWM_CONF_OPTS += --disable-shape
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXFT),y)
JWM_CONF_OPTS += --enable-xft
JWM_DEPENDENCIES += xlib_libXft
else
JWM_CONF_OPTS += --disable-xft
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXINERAMA),y)
JWM_CONF_OPTS += --enable-xinerama
JWM_DEPENDENCIES += xlib_libXinerama
else
JWM_CONF_OPTS += --disable-xinerama
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXMU),y)
JWM_CONF_OPTS += --enable-xmu
JWM_DEPENDENCIES += xlib_libXmu
else
JWM_CONF_OPTS += --disable-xmu
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXPM),y)
JWM_CONF_OPTS += --enable-xpm
JWM_DEPENDENCIES += xlib_libXpm
else
JWM_CONF_OPTS += --disable-xpm
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBXRENDER),y)
JWM_CONF_OPTS += --enable-xrender
JWM_DEPENDENCIES += xlib_libXrender
else
JWM_CONF_OPTS += --disable-xrender
endif

# This package uses autoconf, but not automake, so we need to call
# their special autogen.sh script, and have custom target and staging
# installation commands.

define JWM_RUN_AUTOGEN
	cd $(@D) && PATH=$(BR_PATH) ./autogen.sh
endef
JWM_PRE_CONFIGURE_HOOKS += JWM_RUN_AUTOGEN

$(eval $(autotools-package))

###############################################################################
#
# retroarch
#
###############################################################################

RETROARCH_VERSION = 3e27a504ed3b3675d377e985094f3fdb267237bc
RETROARCH_SITE = https://github.com/libretro/RetroArch.git
RETROARCH_SITE_METHOD = git
RETROARCH_DEPENDENCIES = host-pkgconf

RETROARCH_CONF_OPTS += --disable-oss
RETROARCH_CONF_OPTS += --disable-python
RETROARCH_CONF_OPTS += --disable-pulse
RETROARCH_CONF_OPTS += --disable-cheevos
RETROARCH_CONF_OPTS += --disable-freetype
RETROARCH_CONF_OPTS += --disable-7zip
RETROARCH_CONF_OPTS += --disable-builtinflac
RETROARCH_CONF_OPTS += --disable-ssl
RETROARCH_CONF_OPTS += --disable-libxml2
RETROARCH_CONF_OPTS += --disable-ffmpeg
RETROARCH_CONF_OPTS += --disable-qt

ifeq ($(BR2_PACKAGE_RETROARCH_RGUI),)
	RETROARCH_CONF_OPTS += --disable-rgui
endif

ifeq ($(BR2_PACKAGE_RETROARCH_NETWORKING),)
	RETROARCH_CONF_OPTS += --disable-networking
endif

ifeq ($(BR2_PACKAGE_RETROARCH_HID),)
	RETROARCH_CONF_OPTS += --disable-hid --disable-libusb
endif

ifeq ($(BR2_PACKAGE_RETROARCH_ASSETS),)

define RETRO_ASSETS_INSTALL_TARGET_CMDS
	cp -r  retro-assets  $(TARGET_DIR)/usr/lib/libretro/
endef

endif

ifeq ($(BR2_PACKAGE_ZLIB),y)
	RETROARCH_CONF_OPTS += --enable-zlib
	RETROARCH_DEPENDENCIES += zlib
else
	RETROARCH_CONF_OPTS += --disable-zlib
endif

ifeq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	RETROARCH_CONF_OPTS += --enable-x11
	RETROARCH_DEPENDENCIES += xlib_libX11
else
	RETROARCH_CONF_OPTS += --disable-x11
	RETROARCH_CONF_ENV += HAVE_XCB=no
endif

ifeq ($(BR2_PACKAGE_SDL),y)
	RETROARCH_CONF_OPTS += --enable-sdl
	RETROARCH_DEPENDENCIES += sdl
else
	RETROARCH_CONF_OPTS += --disable-sdl
endif

ifeq ($(BR2_PACKAGE_WAYLAND),y)
	RETROARCH_CONF_OPTS += --enable-wayland
	RETROARCH_DEPENDENCIES += wayland
else
	RETROARCH_CONF_OPTS += --disable-wayland
endif

# Prefer using wayland than sdl2
ifeq ($(BR2_PACKAGE_SDL2):$(BR2_PACKAGE_WAYLAND),y:)
	RETROARCH_CONF_OPTS += --enable-sdl2
	RETROARCH_DEPENDENCIES += sdl2
else
	RETROARCH_CONF_OPTS += --disable-sdl2
endif

ifeq ($(BR2_PACKAGE_HAS_LIBGLES),y)
	RETROARCH_CONF_OPTS += --enable-opengles
	RETROARCH_DEPENDENCIES += libgles
else
	RETROARCH_CONF_OPTS += --disable-opengles
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	RETROARCH_CONF_OPTS += --enable-egl
	RETROARCH_DEPENDENCIES += libegl
else
	RETROARCH_CONF_OPTS += --disable-egl
endif

ifeq ($(BR2_PACKAGE_HAS_LIBOPENVG),y)
	RETROARCH_DEPENDENCIES += libopenvg
endif

ifeq ($(BR2_ARM_CPU_HAS_NEON),y)
	RETROARCH_CONF_OPTS += --enable-neon
endif

ifeq ($(BR2_GCC_TARGET_FLOAT_ABI),"hard")
	RETROARCH_CONF_OPTS += --enable-floathard
endif

define RETROARCH_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		PKG_CONFIG_SYSROOT_DIR="$(STAGING_DIR)"	\
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig" \
		$(RETROARCH_CONF_ENV) \
		./configure \
		--prefix=/usr \
		$(RETROARCH_CONF_OPTS) \
	)
endef

$(eval $(autotools-package))

# DEFINITION OF LIBRETRO PLATFORM
LIBRETRO_PLATFORM += buildroot

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_PLATFORM += gles
endif

ifeq ($(BR2_ARM_CPU_ARMV4),y)
	LIBRETRO_PLATFORM += armv4
else ifeq ($(BR2_ARM_CPU_ARMV5),y)
	LIBRETRO_PLATFORM += armv5
else ifeq ($(BR2_ARM_CPU_ARMV6),y)
	LIBRETRO_PLATFORM += armv6
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_PLATFORM += armv7
else ifeq ($(BR2_arm),y)
	LIBRETRO_PLATFORM += armv7
else ifeq ($(BR2_aarch64),y)
	LIBRETRO_PLATFORM += armv8
endif

ifeq ($(BR2_GCC_TARGET_FLOAT_ABI),"hard")
	LIBRETRO_PLATFORM += hardfloat
endif

ifeq ($(BR2_ARM_CPU_HAS_NEON),y)
	LIBRETRO_PLATFORM += neon
endif

################################################################################
#
# MAME
#
################################################################################
LIBRETRO_MAME_VERSION = 47c241e4f528f663a36ae82a3bbaa789d929e282
LIBRETRO_MAME_SITE = $(call github,libretro,mame,$(LIBRETRO_MAME_VERSION))

LIBRETRO_MAME_CONF += TARGETOS=linux
LIBRETRO_MAME_CONF += TOOLCHAIN=$(TARGET_CROSS)
LIBRETRO_MAME_CONF += NOASM=1
LIBRETRO_MAME_CONF += UNAME_M=$(BR2_ARCH)
LIBRETRO_MAME_CONF += UNAME="Linux "$(BR2_ARCH)

ifeq ($(BR2_arm),y)
	LIBRETRO_MAME_CONF += PTR64=0
endif

ifeq ($(BR2_aarch64),y)
	LIBRETRO_MAME_CONF += PTR64=1
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_MAME_CFLAGS += -DBGFX_CONFIG_RENDERER_OPENGLES=1
ifneq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	LIBRETRO_MAME_CFLAGS += -DMESA_EGL_NO_X11_HEADERS
endif
endif

define LIBRETRO_MAME_BUILD_CMDS
	BUILDDIR="" CFLAGS="$(TARGET_CFLAGS) $(LIBRETRO_MAME_CFLAGS)" \
	       CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) -f Makefile.libretro \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)" $(LIBRETRO_MAME_CONF)
endef

define LIBRETRO_MAME_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mame_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mame_libretro.so
endef

$(eval $(generic-package))

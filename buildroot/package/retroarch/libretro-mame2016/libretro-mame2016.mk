################################################################################
#
# MAME2016
#
################################################################################
LIBRETRO_MAME2016_VERSION = e06d731644217f46bf5a7613222632d41a327f93
LIBRETRO_MAME2016_SITE = $(call github,libretro,mame2016-libretro,$(LIBRETRO_MAME2016_VERSION))

LIBRETRO_MAME2016_CONF += TARGETOS=linux
LIBRETRO_MAME2016_CONF += TOOLCHAIN=$(TARGET_CROSS)
LIBRETRO_MAME2016_CONF += NOASM=1
LIBRETRO_MAME2016_CONF += UNAME_M=$(BR2_ARCH)
LIBRETRO_MAME2016_CONF += UNAME="Linux "$(BR2_ARCH)

ifeq ($(BR2_arm),y)
	LIBRETRO_MAME2016_CONF += PTR64=0
endif

ifeq ($(BR2_aarch64),y)
	LIBRETRO_MAME2016_CONF += PTR64=1
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_MAME2016_CFLAGS += -DBGFX_CONFIG_RENDERER_OPENGLES=1
ifneq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	LIBRETRO_MAME2016_CFLAGS += -DMESA_EGL_NO_X11_HEADERS
endif
endif

define LIBRETRO_MAME2016_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) $(LIBRETRO_MAME2016_CFLAGS)" \
	       CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) -f Makefile.libretro \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)" $(LIBRETRO_MAME2016_CONF)
endef

define LIBRETRO_MAME2016_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mame2016_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mame2016_libretro.so
endef

$(eval $(generic-package))

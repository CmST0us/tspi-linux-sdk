################################################################################
#
# SNES9X2010
#
################################################################################
LIBRETRO_SNES9X2010_VERSION = 1b5ec5b93ad7e85bb63e29d81345cda9a1a42845
LIBRETRO_SNES9X2010_SITE = $(call github,libretro,snes9x2010,$(LIBRETRO_SNES9X2010_VERSION))

LIBRETRO_SNES9X2010_PLATFORM=$(LIBRETRO_PLATFORM)

define LIBRETRO_SNES9X2010_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) -O3 -ffast-math" \
	       CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) -f Makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_SNES9X2010_PLATFORM)"
endef

define LIBRETRO_SNES9X2010_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/snes9x2010_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/snes9x2010_libretro.so
endef

$(eval $(generic-package))

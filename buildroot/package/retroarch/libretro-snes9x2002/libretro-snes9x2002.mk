################################################################################
#
# SNES9X2002
#
################################################################################
LIBRETRO_SNES9X2002_VERSION = 39d8b168453556fe80905c51c8f38fdbade8fd13
LIBRETRO_SNES9X2002_SITE = $(call github,libretro,snes9x2002,$(LIBRETRO_SNES9X2002_VERSION))

define LIBRETRO_SNES9X2002_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="unix"
endef

define LIBRETRO_SNES9X2002_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/snes9x2002_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/snes9x2002_libretro.so
endef

$(eval $(generic-package))

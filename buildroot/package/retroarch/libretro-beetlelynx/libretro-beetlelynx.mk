################################################################################
#
# BEETLELYNX
#
################################################################################
LIBRETRO_BEETLELYNX_VERSION = 6816829ae785e2d468256d346fcd90b5baaa7327
LIBRETRO_BEETLELYNX_SITE = $(call github,libretro,beetle-lynx-libretro,$(LIBRETRO_BEETLELYNX_VERSION))

define LIBRETRO_BEETLELYNX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLELYNX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_lynx_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlelynx_libretro.so
endef

$(eval $(generic-package))

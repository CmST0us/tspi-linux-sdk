################################################################################
#
# BEETLESUPERGRAFX
#
################################################################################
LIBRETRO_BEETLESUPERGRAFX_VERSION = 0d4d96428073f8734e80a2ebc157daa228babe51
LIBRETRO_BEETLESUPERGRAFX_SITE = $(call github,libretro,beetle-supergrafx-libretro,$(LIBRETRO_BEETLESUPERGRAFX_VERSION))

define LIBRETRO_BEETLESUPERGRAFX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_BEETLESUPERGRAFX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_supergrafx_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlesupergrafx_libretro.so
endef

$(eval $(generic-package))

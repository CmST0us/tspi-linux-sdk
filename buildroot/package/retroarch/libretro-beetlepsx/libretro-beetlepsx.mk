################################################################################
#
# BEETLEPSX
#
################################################################################
LIBRETRO_BEETLEPSX_VERSION = 9b9a3325dbefae05ce5614a26d50a5b6ccb69d85
LIBRETRO_BEETLEPSX_SITE = $(call github,libretro,beetle-psx-libretro,$(LIBRETRO_BEETLEPSX_VERSION))

define LIBRETRO_BEETLEPSX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++ -lm" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)" $(LIBRETRO_BEETLEPSX_CONF)
endef

define LIBRETRO_BEETLEPSX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mednafen_psx_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/beetlepsx_libretro.so
endef

$(eval $(generic-package))

################################################################################
#
# MAME2003PLUS
#
################################################################################
LIBRETRO_MAME2003PLUS_VERSION = 7c1aba12a23fcb8dcd3ca23b6593a84399133e59
LIBRETRO_MAME2003PLUS_SITE = $(call github,libretro,mame2003-plus-libretro,$(LIBRETRO_MAME2003PLUS_VERSION))

define LIBRETRO_MAME2003PLUS_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) $(LIBRETRO_MAME2003PLUS_CFLAGS)" \
	       CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PLATFORM)"
endef

define LIBRETRO_MAME2003PLUS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mame2003_plus_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mame2003plus_libretro.so
endef

$(eval $(generic-package))

################################################################################
#
# ARMSNES
#
################################################################################
LIBRETRO_ARMSNES_VERSION = dbea0479cb6fba8583c0ca642ad70dd1bf0b58d1
LIBRETRO_ARMSNES_SITE = $(call github,RetroPie,ARMSNES-libretro,$(LIBRETRO_ARMSNES_VERSION))

define LIBRETRO_ARMSNES_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) -O3 -ffast-math -D__FAST_OBJS__" \
			 CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)"
endef

define LIBRETRO_ARMSNES_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/libpocketsnes.so \
		$(TARGET_DIR)/usr/lib/libretro/armsnes_libretro.so
endef

$(eval $(generic-package))

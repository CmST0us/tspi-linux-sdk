################################################################################
#
# BNES
#
################################################################################
LIBRETRO_BNES_VERSION = 598c123771c885f8c48967f82b3a496b9e105b79
LIBRETRO_BNES_SITE = $(call github,libretro,bnes-libretro,$(LIBRETRO_BNES_VERSION))

define LIBRETRO_BNES_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="unix"
endef

define LIBRETRO_BNES_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/bnes_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/bnes_libretro.so
endef

$(eval $(generic-package))

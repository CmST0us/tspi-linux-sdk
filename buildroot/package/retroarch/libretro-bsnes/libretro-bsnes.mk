################################################################################
#
# BSNES
#
################################################################################
LIBRETRO_BSNES_VERSION = c6e90ddcdfdc5ca1b56319df662912b3c026d78f
LIBRETRO_BSNES_SITE = $(call github,libretro,bsnes-libretro,$(LIBRETRO_BSNES_VERSION))

LIBRETRO_BSNES_CONF += profile=$(BR2_PACKAGE_LIBRETRO_BSNES_PROFILE)

define LIBRETRO_BSNES_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="unix" $(LIBRETRO_BSNES_CONF)
endef

define LIBRETRO_BSNES_INSTALL_TARGET_CMDS
	$(INSTALL) -D \
		$(@D)/out/bsnes_$(BR2_PACKAGE_LIBRETRO_BSNES_PROFILE)_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/bsnes_libretro.so
endef

$(eval $(generic-package))

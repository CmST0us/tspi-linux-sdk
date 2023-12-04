################################################################################
#
# GENESISPLUSGX
#
################################################################################
LIBRETRO_GENESISPLUSGX_VERSION = 7856b7208981fa4dc33623f89c4f330a95019c26
LIBRETRO_GENESISPLUSGX_SITE = $(call github,libretro,Genesis-Plus-GX,$(LIBRETRO_GENESISPLUSGX_VERSION))

LIBRETRO_GENESISPLUSGX_PLATFORM = $(LIBRETRO_PLATFORM)

# Reusing RPI configs
ifeq ($(BR2_arm),y)
	LIBRETRO_GENESISPLUSGX_PLATFORM += rpi
endif
ifeq ($(BR2_ARM_CPU_ARMV6),y)
	LIBRETRO_GENESISPLUSGX_PLATFORM += rpi1
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_GENESISPLUSGX_PLATFORM += rpi2
else ifeq ($(BR2_ARM_CPU_ARMV8A),y)
	LIBRETRO_GENESISPLUSGX_PLATFORM += rpi3
endif

define LIBRETRO_GENESISPLUSGX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/ -f Makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_GENESISPLUSGX_PLATFORM)"
endef

define LIBRETRO_GENESISPLUSGX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/genesis_plus_gx_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/genesisplusgx_libretro.so
endef

$(eval $(generic-package))

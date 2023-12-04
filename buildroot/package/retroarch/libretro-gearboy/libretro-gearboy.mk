################################################################################
#
# GEARBOY
#
################################################################################
LIBRETRO_GEARBOY_VERSION = 4867b81c27d9b1144f077a20c6e2003ba21bd9a2
LIBRETRO_GEARBOY_SITE = $(call github,libretro,Gearboy,$(LIBRETRO_GEARBOY_VERSION))

LIBRETRO_GEARBOY_PLATFORM=$(LIBRETRO_PLATFORM)
LIBRETRO_GEARBOY_PLATFORM += unix

# Reusing RPI configs
ifeq ($(BR2_arm),y)
	LIBRETRO_GEARBOY_PLATFORM += rpi
endif
ifeq ($(BR2_ARM_CPU_ARMV6),y)
	LIBRETRO_GEARBOY_PLATFORM += rpi1
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_GEARBOY_PLATFORM += rpi2
else ifeq ($(BR2_ARM_CPU_ARMV8A),y)
	LIBRETRO_GEARBOY_PLATFORM += rpi3
endif

define LIBRETRO_GEARBOY_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/platforms/libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_GEARBOY_PLATFORM)"
endef

define LIBRETRO_GEARBOY_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/platforms/libretro/gearboy_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/gearboy_libretro.so
endef

$(eval $(generic-package))

################################################################################
#
# PARALLELN64
#
################################################################################
LIBRETRO_PARALLELN64_VERSION = ab155da18068f638e5ace2e5e6f7387bddc3511b
LIBRETRO_PARALLELN64_SITE = $(call github,libretro,parallel-n64,$(LIBRETRO_PARALLELN64_VERSION))

LIBRETRO_PARALLELN64_PLATFORM=$(LIBRETRO_PLATFORM)
LIBRETRO_PARALLELN64_PLATFORM += unix
LIBRETRO_PARALLELN64_PLATFORM += mesa

# Reusing RPI configs
ifeq ($(BR2_arm),y)
	LIBRETRO_PARALLELN64_PLATFORM += rpi
endif
ifeq ($(BR2_ARM_CPU_ARMV6),y)
	LIBRETRO_PARALLELN64_PLATFORM += rpi1
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_PARALLELN64_PLATFORM += rpi2
else ifeq ($(BR2_ARM_CPU_ARMV8A),y)
	LIBRETRO_PARALLELN64_PLATFORM += rpi3
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_PARALLELN64_CONF += FORCE_GLES=1
endif

define LIBRETRO_PARALLELN64_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS) -lstdc++" \
	       $(MAKE) -C $(@D) \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PARALLELN64_PLATFORM)" \
	       $(LIBRETRO_PARALLELN64_CONF)
endef

define LIBRETRO_PARALLELN64_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/parallel_n64_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/paralleln64_libretro.so
endef

$(eval $(generic-package))

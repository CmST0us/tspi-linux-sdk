################################################################################
#
# MUPEN64PLUS
#
################################################################################
LIBRETRO_MUPEN64PLUS_VERSION = 4ca2fa8633666e26e2f163dcd3c226b598cb2aa4
LIBRETRO_MUPEN64PLUS_SITE = $(call github,libretro,mupen64plus-libretro,$(LIBRETRO_MUPEN64PLUS_VERSION))

LIBRETRO_MUPEN64PLUS_PLATFORM=$(LIBRETRO_PLATFORM)

# Reusing RPI configs
ifeq ($(BR2_arm),y)
	LIBRETRO_MUPEN64PLUS_PLATFORM += rpi
endif
ifeq ($(BR2_ARM_CPU_ARMV6),y)
	LIBRETRO_MUPEN64PLUS_PLATFORM += rpi1
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_MUPEN64PLUS_PLATFORM += rpi2
else ifeq ($(BR2_ARM_CPU_ARMV8A),y)
	LIBRETRO_MUPEN64PLUS_PLATFORM += rpi3
endif

ifneq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	LIBRETRO_MUPEN64PLUS_CFLAGS += -DMESA_EGL_NO_X11_HEADERS
	LIBRETRO_MUPEN64PLUS_CXXFLAGS += -DMESA_EGL_NO_X11_HEADERS
endif

define LIBRETRO_MUPEN64PLUS_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) $(LIBRETRO_MUPEN64PLUS_CFLAGS)" \
	       CXXFLAGS="$(TARGET_CXXFLAGS) $(LIBRETRO_MUPEN64PLUS_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/ \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_MUPEN64PLUS_PLATFORM)"
endef

define LIBRETRO_MUPEN64PLUS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/mupen64plus_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/mupen64plus_libretro.so
endef

$(eval $(generic-package))

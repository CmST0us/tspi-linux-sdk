################################################################################
#
# PICODRIVE
#
################################################################################
LIBRETRO_PICODRIVE_VERSION = 2db90628f5f29f1d4030172ce4f57ab757fa9d93
LIBRETRO_PICODRIVE_SITE = https://github.com/libretro/picodrive.git
LIBRETRO_PICODRIVE_SITE_METHOD = git
LIBRETRO_PICODRIVE_GIT_SUBMODULES = yes

LIBRETRO_PICODRIVE_PLATFORM = $(LIBRETRO_PLATFORM)

ifeq ($(BR2_arm),y)
	LIBRETRO_PICODRIVE_PLATFORM += raspberrypi
endif

ifneq ($(BR2_ARM_CPU_ARMV4)$(BR2_ARM_CPU_ARMV5),)
	LIBRETRO_PICODRIVE_CONF += HAVE_ARMv6=0
endif

define LIBRETRO_PICODRIVE_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS)" CXXFLAGS="$(TARGET_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       $(MAKE) -C $(@D)/ -f Makefile.libretro \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(LIBRETRO_PICODRIVE_PLATFORM)" \
	       $(LIBRETRO_PICODRIVE_CONF)
endef

define LIBRETRO_PICODRIVE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/picodrive_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/picodrive_libretro.so
endef

$(eval $(generic-package))

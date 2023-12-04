################################################################################
#
# PCSX
#
################################################################################
PCSX_VERSION = bebe989ba22f0bac230a0b8a75617ae895ea274f
PCSX_SITE = https://github.com/notaz/pcsx_rearmed.git
PCSX_SITE_METHOD = git
PCSX_GIT_SUBMODULES = yes

PCSX_CONF_OPTS += TARGET=pcsx
PCSX_CONF_OPTS += PLATFORM=generic
PCSX_CONF_OPTS += SOUND_DRIVERS="alsa sdl"

PCSX_DEPENDENCIES += sdl libpng
PCSX_LDFLAGS +=-lSDL
PCSX_LDFLAGS += -lpng

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	PCSX_CONF_OPTS += HAVE_GLES=1
	PCSX_LDFLAGS += -lEGL -lGLESv2 -lGLESv1_CM
endif
PCSX_CONF_OPTS += MAIN_LDFLAGS="$(PCSX_LDFLAGS)"

# DEFINITION OF LIBRETRO PLATFORM
ifeq ($(BR2_ARM_CPU_ARMV4),y)
	PCSX_PLATFORM += armv4
else ifeq ($(BR2_ARM_CPU_ARMV5),y)
	PCSX_PLATFORM += armv5
else ifeq ($(BR2_ARM_CPU_ARMV6),y)
	PCSX_PLATFORM += armv6
else ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	PCSX_PLATFORM += armv7
else ifeq ($(BR2_arm),y)
	PCSX_PLATFORM += armv7
else ifeq ($(BR2_aarch64),y)
	PCSX_PLATFORM += armv8
endif

ifeq ($(BR2_GCC_TARGET_FLOAT_ABI),"hard")
	PCSX_PLATFORM += hardfloat
endif

ifeq ($(BR2_ARM_CPU_HAS_NEON),y)
	PCSX_PLATFORM += neon
endif

PCSX_CFLAGS += -I$(STAGING_DIR)/usr/include/SDL

ifneq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	PCSX_CFLAGS += -DMESA_EGL_NO_X11_HEADERS
	PCSX_CXXFLAGS += -DMESA_EGL_NO_X11_HEADERS
endif

define PCSX_BUILD_CMDS
	CFLAGS="$(TARGET_CFLAGS) $(PCSX_CFLAGS)" \
	       CXXFLAGS="$(TARGET_CXXFLAGS) $(PCSX_CXXFLAGS)" \
	       LDFLAGS="$(TARGET_LDFLAGS)" \
	       $(MAKE) -C $(@D)/ -f Makefile.libretro \
	       CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" LD="$(TARGET_CC)" \
	       RANLIB="$(TARGET_RANLIB)" AR="$(TARGET_AR)" \
	       platform="$(PCSX_PLATFORM)" $(PCSX_CONF_OPTS)
endef

define PCSX_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/pcsx $(TARGET_DIR)/usr/bin/pcsx
endef

$(eval $(generic-package))

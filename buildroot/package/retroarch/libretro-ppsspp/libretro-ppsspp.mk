################################################################################
#
# PPSSPP
#
################################################################################
LIBRETRO_PPSSPP_VERSION = 6fe7a97462333a4b86da8d4a526e09711f66ff06
LIBRETRO_PPSSPP_SITE = https://github.com/libretro/ppsspp.git
LIBRETRO_PPSSPP_SITE_METHOD = git
LIBRETRO_PPSSPP_GIT_SUBMODULES = yes

# Configs for libretro
LIBRETRO_PPSSPP_CONF_OPTS += -DLIBRETRO=ON
LIBRETRO_PPSSPP_CONF_OPTS += -DUSE_SDL2=OFF
LIBRETRO_PPSSPP_CONF_OPTS += -DUSING_QT_UI=OFF
LIBRETRO_PPSSPP_CONF_OPTS += -DUSING_X11_VULKAN=OFF
LIBRETRO_PPSSPP_CONF_OPTS += -DUSING_X11=OFF

LIBRETRO_PPSSPP_CONF_OPTS += -DLINUX=ON
LIBRETRO_PPSSPP_CONF_OPTS += -DUSE_FFMPEG=OFF

ifeq ($(BR2_arm),y)
	LIBRETRO_PPSSPP_CONF_OPTS += -DARM=ON
else
ifeq ($(BR2_aarch64),y)
	LIBRETRO_PPSSPP_CONF_OPTS += -DARM64=ON
endif
endif

ifeq ($(BR2_ARM_CPU_ARMV7A),y)
	LIBRETRO_PPSSPP_CONF_OPTS += -DARMV7=ON
endif

ifeq ($(BR2_PACKAGE_HAS_LIBEGL),y)
	LIBRETRO_PPSSPP_CONF_OPTS += -DUSING_EGL=ON
	LIBRETRO_PPSSPP_CONF_OPTS += -DUSING_GLES2=ON
endif

ifneq ($(BR2_PACKAGE_XLIB_LIBX11),y)
	LIBRETRO_PPSSPP_CONF_OPTS += -DCMAKE_C_FLAGS=-DMESA_EGL_NO_X11_HEADERS
	LIBRETRO_PPSSPP_CONF_OPTS += -DCMAKE_CXX_FLAGS=-DMESA_EGL_NO_X11_HEADERS
endif

define LIBRETRO_PPSSPP_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/lib/ppsspp_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/ppsspp_libretro.so
	$(INSTALL) -D $(@D)/lib/libSPIRV.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D $(@D)/lib/libglslang.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D $(@D)/lib/libHLSL.so $(TARGET_DIR)/usr/lib/
	$(INSTALL) -D $(@D)/lib/libSPVRemapper.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(cmake-package))

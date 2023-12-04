################################################################################
#
# gl4es
#
################################################################################
GL4ES_VERSION = db0218b1616a6e18be8cf17f33118d1ffc405993
GL4ES_SITE = $(call github,ptitSeb,gl4es,$(GL4ES_VERSION))
GL4ES_INSTALL_STAGING = YES

GL4ES_DEPENDENCIES = libegl libgles

GL4ES_CONF_OPTS += -DDEFAULT_ES=2 -DUSE_CLOCK=ON \
	-DCMAKE_C_FLAGS="-ftree-vectorize -fsingle-precision-constant -ffast-math" \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo

ifeq ($(BR2_PACKAGE_XLIB_LIBX11),y)
ifeq ($(BR2_PACKAGE_MESA3D),y)
GL4ES_DEPENDENCIES += mesa3d
endif
else
GL4ES_CONF_OPTS += -DNOX11=ON
endif

$(eval $(cmake-package))

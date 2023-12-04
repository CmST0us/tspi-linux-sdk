###############################################################################
#
# MYNT EYE D SDK
#
###############################################################################

MYNT_EYE_D_SDK_LICENSE = LICENSE
MYNT_EYE_D_SDK_INSTALL_STAGING = YES

MYNT_EYE_D_SDK_DEPENDENCIES += libusb-compat jpeg

ifeq ($(BR2_PACKAGE_OPENCV),y)
# The samples are using opencv instead of opencv3
MYNT_EYE_D_SDK_DEPENDENCIES += opencv
else ifeq ($(BR2_PACKAGE_OPENCV3),y)
MYNT_EYE_D_SDK_DEPENDENCIES += opencv3
endif

# For selecting prebuilt eSPDI library
ifeq ($(BR2_arm),y)
MYNT_EYE_D_SDK_CONF_OPTS += -D__arch=armv7l
else ifeq ($(BR2_aarch64),y)
MYNT_EYE_D_SDK_CONF_OPTS += -D__arch=aarch64
endif

# Support cross-compiling
define MYNT_EYE_D_SDK_CMAKE_FIXUP
	sed -i 's#\(/usr/lib\)#$${CMAKE_SYSROOT}/\1#' \
		$(STAGING_DIR)/usr/lib/cmake/mynteyed/mynteyed-targets-release.cmake
	sed -i 's#\(INTERFACE_INCLUDE_DIRECTORIES \"\)#\1$${CMAKE_SYSROOT}#' \
		$(STAGING_DIR)/usr/lib/cmake/mynteyed/mynteyed-targets.cmake
endef
MYNT_EYE_D_SDK_POST_INSTALL_STAGING_HOOKS += MYNT_EYE_D_SDK_CMAKE_FIXUP

$(eval $(cmake-package))

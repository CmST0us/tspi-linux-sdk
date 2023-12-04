################################################################################
#
# vulkan-tools
#
################################################################################

VULKAN_TOOLS_VERSION = 1.3.214
VULKAN_TOOLS_SITE = $(call github,KhronosGroup,Vulkan-Tools,v$(VULKAN_TOOLS_VERSION))
VULKAN_TOOLS_LICENSE = Apache-2.0
VULKAN_TOOLS_LICENSE_FILES = LICENSE.txt

VULKAN_TOOLS_DEPENDENCIES = vulkan-loader

VULKAN_TOOLS_CONF_OPTS = -DBUILD_CUBE=OFF

ifeq ($(BR2_PACKAGE_XORG7),)
VULKAN_TOOLS_CONF_OPTS += \
	-DBUILD_WSI_XCB_SUPPORT=OFF -DBUILD_WSI_XLIB_SUPPORT=OFF
else
VULKAN_TOOLS_CONF_OPTS += \
	-DBUILD_WSI_XCB_SUPPORT=ON -DBUILD_WSI_XLIB_SUPPORT=ON
VULKAN_TOOLS_DEPENDENCIES += libxcb xlib_libX11 xlib_libXrandr
endif

ifeq ($(BR2_PACKAGE_WAYLAND),)
VULKAN_TOOLS_CONF_OPTS += -DBUILD_WSI_WAYLAND_SUPPORT=OFF
else
VULKAN_TOOLS_CONF_OPTS += -DBUILD_WSI_WAYLAND_SUPPORT=ON
VULKAN_TOOLS_DEPENDENCIES += wayland
endif

$(eval $(cmake-package))

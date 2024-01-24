################################################################################
#
# spirv-headers
#
################################################################################

SPIRV_HEADERS_VERSION = 1.3.216.0
SPIRV_HEADERS_SITE = $(call github,KhronosGroup,SPIRV-Headers,sdk-$(SPIRV_HEADERS_VERSION))
SPIRV_HEADERS_LICENSE = Apache-2.0
SPIRV_HEADERS_LICENSE_FILES = LICENSE.txt
SPIRV_HEADERS_INSTALL_STAGING = YES

$(eval $(cmake-package))

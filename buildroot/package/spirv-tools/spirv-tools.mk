################################################################################
#
# spirv-tools
#
################################################################################

SPIRV_TOOLS_VERSION = 1.3.216.0
SPIRV_TOOLS_SITE = $(call github,KhronosGroup,SPIRV-Tools,sdk-$(SPIRV_TOOLS_VERSION))
SPIRV_TOOLS_LICENSE = Apache-2.0
SPIRV_TOOLS_LICENSE_FILES = LICENSE.txt

SPIRV_TOOLS_DEPENDENCIES = spirv-headers

SPIRV_TOOLS_CONF_OPTS = -DSPIRV-Headers_SOURCE_DIR=${STAGING_DIR}/usr

$(eval $(cmake-package))

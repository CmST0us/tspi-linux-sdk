################################################################################
#
# tinyxml2
#
################################################################################

TINYXML2_VERSION = 6.2.0
TINYXML2_SITE = $(call github,leethomason,tinyxml2,$(TINYXML2_VERSION))
TINYXML2_LICENSE = Zlib
TINYXML2_LICENSE_FILES = readme.md
TINYXML2_INSTALL_STAGING = YES

ifeq ($(BR2_STATIC_LIBS),y)
TINYXML2_CONF_OPTS += -DBUILD_STATIC_LIBS=ON
endif

define TINYXML2_FIX_CONFIG_CMAKE_FILE
	$(SED) 's%set(_IMPORT_PREFIX \"\/usr\")%set(_IMPORT_PREFIX \"$(STAGING_DIR)\/usr\")%' \
		$(STAGING_DIR)/usr/lib/cmake/tinyxml2/tinyxml2Targets.cmake
	$(SED) 's%INTERFACE_INCLUDE_DIRECTORIES \"\/usr\/include\"%INTERFACE_INCLUDE_DIRECTORIES \"$(STAGING_DIR)\/usr\/include\"%' \
		$(STAGING_DIR)/usr/lib/cmake/tinyxml2/tinyxml2Targets.cmake
endef
TINYXML2_POST_INSTALL_STAGING_HOOKS += TINYXML2_FIX_CONFIG_CMAKE_FILE

$(eval $(cmake-package))

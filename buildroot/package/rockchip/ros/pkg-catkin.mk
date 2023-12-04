################################################################################
# ROS catkin package infrastructure
################################################################################

ROSPREFIX = /opt/ros/kinetic/
ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSPREFIX = /opt/ros/indigo/
endif

define inner-catkin-package

$(2)_DEPENDENCIES += host-catkin

$(2)_INSTALL_STAGING ?= YES
$(2)_CONF_OPTS += \
	-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/$(ROSPREFIX);$(STAGING_DIR)/usr;$(HOST_DIR)/$(ROSPREFIX);$(HOST_DIR)/usr" \
	-DCMAKE_INSTALL_PREFIX="$(ROSPREFIX)" \

$(2)_MAKE_ENV += \
	_python_sysroot=$(STAGING_DIR) \
	_python_prefix=/usr \
	_python_exec_prefix=/usr

# Call the cmake package infrastructure to generate the necessary make targets
$(call inner-cmake-package,$(1),$(2),$(3),$(4))

endef

################################################################################
# catkin-package -- the target generator macro for catkin packages
################################################################################

catkin-package = $(call inner-catkin-package,$(pkgname),$(call UPPERCASE,$(pkgname)),$(call UPPERCASE,$(pkgname)),target)

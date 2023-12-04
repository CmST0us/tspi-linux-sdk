#default to KINETIC
ROSLIB_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSLIB_VERSION = 1.11.14
endif

ROSLIB_SITE = $(call github,ros,ros,$(ROSLIB_VERSION))
ROSLIB_SUBDIR = core/roslib

ROSLIB_DEPENDENCIES = boost rospack

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
define ROSLIB_INDIGO_FIX_ROS_SH
	$(SED) 's/@(PYTHON_EXECUTABLE)/\/usr\/bin\/python/' $(@D)/core/roslib/env-hooks/10.ros.sh.em
	$(SED) 's%CATKIN_DEVEL_PREFIX%CMAKE_INSTALL_PREFIX%' $(@D)/core/roslib/env-hooks/10.ros.sh.em
endef
ROSLIB_POST_PATCH_HOOKS += ROSLIB_INDIGO_FIX_ROS_SH
endif

$(eval $(catkin-package))

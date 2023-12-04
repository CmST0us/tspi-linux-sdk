#default to KINETIC
ROSBUILD_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSBUILD_VERSION = 1.11.14
endif

ROSBUILD_SITE = $(call github,ros,ros,$(ROSBUILD_VERSION))
ROSBUILD_SUBDIR = core/rosbuild

ifeq ($(BR2_PACKAGE_ROS_KINETIC),y)
define ROSBUILD_FIX_ROS_ROOT_PATH
	$(SED) 's%/@(CATKIN_GLOBAL_SHARE_DESTINATION)/ros%%' $(@D)/core/rosbuild/env-hooks/10.rosbuild.sh.em
endef
ROSBUILD_POST_PATCH_HOOKS += ROSBUILD_FIX_ROS_ROOT_PATH
endif

$(eval $(catkin-package))

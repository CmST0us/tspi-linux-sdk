#default to KINETIC
ROSPARAM_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSPARAM_VERSION = 1.11.21
endif

ROSPARAM_SITE = $(call github,ros,ros_comm,$(ROSPARAM_VERSION))
ROSPARAM_SUBDIR = tools/rosparam

$(eval $(catkin-package))

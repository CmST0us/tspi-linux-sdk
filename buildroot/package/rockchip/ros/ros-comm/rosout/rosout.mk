#default to KINETIC
ROSOUT_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSOUT_VERSION = 1.11.21
endif

ROSOUT_SITE = $(call github,ros,ros_comm,$(ROSOUT_VERSION))
ROSOUT_SUBDIR = tools/rosout

ROSOUT_DEPENDENCIES = roscpp rosgraph_msgs

$(eval $(catkin-package))

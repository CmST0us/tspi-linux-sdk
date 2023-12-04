#default to KINETIC
ROSNODE_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSNODE_VERSION = 1.11.21
endif

ROSNODE_SITE = $(call github,ros,ros_comm,$(ROSNODE_VERSION))
ROSNODE_SUBDIR = tools/rosnode

ROSNODE_DEPENDENCIES = rostest

$(eval $(catkin-package))

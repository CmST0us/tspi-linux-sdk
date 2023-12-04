#default to KINETIC
ROSWTF_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSWTF_VERSION = 1.11.21
endif

ROSWTF_SITE = $(call github,ros,ros_comm,$(ROSWTF_VERSION))
ROSWTF_SUBDIR = utilities/roswtf

ROSWTF_DEPENDENCIES = rostest

$(eval $(catkin-package))

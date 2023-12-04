#default to KINETIC
ROSMASTER_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSMASTER_VERSION = 1.11.21
endif

ROSMASTER_SITE = $(call github,ros,ros_comm,$(ROSMASTER_VERSION))
ROSMASTER_SUBDIR = tools/rosmaster

$(eval $(catkin-package))

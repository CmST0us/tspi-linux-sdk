#default to KINETIC
ROSMSG_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSMSG_VERSION = 1.11.21
endif

ROSMSG_SITE = $(call github,ros,ros_comm,$(ROSMSG_VERSION))
ROSMSG_SUBDIR = tools/rosmsg

$(eval $(catkin-package))

#default to KINETIC
ROS_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROS_VERSION = 1.11.14
endif

ROS_SITE = $(call github,ros,ros,$(ROS_VERSION))
ROS_SUBDIR = ros

$(eval $(catkin-package))

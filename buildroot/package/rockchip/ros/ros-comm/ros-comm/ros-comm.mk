#default to KINETIC
ROS_COMM_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROS_COMM_VERSION = 1.11.21
endif

ROS_COMM_SITE = $(call github,ros,ros_comm,$(ROS_COMM_VERSION))
ROS_COMM_SUBDIR = ros_comm

$(eval $(catkin-package))

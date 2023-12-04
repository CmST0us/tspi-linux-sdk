#default to KINETIC
ROSBAG_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSBAG_VERSION = 1.11.21
endif

ROSBAG_SITE = $(call github,ros,ros_comm,$(ROSBAG_VERSION))
ROSBAG_SUBDIR = tools/rosbag

ROSBAG_DEPENDENCIES = boost topic-tools rosbag-storage rosconsole

$(eval $(catkin-package))

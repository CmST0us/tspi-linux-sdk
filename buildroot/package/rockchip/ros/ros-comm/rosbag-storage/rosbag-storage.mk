#default to KINETIC
ROSBAG_STORAGE_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSBAG_STORAGE_VERSION = 1.11.21
endif

ROSBAG_STORAGE_SOURCE = $(ROSBAG_STORAGE_VERSION).tar.gz
ROSBAG_STORAGE_SITE = $(call github,ros,ros_comm,$(ROSBAG_STORAGE_VERSION))
ROSBAG_STORAGE_SUBDIR = tools/rosbag_storage

ROSBAG_STORAGE_DEPENDENCIES = roslz4 boost bzip2 boost console-bridge cpp_common \
							  roscpp_serialization roscpp_traits rostime

$(eval $(catkin-package))

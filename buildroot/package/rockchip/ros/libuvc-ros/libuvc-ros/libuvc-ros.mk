LIBUVC_ROS_VERSION = 0.0.9
LIBUVC_ROS_SITE = $(call github,ros-drivers,libuvc_ros,$(LIBUVC_ROS_VERSION))
LIBUVC_ROS_SUBDIR = libuvc_ros

$(eval $(catkin-package))

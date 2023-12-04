TF2_ROS_VERSION = 0.5.17

TF2_ROS_SITE = $(call github,ros,geometry2,$(TF2_ROS_VERSION))
TF2_ROS_SUBDIR = tf2_ros
TF2_ROS_DEPENDENCIES = actionlib actionlib-msgs geometry-msgs message-filters \
					   roscpp rosgraph rospy tf2 tf2-msgs tf2-py

$(eval $(catkin-package))

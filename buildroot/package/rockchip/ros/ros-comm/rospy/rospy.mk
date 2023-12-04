#default to KINETIC
ROSPY_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSPY_VERSION = 1.11.21
endif

ROSPY_SITE = $(call github,ros,ros_comm,$(ROSPY_VERSION))
ROSPY_SUBDIR = clients/rospy

ROSPY_DEPENDENCIES = std-msgs genpy rosgraph roslib rosgraph_msgs roscpp \
					 python-pyyaml

$(eval $(catkin-package))

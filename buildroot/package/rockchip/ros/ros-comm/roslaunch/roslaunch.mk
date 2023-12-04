#default to KINETIC
ROSLAUNCH_VERSION = 1.12.14
ROSLAUNCH_WORK_DIR = opt/ros/kinetic

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSLAUNCH_VERSION = 1.11.21
ROSLAUNCH_WORK_DIR = opt/ros/indigo
endif

ROSLAUNCH_SITE = $(call github,ros,ros_comm,$(ROSLAUNCH_VERSION))
ROSLAUNCH_SUBDIR = tools/roslaunch

$(eval $(catkin-package))

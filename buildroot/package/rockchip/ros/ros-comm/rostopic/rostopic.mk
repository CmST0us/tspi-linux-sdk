#default to KINETIC
ROSTOPIC_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSTOPIC_VERSION = 1.11.21
endif

ROSTOPIC_SITE = $(call github,ros,ros_comm,$(ROSTOPIC_VERSION))
ROSTOPIC_SUBDIR = tools/rostopic

ROSTOPIC_DEPENDENCIES = rostest

$(eval $(catkin-package))

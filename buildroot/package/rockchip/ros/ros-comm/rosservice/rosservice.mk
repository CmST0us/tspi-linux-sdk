#default to KINETIC
ROSSERVICE_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSSERVICE_VERSION = 1.11.21
endif

ROSSERVICE_SITE = $(call github,ros,ros_comm,$(ROSSERVICE_VERSION))
ROSSERVICE_SUBDIR = tools/rosservice

$(eval $(catkin-package))

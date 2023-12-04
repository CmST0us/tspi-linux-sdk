#default to KINETIC
ROSCLEAN_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSCLEAN_VERSION = 1.11.14
endif

ROSCLEAN_SITE = $(call github,ros,ros,$(ROSCLEAN_VERSION))
ROSCLEAN_SUBDIR = tools/rosclean

$(eval $(catkin-package))

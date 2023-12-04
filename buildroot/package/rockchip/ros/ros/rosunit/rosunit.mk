#default to KINETIC
ROSUNIT_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSUNIT_VERSION = 1.11.14
endif

ROSUNIT_SITE = $(call github,ros,ros,$(ROSUNIT_VERSION))
ROSUNIT_SUBDIR = tools/rosunit

$(eval $(catkin-package))

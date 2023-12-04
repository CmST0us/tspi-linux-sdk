#default to KINETIC
MK_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
MK_VERSION = 1.11.14
endif

MK_SITE = $(call github,ros,ros,$(MK_VERSION))
MK_SUBDIR = core/mk

$(eval $(catkin-package))

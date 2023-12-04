#default to KINETIC
ROSCREATE_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSCREATE_VERSION = 1.11.14
endif

ROSCREATE_SITE = $(call github,ros,ros,$(ROSCREATE_VERSION))
ROSCREATE_SUBDIR = tools/roscreate

$(eval $(catkin-package))

#default to KINETIC
ROSMAKE_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSMAKE_VERSION = 1.11.14
endif

ROSMAKE_SOURCE = $(ROSMAKE_VERSION).tar.gz
ROSMAKE_SITE = $(call github,ros,ros,$(ROSMAKE_VERSION))
ROSMAKE_SUBDIR = tools/rosmake

$(eval $(catkin-package))

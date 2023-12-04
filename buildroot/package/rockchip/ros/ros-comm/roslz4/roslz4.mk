#default to KINETIC
ROSLZ4_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSLZ4_VERSION = 1.11.21
endif

ROSLZ4_SITE = $(call github,ros,ros_comm,$(ROSLZ4_VERSION))
ROSLZ4_SUBDIR = utilities/roslz4

ROSLZ4_DEPENDENCIES = lz4 bzip2

$(eval $(catkin-package))

TRAJECTORY_MSGS_VERSION = 1.11.9
TRAJECTORY_MSGS_SITE = $(call github,ros,common_msgs,$(TRAJECTORY_MSGS_VERSION))
TRAJECTORY_MSGS_SUBDIR = trajectory_msgs

TRAJECTORY_MSGS_DEPENDENCIES = geometry-msgs

$(eval $(catkin-package))

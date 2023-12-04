SHAPE_MSGS_VERSION = 1.11.9
SHAPE_MSGS_SITE = $(call github,ros,common_msgs,$(SHAPE_MSGS_VERSION))
SHAPE_MSGS_SUBDIR = shape_msgs

SHAPE_MSGS_DEPENDENCIES = geometry-msgs

$(eval $(catkin-package))

GEOMETRY_MSGS_VERSION = 1.11.9
GEOMETRY_MSGS_SITE = $(call github,ros,common_msgs,$(GEOMETRY_MSGS_VERSION))
GEOMETRY_MSGS_SUBDIR = geometry_msgs

GEOMETRY_MSGS_DEPENDENCIES = message-generation std-msgs gtest

$(eval $(catkin-package))

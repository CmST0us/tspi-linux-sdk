NAV_MSGS_VERSION = 1.11.9
NAV_MSGS_SITE = $(call github,ros,common_msgs,$(NAV_MSGS_VERSION))
NAV_MSGS_SUBDIR = nav_msgs

NAV_MSGS_DEPENDENCIES = actionlib-msgs geometry-msgs

$(eval $(catkin-package))

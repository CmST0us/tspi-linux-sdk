HECTOR_NAV_MSGS_VERSION = 0.3.4
HECTOR_NAV_MSGS_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_NAV_MSGS_VERSION))
HECTOR_NAV_MSGS_SUBDIR = hector_nav_msgs

HECTOR_NAV_MSGS_DEPENDENCIES = nav-msgs geometry-msgs message-generation

$(eval $(catkin-package))

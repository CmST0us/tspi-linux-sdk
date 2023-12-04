TF2_GEOMETRY_MSGS_VERSION = 0.5.17

TF2_GEOMETRY_MSGS_SITE = $(call github,ros,geometry2,$(TF2_GEOMETRY_MSGS_VERSION))
TF2_GEOMETRY_MSGS_SUBDIR = tf2_geometry_msgs
TF2_GEOMETRY_MSGS_DEPENDENCIES = orocos-kdl tf2-ros

$(eval $(catkin-package))

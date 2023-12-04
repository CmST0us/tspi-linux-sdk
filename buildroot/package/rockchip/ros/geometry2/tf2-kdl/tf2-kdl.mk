TF2_KDL_VERSION = 0.5.17

TF2_KDL_SITE = $(call github,ros,geometry2,$(TF2_KDL_VERSION))
TF2_KDL_SUBDIR = tf2_kdl
TF2_KDL_DEPENDENCIES = orocos-kdl tf2-ros

$(eval $(catkin-package))

TF2_VERSION = 0.5.17

TF2_SITE = $(call github,ros,geometry2,$(TF2_VERSION))
TF2_SUBDIR = tf2
TF2_DEPENDENCIES = geometry-msgs tf2-msgs boost rostime console-bridge gtest

$(eval $(catkin-package))

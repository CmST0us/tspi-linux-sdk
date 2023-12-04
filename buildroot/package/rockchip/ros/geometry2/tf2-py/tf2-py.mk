TF2_PY_VERSION = 0.5.17

TF2_PY_SITE = $(call github,ros,geometry2,$(TF2_PY_VERSION))
TF2_PY_SUBDIR = tf2_py

TF2_PY_DEPENDENCIES = tf2 rospy

$(eval $(catkin-package))

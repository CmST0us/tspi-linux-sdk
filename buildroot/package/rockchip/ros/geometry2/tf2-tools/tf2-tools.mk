TF2_TOOLS_VERSION = 0.5.17

TF2_TOOLS_SITE = $(call github,ros,geometry2,$(TF2_TOOLS_VERSION))
TF2_TOOLS_SUBDIR = tf2_tools

TF2_TOOLS_DEPENDENCIES = tf2-ros

$(eval $(catkin-package))

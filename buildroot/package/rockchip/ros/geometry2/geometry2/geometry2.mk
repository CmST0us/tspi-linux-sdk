GEOMETRY2_VERSION = 0.5.17

GEOMETRY2_SITE = $(call github,ros,geometry2,$(GEOMETRY2_VERSION))
GEOMETRY2_SUBDIR = geometry2

$(eval $(catkin-package))

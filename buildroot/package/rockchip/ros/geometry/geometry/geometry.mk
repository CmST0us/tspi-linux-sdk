GEOMETRY_VERSION = 1.11.9

GEOMETRY_SITE = $(call github,ros,geometry,$(GEOMETRY_VERSION))
GEOMETRY_SUBDIR = geometry

$(eval $(catkin-package))

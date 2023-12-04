KDL_CONVERSIONS_VERSION = 1.11.9
KDL_CONVERSIONS_SITE = $(call github,ros,geometry,$(KDL_CONVERSIONS_VERSION))
KDL_CONVERSIONS_SUBDIR = kdl_conversions

KDL_CONVERSIONS_DEPENDENCIES = geometry-msgs orocos-kdl

$(eval $(catkin-package))

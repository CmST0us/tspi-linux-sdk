HECTOR_MAP_TOOLS_VERSION = 0.3.4
HECTOR_MAP_TOOLS_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_MAP_TOOLS_VERSION))
HECTOR_MAP_TOOLS_SUBDIR = hector_map_tools

HECTOR_MAP_TOOLS_DEPENDENCIES = nav-msgs

$(eval $(catkin-package))

HECTOR_MAP_SERVER_VERSION = 0.3.4
HECTOR_MAP_SERVER_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_MAP_SERVER_VERSION))
HECTOR_MAP_SERVER_SUBDIR = hector_map_server

HECTOR_MAP_SERVER_DEPENDENCIES = roscpp hector-map-tools hector-marker-drawing \
				 hector-nav-msgs nav-msgs tf

$(eval $(catkin-package))

HECTOR_GEOTIFF_VERSION = 0.3.4
HECTOR_GEOTIFF_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_GEOTIFF_VERSION))
HECTOR_GEOTIFF_SUBDIR = hector_geotiff

HECTOR_GEOTIFF_DEPENDENCIES = eigen cmake_modules hector-map-tools \
			      hector-nav-msgs nav-msgs pluginlib \
			      roscpp std-msgs qt

$(eval $(catkin-package))

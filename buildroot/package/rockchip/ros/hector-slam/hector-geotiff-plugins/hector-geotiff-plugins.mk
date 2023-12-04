HECTOR_GEOTIFF_PLUGINS_VERSION = 0.3.4
HECTOR_GEOTIFF_PLUGINS_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_GEOTIFF_PLUGINS_VERSION))
HECTOR_GEOTIFF_PLUGINS_SUBDIR = hector_geotiff_plugins

HECTOR_GEOTIFF_PLUGINS_DEPENDENCIES = hector-geotiff hector-nav-msgs

$(eval $(catkin-package))

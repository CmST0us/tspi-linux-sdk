HECTOR_COMPRESSED_MAP_TRANSPORT_VERSION = 0.3.4
HECTOR_COMPRESSED_MAP_TRANSPORT_SITE = $(call github,tu-darmstadt-ros-pkg,hector_slam,$(HECTOR_COMPRESSED_MAP_TRANSPORT_VERSION))
HECTOR_COMPRESSED_MAP_TRANSPORT_SUBDIR = hector_compressed_map_transport

HECTOR_COMPRESSED_MAP_TRANSPORT_DEPENDENCIES = nav-msgs cmake_modules cv-bridge \
					       geometry-msgs hector-map-tools  \
					       image-transport sensor-msgs

$(eval $(catkin-package))

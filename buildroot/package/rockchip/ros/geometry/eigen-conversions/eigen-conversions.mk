EIGEN_CONVERSIONS_VERSION = 1.11.9

EIGEN_CONVERSIONS_SITE = $(call github,ros,geometry,$(EIGEN_CONVERSIONS_VERSION))
EIGEN_CONVERSIONS_SUBDIR = eigen_conversions

EIGEN_CONVERSIONS_DEPENDENCIES = orocos-kdl cmake_modules geometry-msgs \
				 std-msgs eigen
EIGEN_CONVERSIONS_CONF_OPTS = -DEIGEN3_INCLUDE_DIRS=$(STAGING_DIR)/usr/include/eigen3

$(eval $(catkin-package))

PLUGINLIB_VERSION = 1.11.3
ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
PLUGINLIB_VERSION = 1.9.16
endif

PLUGINLIB_SITE = $(call github,ros,pluginlib,$(PLUGINLIB_VERSION))

PLUGINLIB_DEPENDENCIES = boost class-loader rosconsole roslib

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
PLUGINLIB_DEPENDENCIES += tinyxml gtest
endif

ifeq ($(BR2_PACKAGE_ROS_KINETIC),y)
PLUGINLIB_DEPENDENCIES += tinyxml2
define PLUGINLIB_KINETIC_PATCH
	$(SED) 's%find_package(TinyXML2%find_package(tinyxml2%' $(@D)/CMakeLists.txt
endef
PLUGINLIB_POST_PATCH_HOOKS += PLUGINLIB_KINETIC_PATCH
endif


$(eval $(catkin-package))

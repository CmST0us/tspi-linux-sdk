#default to KINETIC
XMLRPCPP_VERSION = 1.12.14

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
XMLRPCPP_VERSION = 1.11.21
endif

XMLRPCPP_SITE = $(call github,ros,ros_comm,$(XMLRPCPP_VERSION))
XMLRPCPP_SUBDIR = utilities/xmlrpcpp

XMLRPCPP_DEPENDENCIES = cpp_common rostime

$(eval $(catkin-package))

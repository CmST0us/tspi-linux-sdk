DYNAMIC_RECONFIGURE_VERSION = 1.5.48
DYNAMIC_RECONFIGURE_SITE = $(call github,ros,dynamic_reconfigure,$(DYNAMIC_RECONFIGURE_VERSION))

DYNAMIC_RECONFIGURE_CONF_OPTS += -DCATKIN_ENABLE_TESTING=False
DYNAMIC_RECONFIGURE_DEPENDENCIES = boost roslib std-msgs roscpp rostest \
				   python-rospkg

$(eval $(catkin-package))


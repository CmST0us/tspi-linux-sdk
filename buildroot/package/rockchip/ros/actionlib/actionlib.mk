ACTIONLIB_VERSION = 1.11.12
ACTIONLIB_SITE = $(call github,ros,actionlib,$(ACTIONLIB_VERSION))

ACTIONLIB_DEPENDENCIES = \
	actionlib-msgs roscpp boost

ifeq ($(BR2_PACKAGE_ACTIONLIB_ENABLE_TEST),y)
ACTIONLIB_DEPENDENCIES += rostest
else
ACTIONLIB_CONF_OPTS += -DCATKIN_ENABLE_TESTING=False
endif

$(eval $(catkin-package))

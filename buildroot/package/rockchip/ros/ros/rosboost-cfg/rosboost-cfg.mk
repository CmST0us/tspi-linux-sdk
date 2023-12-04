#default to KINETIC
ROSBOOST_CFG_VERSION = 1.14.4

ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
ROSBOOST_CFG_VERSION = 1.11.14
endif

ROSBOOST_CFG_SOURCE = $(ROSBOOST_CFG_VERSION).tar.gz
ROSBOOST_CFG_SITE = $(call github,ros,ros,$(ROSBOOST_CFG_VERSION))
ROSBOOST_CFG_SUBDIR = tools/rosboost_cfg

$(eval $(catkin-package))

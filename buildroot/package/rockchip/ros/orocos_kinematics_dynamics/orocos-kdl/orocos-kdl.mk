OROCOS_KDL_VERSION = 1.3.0

OROCOS_KDL_SITE = $(call github,orocos,orocos_kinematics_dynamics,$(OROCOS_KDL_VERSION))
OROCOS_KDL_SUBDIR = orocos_kdl
OROCOS_KDL_INSTALL_STAGING = YES

OROCOS_KDL_CONF_OPTS += \
	-DSTAGING_DIR="$(STAGING_DIR)"

OROCOS_KDL_DEPENDENCIES = eigen

$(eval $(cmake-package))

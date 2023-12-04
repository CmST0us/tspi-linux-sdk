###############################################################################
#
# MYNT EYE D SDK samples
#
###############################################################################

MYNT_EYE_D_SDK_SAMPLES_VERSION = $(MYNT_EYE_D_SDK_VERSION)
MYNT_EYE_D_SDK_SAMPLES_SITE = $(MYNT_EYE_D_SDK_SITE)
MYNT_EYE_D_SDK_SAMPLES_LICENSE = LICENSE

MYNT_EYE_D_SDK_SAMPLES_SUBDIR = samples

MYNT_EYE_D_SDK_SAMPLES_DEPENDENCIES = mynt-eye-d-sdk

# TODO: Support VTK
MYNT_EYE_D_SDK_SAMPLES_CONF_OPTS = -DWITH_PCL=OFF

# The samples doesn't have a install target in cmake
define MYNT_EYE_D_SDK_SAMPLES_INSTALL_TARGET_CMDS
	cp -rp $(@D)/samples/_output/bin/* $(STAGING_DIR)/usr/bin/
endef

$(eval $(cmake-package))

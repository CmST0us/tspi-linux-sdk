################################################################################
#
# intel-wds
#
################################################################################

INTEL_WDS_VERSION = ece955a9947e8d5848223c849d2c0f3f928078d4
INTEL_WDS_SITE = $(call github,intel,wds,$(INTEL_WDS_VERSION))
INTEL_WDS_LICENSE = LGPL-2.1+
INTEL_WDS_LICENSE_FILES = COPYING
INTEL_WDS_DEPENDENCIES = \
	host-pkgconf \
	libglib2 \
	gstreamer1 \

define INTEL_WDS_INSTALL_TARGET_CMDS
	cp -f $(@D)/sink/sink-test $(TARGET_DIR)/usr/bin/
endef

$(eval $(cmake-package))

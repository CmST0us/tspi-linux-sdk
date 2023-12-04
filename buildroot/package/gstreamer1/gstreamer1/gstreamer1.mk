################################################################################
#
# gstreamer1
#
################################################################################

define GSTREAMER1_INSTALL_TARGET_ENV
	$(INSTALL) -D -m 0644 $(GSTREAMER1_PKGDIR)/gst.sh \
		$(TARGET_DIR)/etc/profile.d/gst.sh
endef
GSTREAMER1_POST_INSTALL_TARGET_HOOKS += GSTREAMER1_INSTALL_TARGET_ENV

ifeq ($(BR2_PREFER_ROCKCHIP_RGA),y)
define GSTREAMER1_TARGET_USE_RGA
	$(SED) 's/# \(.*USE_RGA\)/\1/' $(TARGET_DIR)/etc/profile.d/gst.sh
endef
GSTREAMER1_POST_INSTALL_TARGET_HOOKS += GSTREAMER1_TARGET_USE_RGA
endif

ifeq ($(BR2_PACKAGE_GSTREAMER1_18),y)
include $(pkgdir)/1_18.inc
else ifeq ($(BR2_PACKAGE_GSTREAMER1_20),y)
include $(pkgdir)/1_20.inc
endif

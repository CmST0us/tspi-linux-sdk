################################################################################
#
# gst1-plugins-base
#
################################################################################

ifeq ($(BR2_PACKAGE_ROCKCHIP_RGA),y)
GST1_PLUGINS_BASE_DEPENDENCIES += rockchip-rga
endif

ifeq ($(BR2_PACKAGE_GSTREAMER1_18),y)
include $(pkgdir)/1_18.inc
else ifeq ($(BR2_PACKAGE_GSTREAMER1_20),y)
include $(pkgdir)/1_20.inc
endif

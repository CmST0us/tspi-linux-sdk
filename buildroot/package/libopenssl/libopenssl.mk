################################################################################
#
# libopenssl
#
################################################################################

ifeq ($(BR2_PACKAGE_LIBOPENSSL_1_0),y)
include $(pkgdir)/libopenssl-1.0.inc
else
include $(pkgdir)/libopenssl-1.1.inc
endif

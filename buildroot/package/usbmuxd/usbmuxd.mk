################################################################################
#
# usbmuxd
#
################################################################################

USBMUXD_VERSION = 1.1.1
USBMUXD_SOURCE = usbmuxd-$(USBMUXD_VERSION).tar.bz2
USBMUXD_SITE = https://github.com/libimobiledevice/usbmuxd/releases/download/$(USBMUXD_VERSION)
USBMUXD_LICENSE = LGPL-2.1+
USBMUXD_LICENSE_FILES = COPYING
USBMUXD_CPE_ID_VENDOR = usbmuxd

USBMUXD_DEPENDENCIES = libusb libimobiledevice libimobiledevice-glue

$(eval $(autotools-package))

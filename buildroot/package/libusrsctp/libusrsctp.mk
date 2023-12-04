################################################################################
#
# LIBUSRSCTP
#
################################################################################
LIBUSRSCTP_VERSION = 928bf9cc66892c809fcfb6c7985887a1f381aac4
LIBUSRSCTP_SITE = $(call github,sctplab,usrsctp,$(LIBUSRSCTP_VERSION))

LIBUSRSCTP_INSTALL_STAGING = YES

LIBUSRSCTP_CONF_OPTS += -DCMAKE_SHARED_LINKER_FLAGS="-lpthread"

$(eval $(cmake-package))

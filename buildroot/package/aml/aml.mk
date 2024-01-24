################################################################################
#
# aml
#
################################################################################

AML_SITE = https://github.com/any1/aml.git
AML_VERSION = 3285a4345deaf6ae63a88cb666ecd743090b0081
AML_SITE_METHOD = git
AML_LICENSE = ISC
AML_LICENSE_FILES = COPYING
AML_INSTALL_STAGING = YES

$(eval $(meson-package))

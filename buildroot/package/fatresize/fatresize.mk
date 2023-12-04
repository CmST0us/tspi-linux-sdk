################################################################################
#
# fatresize
#
################################################################################

FATRESIZE_VERSION = 321973ba156bbf2489e82c47c94b2bca74b16316
FATRESIZE_SITE = $(call github,ya-mouse,fatresize,$(FATRESIZE_VERSION))
FATRESIZE_LICENSE = GPL-3.0
FATRESIZE_LICENSE_FILES = COPYING
FATRESIZE_AUTORECONF = YES

FATRESIZE_DEPENDENCIES = parted

$(eval $(autotools-package))

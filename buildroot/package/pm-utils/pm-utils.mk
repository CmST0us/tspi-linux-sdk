################################################################################
#
# pm-utils
#
################################################################################

PM_UTILS_VERSION = 1.4.1
PM_UTILS_SITE = http://pm-utils.freedesktop.org/releases
PM_UTILS_SOURCE = pm-utils-$(PM_UTILS_VERSION).tar.gz
PM_UTILS_LICENSE = GPL-2.0+
PM_UTILS_LICENSE_FILES = COPYING

$(eval $(autotools-package))

################################################################################
#
# stressapptest
#
################################################################################

STRESSAPPTEST_SITE = https://github.com/stressapptest/stressapptest.git
STRESSAPPTEST_SITE_METHOD = git
STRESSAPPTEST_VERSION = 6714c57d0d67f5a2a7a9987791af6729289bf64e

STRESSAPPTEST_LICENSE = Apache-2.0
STRESSAPPTEST_LICENSE_FILES = NOTICE

$(eval $(autotools-package))

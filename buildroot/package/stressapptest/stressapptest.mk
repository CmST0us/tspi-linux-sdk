################################################################################
#
# stress app test
#
################################################################################
STRESSAPPTEST_SITE = https://github.com/stressapptest/stressapptest
STRESSAPPTEST_VERSION = master
STRESSAPPTEST_SITE_METHOD = git
STRESSAPPTEST_GIT_SUBMODULES = YES

$(eval $(autotools-package))
